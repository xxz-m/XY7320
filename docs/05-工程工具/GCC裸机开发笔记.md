---
title: GCC 裸机开发笔记
module: toolchain
type: ref
tags: [gcc, cmake, app-fw, type/ref]
created: 2026-06-11
related: "[[APP工程配置]], [[CMake与STM32工程笔记]], [[踩坑记录]]"
---

# GCC 裸机开发笔记

本文汇总在 STM32F407 上使用 arm-none-eabi-gcc / newlib 进行裸机开发时遇到的关键问题和解决方案。内容来源于 [[踩坑记录]]、项目笔记和 mOTA 移植过程中的实际经验。

---

## GCC/newlib 启动链路 vs Keil 启动链路

> [!warning] 核心区别
> Keil 和 GCC 的启动链路完全不同，从 Keil 迁移到 GCC 时必须理解这一点，否则跳转后极易 HardFault。

### Keil/ARMCC 启动链路

Keil 的启动文件、运行库、scatter 文件是一整套配好的：

```text
Reset_Handler
  -> SystemInit
  -> __main / __scatterload
  -> main
```

Keil 使用 `__main` 作为入口，内部完成 data 拷贝、bss 清零、`__rt_lib_init` 等初始化，然后调用 `main()`。

### GCC/newlib 启动链路

CLion + arm-none-eabi-gcc 使用的是 GCC/newlib 启动链路：

```text
Reset_Handler
  -> SystemInit
  -> copy data
  -> clear bss
  -> __libc_init_array
  -> main
```

GCC 链路中，启动文件（`startup_stm32f407xx.s`）手动完成 data 拷贝和 bss 清零，然后调用 `__libc_init_array` 完成 C/C++ 静态初始化，最后进入 `main()`。

> [!important] 关键差异
> 如果 CMake 链接参数不够"裸机化"，newlib 可能拉入 `atexit`、`__register_exitproc`、锁对象、析构注册等 MCU 程序通常不需要的逻辑。MCU 固件一般不会从 `main()` 正常返回，也不需要 `exit()` 相关清理逻辑，这些东西反而可能在 Bootloader 跳转场景里踩坑。

### 为什么第一版不注释也能跑

第一版大概率是因为 `.init_array` 基本为空，`__libc_init_array()` 进去转一圈就回到 `main()`。

后来工程启用了 CXX、工具链版本变化、链接参数变化，或者引入了更多运行库依赖，导致 `.init_array` 和 newlib 初始化内容变多，于是 `__libc_init_array()` 不再是"空转"，而是拉进了 `__register_exitproc` 等逻辑。

---

## 裸机链接参数

> [!important] 必须配置的裸机链接参数
> 以下参数需要维护在 `CMakeLists_template.txt` 中，确保 CubeMX/CLion 重新生成 CMake 时也能保留。

```cmake
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)
add_link_options(--specs=nano.specs --specs=nosys.specs -nostartfiles)
```

各参数含义：

| 参数 | 作用 |
|------|------|
| `--specs=nano.specs` | 使用 newlib-nano 精简 C 库，体积更小，适合 MCU |
| `--specs=nosys.specs` | 使用 nosys 系统调用桩，不依赖操作系统 |
| `-nostartfiles` | 不链接默认启动文件（`crt0.o` 等），避免拉入不适合裸机的初始化逻辑 |
| `-fno-use-cxa-atexit` | 禁止 C++ 使用 `__cxa_atexit` 注册析构函数，避免拉入 `__register_exitproc` 等退出清理逻辑 |

---

## __libc_init_array 问题与解决

### 问题现象

Bootloader 成功接收 APP、写入 APP 分区，并且检查首地址通过：

```text
[simple update] app stack: 0x20020000
[simple update] app reset: 0x080106ED
[simple update] APP valid
[simple update] update ok, jump app
```

但是跳转后 LED 没有闪烁。用 ST-LINK / OpenOCD halt 后看到 CPU 已经进入 APP 地址段，但处于 HardFault：

```text
current mode: Handler HardFault
pc: 0x08011250
msp: 0x2001ff90
```

