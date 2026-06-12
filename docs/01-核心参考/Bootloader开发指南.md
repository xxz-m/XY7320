---
title: Bootloader 开发指南
module: bootloader
type: ref
tags: [bootloader, linker, flash, jump, type/ref]
created: 2026-06-11
related: "[[硬件参考]], [[APP工程配置]], [[简化升级协议]], [[mOTA移植指南]]"
---

# Bootloader 开发指南

本文档是 XY7320 Bootloader 开发的唯一权威参考，涵盖链接脚本、启动跳转、Flash 操作、mOTA 集成等核心内容。

---

## 链接脚本配置

Bootloader 链接脚本位于：

```text
XY_bootloadr/STM32F407ZGTX_FLASH.ld
```

关键配置：

```ld
FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
```

含义：

- Bootloader 从 `0x08000000` 开始存放
- 最多使用 64 KB
- 正好覆盖 STM32F407 的 Sector0 ~ Sector3

> [!warning] Flash 长度限制
> `LENGTH = 64K` 与 `boot_config.h` 中的 `BOOTLOADER_SIZE = 0x00010000` 必须对应。
> 如果链接脚本允许的范围超出 64 KB，Bootloader 可能侵占 APP 区。

![[硬件参考#Flash 分区表]]

---

## 启动跳转流程

Bootloader 跳转 APP 时，APP 首地址处必须是有效的向量表。

跳转步骤（唯一权威描述，共 8 步）：

```text
1. 读取 APP 首地址（0x08010000）处的 word 作为初始 MSP
2. 读取 APP 首地址 + 4（0x08010004）处的 word 作为 Reset_Handler
3. 关闭全局中断（__disable_irq）
4. 关闭 SysTick（CTRL / LOAD / VAL 清零）
5. 清除所有 NVIC 中断使能和挂起（ICER / ICPR）
6. 设置 SCB->VTOR = APP 地址
7. 设置 MSP = APP 初始栈顶
8. 调用 APP Reset_Handler（函数指针跳转）
```

> [!important] 顺序不可打乱
> 必须先清场（关中断、关 SysTick、清 NVIC），再设置 VTOR 和 MSP，最后跳转。
> 否则 APP 可能在跳转瞬间响应到错误的中断向量。

---

## Boot_IsValidApp() 实现

通过判断 APP 首地址处的 MSP 是否落在 SRAM 范围内来确定 APP 是否有效。

```c
#define SRAM_START_ADDR  0x20000000U
#define SRAM_SIZE        0x00020000U
#define SRAM_END_ADDR    (SRAM_START_ADDR + SRAM_SIZE)

int Boot_IsValidApp(uint32_t app_addr)
{
    uint32_t stack_addr = *(volatile uint32_t *)app_addr;

    if ((stack_addr >= SRAM_START_ADDR) && (stack_addr <= SRAM_END_ADDR)) {
        return 1;   // APP 有效
    }

    return 0;       // APP 无效
}
```

> [!note] mOTA 中的等价实现
> mOTA 使用 mask 方式判断：
> ```c
> #define FIRMWARE_HEAD_DATA       0x20000000
> #define FIRMWARE_HEAD_DATA_MASK  0x2FF00000
> ```
> 读取 APP 首 word，与 mask 相与后判断是否等于 `0x20000000`。
> 两种方式本质相同，都是验证栈顶地址是否落在 SRAM 范围。详见 [[硬件参考#APP 有效性判断]]。

---

## Boot_JumpToApp() 完整代码示例

```c
typedef void (*pFunction)(void);

void Boot_JumpToApp(uint32_t app_addr)
{
    /* 1. 读取 APP 向量表前两项 */
    uint32_t app_stack = *(volatile uint32_t *)app_addr;
    uint32_t app_reset = *(volatile uint32_t *)(app_addr + 4);

    /* 2. 关闭全局中断 */
    __disable_irq();

    /* 3. 关闭 SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* 4. 清除所有 NVIC 中断使能和挂起 */
    for (uint8_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;   // 清除中断使能
        NVIC->ICPR[i] = 0xFFFFFFFF;   // 清除中断挂起
    }

    /* 5. 设置向量表偏移到 APP */
    SCB->VTOR = app_addr;

    /* 6. 设置主栈指针 */
    __set_MSP(app_stack);
    __set_CONTROL(0);

    /* 7. 跳转到 APP Reset_Handler */
    ((pFunction)app_reset)();

    /* 不应到达这里 */
    while (1) {}
}
```

> [!tip] 调试建议
> 如果跳转后 APP 不运行或 HardFault，按以下顺序排查：
> 1. 确认 APP 首地址处 MSP 值是否合法（应在 SRAM 范围内）
> 2. 确认 APP 首地址 + 4 处 Reset_Handler 是否合法（应在 APP Flash 范围内）
> 3. 确认 `SCB->VTOR` 是否已设置为 APP 地址
> 4. 确认 SysTick 是否已完全关闭
> 5. 确认 NVIC 所有中断使能和挂起是否已清除

---

## Flash 读写擦除实现要点

Flash 底层操作实现在 `flash_port_stm32f4.c` 中，提供三个函数：

```c
int read(long offset, uint8_t *buf, size_t size);
int write(long offset, const uint8_t *buf, size_t size);
int erase(long offset, size_t size);
```

> [!warning] 函数不能声明为 static
> `bsp_flash.c` 通过 `extern` 声明寻找 `read` / `write` / `erase`。
> 如果 `flash_port_stm32f4.c` 中声明为 `static`，链接器将找不到这些函数。

### read -- 读取

直接从 Flash 地址读取（Flash 在 STM32 中是内存映射的）：

```c
memcpy(buf, (const void *)offset, size);
```

检查要点：

- `buf` 不为空
- `size` 不为 0
- 地址范围在 APP 区内（`0x08010000` ~ `0x080FFFFF`）

### write -- 写入

STM32F407 片内 Flash 按 32 bit（WORD）写入：

```c
HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data32);
```

要求：

- 地址 **4 字节对齐**
- 写入前 **已擦除**
- 写入范围不能越过 APP 区
- 写入后 **读回校验**

### erase -- 擦除

STM32F407 按 Sector 擦除。APP 区从 `0x08010000` 开始，对应 Sector4。

只允许擦除：

```text
Sector4 ~ Sector11
```

> [!warning] APP 区保护 -- 严禁操作 Bootloader 区
> **绝对禁止** 擦除或写入 Sector0 ~ Sector3（`0x08000000` ~ `0x0800FFFF`）。
> 这是 Bootloader 自身所在区域，误操作将导致设备变砖。
> `flash_port_stm32f4.c` 中已通过地址范围检查实现保护：
> ```c
> #define APP_START_ADDR  0x08010000U
> #define FLASH_END_ADDR  0x08100000U
> ```

---

## APP 区保护

Flash Port 层必须实施以下保护措施：

1. **地址范围检查**：所有 `read` / `write` / `erase` 操作的地址必须 >= `APP_START_ADDR`（`0x08010000`）
2. **Sector 限制**：擦除操作只允许 Sector4 ~ Sector11
3. **写入范围限制**：写入不得越过 `FLASH_END_ADDR`（`0x08100000`）
4. **Bootloader 区只读**：任何情况下不得对 Sector0 ~ Sector3 执行写或擦除操作

---

## mOTA Flash 调用链

阶段二真正接通的 mOTA Flash 调用链：

```text
Bootloader_Init()
   |
   v
FM_Init()
   |
   v
BSP_Flash_Init(&_flash_app_part, APP_PART_NAME, APP_ADDRESS, APP_PART_SIZE)
```

固件写入相关调用链：

```text
firmware_manage.c
   |
   v
BSP_Flash_Read / BSP_Flash_Write / BSP_Flash_Erase
   |
   v
read / write / erase  (flash_port_stm32f4.c)
   |
   v
HAL_FLASH_Program / HAL_FLASHEx_Erase
```

> [!note] 阶段二的核心目标
> 先证明 mOTA 的固件管理层能找到 APP 分区，并能通过 BSP Flash 操作到 STM32F4 内部 Flash。
> 串口和 YModem 属于后续阶段。

---

## 启动决策逻辑

Bootloader 上电后的启动决策流程：

```text
Bootloader 启动
  |
  v
初始化 GPIO / DMA / USART1 / USART2
  |
  v
USART1 输出调试日志
  |
  v
等待 PB8 按键 10 秒（每 10ms 扫描，50ms 消抖）
  |
  +-- 按键有效 --> 停留在 Bootloader，开启 USART2 DMA + IDLE 接收
  |
  +-- 按键无效 --> 检查 APP 是否有效
                      |
                      +-- APP 有效 --> 跳转 APP
                      |
                      +-- APP 无效 --> 停留在 Bootloader，开启 USART2 DMA + IDLE 接收
```

### PB8 按键消抖参数

```c
#define BOOT_KEY_GPIO_PORT       GPIOB
#define BOOT_KEY_GPIO_PIN        GPIO_PIN_8
#define BOOT_KEY_PRESSED_LEVEL   GPIO_PIN_RESET    // 低电平按下
#define BOOT_KEY_WAIT_TIMEOUT_MS 10000U            // 等待超时 10 秒
#define BOOT_KEY_SCAN_PERIOD_MS  10U               // 扫描周期 10ms
#define BOOT_KEY_DEBOUNCE_MS     50U               // 消抖时间 50ms
```

> [!tip] 调试建议
> 启动和等待期间通过 USART1 输出调试日志，可观察：
> - `bootloader start`
> - `hold PB8 to enter upgrade mode, wait 10000 ms...`
> - `PB8 low detected, debounce...` / `PB8 debounce ok`
> - `APP valid, jump to APP` / `APP invalid, stay bootloader`

---

## boot_config.h 与 bootloader_config.h 的区别

工程中当前存在两个配置文件，作用不同：

| 文件 | 路径 | 用途 |
| --- | --- | --- |
| `boot_config.h` | `Core/bootloader/boot_config.h` | Bootloader 自身逻辑使用（跳转地址、SRAM 范围、按键参数等） |
| `bootloader_config.h` | `MOTA/Config/bootloader_config.h` | mOTA 核心使用（Flash 分区大小、单/多分区配置、功能开关等） |

### boot_config.h 关键内容

```c
#define BOOTLOADER_ADDR         0x08000000U
#define BOOTLOADER_SIZE         0x00010000U
#define APP_ADDRESS             0x08010000U
#define APP_PART_SIZE           0x000F0000U
#define SRAM_START_ADDR         0x20000000U
#define SRAM_SIZE               0x00020000U
#define SRAM_END_ADDR           (SRAM_START_ADDR + SRAM_SIZE)
```

### bootloader_config.h 关键内容

```c
#define ONCHIP_FLASH_SIZE       (1024 * 1024)
#define BOOTLOADER_SIZE         (64 * 1024)
#define APP_PART_SIZE           (ONCHIP_FLASH_SIZE - BOOTLOADER_SIZE)
#define USING_PART_PROJECT      ONE_PART_PROJECT
```

> [!warning] 宏重复定义问题
> 两个文件都定义了 `BOOTLOADER_SIZE`、`APP_ADDRESS`、`APP_PART_SIZE`，编译时会产生重定义警告。
> 后续建议统一配置来源，避免维护两份配置导致不一致。

---

## 相关文档

- [[硬件参考]] -- 芯片参数、Flash 分区、串口引脚
- [[APP工程配置]] -- APP 链接脚本、向量表偏移、GCC 裸机配置
- [[简化升级协议]] -- 升级通信协议
- [[mOTA移植指南]] -- mOTA 完整移植流程
