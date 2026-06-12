---
title: APP 工程配置
module: app
type: ref
tags: [app-fw, linker, gcc, type/ref]
created: 2026-06-11
related: "[[Bootloader开发指南]], [[硬件参考]], [[GCC裸机开发笔记]]"
---

# APP 工程配置

本文档是 XY7320 项目中 APP 固件工程配置的唯一权威参考，涵盖链接脚本、向量表偏移、中断重开、GCC 裸机配置等关键内容。

---

## 链接脚本修改

APP 不再从 Flash 默认起始地址 `0x08000000` 运行，因为该区域已被 Bootloader 占用。

APP 链接脚本中必须修改 FLASH 区域配置：

```ld
FLASH (rx) : ORIGIN = 0x08010000, LENGTH = 960K
```

原因：

```text
Bootloader 占用 0x08000000 ~ 0x0800FFFF（64 KB）
APP 从 0x08010000 开始
总 Flash 1 MB - Bootloader 64 KB = 960 KB
```

> [!danger] 不改链接脚本的后果
> 如果 APP 不改链接脚本，它仍然会被链接到 `0x08000000`。
> 这样 Bootloader 跳转时读到的 APP 向量表就是错的，APP 无法正常运行。
> 必须保证以下三者一致：
> - APP 链接地址（linker script）
> - APP 向量表地址（`SCB->VTOR`）
> - Bootloader 的 `APP_ADDRESS` 宏

---

## 向量表偏移

APP 运行后，中断向量表必须指向 APP 自己的起始地址。

在 APP 初始化早期（`main()` 开头）设置：

```c
SCB->VTOR = 0x08010000U;
```

或者在 STM32 HAL 工程中配置 `VECT_TAB_OFFSET` 宏。

### 为什么必须设置

```text
Bootloader 跳转 APP 不等同于芯片真正复位。
跳转时 SCB->VTOR 可能仍指向 Bootloader 的向量表（0x08000000）。
如果 APP 不重设 VTOR，中断触发时会跳转到 Bootloader 的中断处理函数，
导致 HardFault 或行为异常。
```

> [!important] 三者一致原则
> 以下三个值必须完全一致：
> 1. APP 链接脚本中的 `FLASH ORIGIN` = `0x08010000`
> 2. APP 代码中的 `SCB->VTOR` = `0x08010000`
> 3. Bootloader 中的 `APP_ADDRESS` = `0x08010000`

---

## 中断重开

Bootloader 跳转 APP 前会关闭全局中断：

```c
__disable_irq();
```

因此 APP 启动后需要重新开启全局中断：

```c
__enable_irq();
```

### 位置

在 `main()` 开头，设置完 `SCB->VTOR` 之后立即调用：

```c
SCB->VTOR = 0x08010000U;
__enable_irq();
```

### 原因

```text
Bootloader 跳转前已关闭全局中断。
如果不重新开启，APP 的所有中断（包括 SysTick、UART、DMA 等）都不会响应。
```

> [!warning] 不要在启动文件过早开中断
> 不要在启动文件（startup_stm32f4xx.s）很早的位置提前开中断。
> 那时 data/bss 尚未初始化、HAL 和外设状态未准备好，提前开中断反而容易导致问题。
> 应在 `main()` 中、外设初始化之前或之后显式开启。

---

## GCC 裸机链接配置

当前工程使用 CLion + arm-none-eabi-gcc，需要特殊的裸机链接参数。

### CMake 配置

在 `CMakeLists_template.txt` 中维护：

```cmake
# C++ 编译选项：禁用 atexit 析构注册
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)

# 链接选项
add_link_options(
    --specs=nano.specs       # 使用 newlib-nano（精简 C 库）
    --specs=nosys.specs      # 不使用系统调用
    -nostartfiles            # 不使用默认启动文件
)
```

### 各参数作用

| 参数 | 作用 |
| --- | --- |
| `--specs=nano.specs` | 使用 newlib-nano 精简 C 库，减小固件体积 |
| `--specs=nosys.specs` | 提供空系统调用桩（`_sbrk`、`_write` 等），不依赖操作系统 |
| `-nostartfiles` | 不链接 GCC 默认启动对象（`crt0.o` 等），由自定义启动文件接管 |
| `-fno-use-cxa-atexit` | 禁用 C++ `atexit` 析构注册，避免拉入 `__register_exitproc` 等不适合裸机的逻辑 |

> [!danger] 不配置裸机参数的后果
> 如果不使用 `-nostartfiles` 和 `-fno-use-cxa-atexit`，newlib 可能拉入 `atexit`、`__register_exitproc`、锁对象、析构注册等 MCU 不需要的逻辑。
> 这些逻辑在 Bootloader 跳转场景下可能导致 HardFault。
> 详见 [[踩坑记录#__libc_init_array 问题]]。

---

## minimal_runtime.c 的作用

使用 `-nostartfiles` 后，GCC 默认启动文件不再提供 `_init()` 和 `_fini()` 函数。

需要在 APP 工程中提供最小空实现：

```c
void _init(void)
{
}

void _fini(void)
{
}
```

### 为什么需要

```text
__libc_init_array 在启动过程中会调用 _init()。
如果使用 -nostartfiles 但没有提供 _init()，链接时会报：
  undefined reference to `_init`

_fini() 同理，用于退出清理，裸机不需要但链接器可能引用。
```

### 保留 __libc_init_array

有了 `minimal_runtime.c` 提供空实现后，可以保留启动文件中的：

```asm
bl __libc_init_array
```

这样做的好处：

- 保留 C/C++ 静态对象初始化入口
- 后续如果引入 C++ 全局对象、静态构造函数，可以正常工作
- 避免 GCC 默认启动文件带来的裸机启动问题

> [!tip] 不要注释 __libc_init_array
> 如果 APP 后续可能接入 RTOS 或使用 C++ 特性，建议保留 `__libc_init_array`。
> 临时注释虽然可以绕过问题，但后续引入全局对象时会出现隐蔽的初始化失败。
> 正确做法是配置好裸机链接参数 + 提供 `_init` / `_fini` 空实现。

---

## APP main() 开头初始化序列建议

APP 从 Bootloader 跳转进入，不等同于芯片真正复位。`main()` 开头应按以下顺序初始化：

```c
int main(void)
{
    /* 1. 重设向量表到 APP 起始地址 */
    SCB->VTOR = 0x08010000U;

    /* 2. 重新开启全局中断（Bootloader 跳转前已关闭） */
    __enable_irq();

    /* 3. HAL 初始化 */
    HAL_Init();

    /* 4. 系统时钟配置 */
    SystemClock_Config();

    /* 5. 外设初始化 */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    /* ... 其他外设 ... */

    /* 6. 应用逻辑 */
    /* ... */

    while (1)
    {
        /* 主循环 */
    }
}
```

> [!important] 不要假设硬件处于复位默认状态
> 从 Bootloader 跳进 APP 时，外设不一定处于真正上电复位状态。
> APP 应像独立程序一样完整初始化所有使用的硬件。
> 不要依赖 Bootloader 中已经打开过的时钟、GPIO、USART 或 DMA。

---

## 相关文档

- [[Bootloader开发指南]] -- Bootloader 跳转流程、Flash 操作
- [[硬件参考]] -- 芯片参数、Flash 分区
- [[GCC裸机开发笔记]] -- GCC 裸机开发详细笔记
- [[踩坑记录]] -- `__libc_init_array` 问题排查过程