`addr2line` / map 反查后，`0x08011250` 落在 newlib 的 `__register_exitproc` 附近，调用链大致是：

```text
Reset_Handler
  -> SystemInit
  -> copy data
  -> clear bss
  -> __libc_init_array
     -> __register_exitproc
        -> HardFault
```

### 临时方案：注释 __libc_init_array

如果 APP 是最小 C/HAL 程序，没有 C++ 全局对象、静态构造函数，可以临时注释启动文件里的：

```asm
bl __libc_init_array
```

这样可以绕过 newlib 初始化，直接进入 `main()`。

> [!warning] 临时方案的局限
> 如果后续 APP 引入 C++ 全局对象、静态对象、驱动框架初始化、库组件初始化等，注释 `__libc_init_array` 会导致这些构造函数不执行，问题会比较隐蔽。

### 长期方案：nano.specs + minimal_runtime.c

维护 `CMakeLists_template.txt`，让 CubeMX/CLion 重新生成 CMake 时也保留裸机 GCC 参数：

```cmake
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)
add_link_options(--specs=nano.specs --specs=nosys.specs -nostartfiles)
```

同时保留启动文件里的：

```asm
bl __libc_init_array
```

因为使用 `-nostartfiles` 后默认启动文件不再提供 `_init()` / `_fini()`，需要在 APP 工程里提供最小空实现（即 `minimal_runtime.c`）。

---

## minimal_runtime.c

在 APP 工程中新增 `minimal_runtime.c`，提供 `_init` 和 `_fini` 的空实现：

```c
/**
 * minimal_runtime.c
 *
 * 为 GCC 裸机环境提供 _init / _fini 空实现。
 * 使用 -nostartfiles 后，默认启动文件不再提供这两个符号，
 * 需要手动提供以避免链接错误。
 *
 * 这样既保留了 C/C++ 静态初始化入口（__libc_init_array），
 * 又避免拉入不适合当前裸机启动流程的默认启动对象。
 */

void _init(void)
{
}

void _fini(void)
{
}
```

> [!note] 为什么需要这个文件
> 使用 `--specs=nano.specs --specs=nosys.specs -nostartfiles` 后，默认 C 运行时启动对象不再被链接。但 `__libc_init_array` 内部会调用 `_init()`，如果找不到该符号就会链接失败。提供空实现即可满足需求。

---

## CMakeLists_template.txt 维护策略

> [!tip] 核心原则
> `CMakeLists.txt` 是当前编译生效的文件，但 CubeMX 或工具重新生成工程时可能覆盖它。重要配置必须同步到 `CMakeLists_template.txt`。

需要同步到模板文件的关键配置：

```text
MOTA include 路径
MOTA source 路径
CMake 最低版本
链接脚本设置
裸机 GCC 编译/链接参数
```

当前模板文件位置：

```text
XY_bootloadr/CMakeLists_template.txt
XY7320/CMakeLists_template.txt
```

建议每次修改 `CMakeLists.txt` 后，检查对应模板文件是否需要同步更新。

---

## update_flag 在 GCC 下的 .noinit 段放置问题

mOTA 支持两种进入升级方式：

```text
USING_HOST_CMD_UPDATE      上位机命令触发
USING_APP_SET_FLAG_UPDATE  APP 设置标志位后复位进入 Bootloader
```

> [!warning] GCC 与 Keil 的差异
> 官方 README 里 `update_flag` 的固定地址写法更偏 Keil 工程。GCC 下需要处理 `.noinit` 段，防止 RAM 标志位被启动文件清零。

当前工程已经给 GCC 补了：

```c
volatile uint64_t update_flag __attribute__((section(".noinit"), aligned(8)));
```

这能让当前工程编译通过。

> [!note] 后续注意事项
> 如果真的使用 APP 设置标志位升级（`USING_APP_SET_FLAG_UPDATE`），还要继续检查链接脚本里的 `.noinit` 段是否完整配置，确保启动文件不会清零该段。第一版完整升级流程跑通前，建议优先用主机命令触发升级。

---

## RTOS 宏误触发问题

### 问题描述

原始写法类似：

```c
#if (USING_RTOS_TYPE == RTOS_USING_RTTHREAD)
```

如果 `RTOS_USING_RTTHREAD` 没定义，C 预处理器会把它当成 `0`，导致当 `USING_RTOS_TYPE` 也恰好为 `0`（或未定义）时，条件意外成立，误进入 RTOS 分支。

### 修复方式

使用 `#if defined()` 进行双重防护：

```c
#if defined(RTOS_USING_RTTHREAD) && (USING_RTOS_TYPE == RTOS_USING_RTTHREAD)
```

> [!important] 必须防护的位置
> 所有涉及 RTOS 类型判断的宏条件都需要这样处理，包括但不限于：
> - `common.h` 中的 RTOS 分支判断
> - `bsp_uart.h` 中的 RTOS 相关串口配置
> - 其他 BSP 头文件中的 RTOS 条件编译

`bsp_uart.h` 里的 RTOS 判断也要同样处理。

---

## bootloader_port.c 重复定义问题

### 问题现象

曾经同时存在两个 `bootloader_port.c`：

```text
MOTA/Core/bootloader_port.c
MOTA/Port/bootloader_port.c
```

它们都定义了相同的函数：

```text
Bootloader_Port_Init
Bootloader_Port_HostDataProcess
Bootloader_Port_Reset
Bootloader_Port_JumpToAPP
Bootloader_Port_SystemReset
```

导致链接错误：

```text
multiple definition of Bootloader_Port_Init
```

### 根因：GLOB 收集了重复源文件

当前 CMake 使用：

```cmake
file(GLOB_RECURSE SOURCES
    "Core/*.*" "Drivers/*.*"
    "MOTA/Core/*.*"
    "MOTA/BSP/src/*.*"
    "MOTA/Port/*.*"
)
```

只要 `.c` 在这些路径下，就会自动参与编译。两个目录下的同名文件都被 GLOB 收进去，导致重复定义。

### 解决方式

```text
保留 MOTA/Core/bootloader_port.c
删除 MOTA/Port/bootloader_port.c
```

> [!tip] 排查思路
> 后续如果出现类似的 `multiple definition` 错误，按以下步骤排查：
> 1. 搜索同名函数在哪些 `.c` 里定义
> 2. 检查这些 `.c` 是否都在 GLOB 范围内
> 3. 删除或排除重复实现文件

---

## APP 侧启动注意事项

APP 从 Bootloader 跳转启动时，`main()` 开头建议明确做：

```c
SCB->VTOR = 0x08010000U;
__enable_irq();
```

- `SCB->VTOR` 保证中断向量表指向 APP 自己
- `__enable_irq()` 用来恢复 Bootloader 跳转前关闭的全局中断

> [!danger] 不要在启动文件很早的位置提前开中断
> 那时 data/bss、HAL、外设状态可能还没准备好，反而更容易出问题。

---

## 使用 callout 标注重要注意事项

以下是本文档中涉及的关键注意事项汇总：

> [!danger] HardFault 排查
> 如果 Bootloader 跳转 APP 后 HardFault，优先检查：
> 1. `__libc_init_array` 是否拉入了不需要的 newlib 逻辑
> 2. 裸机链接参数是否完整（nano.specs / nosys.specs / -nostartfiles / -fno-use-cxa-atexit）
> 3. `SCB->VTOR` 是否正确设置
> 4. `minimal_runtime.c` 是否提供了 `_init` / `_fini` 空实现

> [!warning] GLOB 收集文件的风险
> `file(GLOB_RECURSE ...)` 会递归收集匹配路径下的所有源文件。新增或删除 `.c` 文件时，务必检查是否会导致重复定义或缺失实现。

> [!tip] 推荐的验证顺序
> 1. 保持当前 Bootloader 跳最小 APP 稳定
> 2. APP 保留 `__libc_init_array`，并使用维护后的 CMake 模板
> 3. 在 APP 工程中接入更多功能模块
> 4. 先确认 APP 直接 ST-LINK 烧到 `0x08010000` 能跑
> 5. 再通过 Bootloader 升级这个 APP
> 6. 如果 Bootloader 跳进去不跑，优先查 VTOR、`__libc_init_array`、链接参数
