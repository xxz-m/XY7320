# XY_bootloadr 移植 mOTA bootloader_ymodem 三阶段指南（新版）

本文档针对你的当前环境重新整理：

```text
目标工程：XY_bootloadr
开发环境：CLion + STM32CubeCLT + CMake + arm-none-eabi-gcc
目标芯片：STM32F407ZGTx，片内 Flash 1MB
参考例程：STM32F407/bootloader_ymodem
参考组件：mOTA v2.0
```

本文档结合了：

```text
1. 你当前 XY_bootloadr 工程结构
2. STM32F407/bootloader_ymodem 例程
3. mOTA 官方 README 中的移植说明
```

移植分为三个大阶段：

```text
阶段一：先跑通 bootloader + APP 跳转
阶段二：移植 mOTA 核心和 Flash 分区管理
阶段三：移植 YModem 串口接收，跑通 fpk 固件升级
```

建议每完成一个阶段就停下来检查，不要一次性全量移植。

---

# 一、移植前必须先明确的几个原则

## 1. bootloader 以简单稳定为主

bootloader 是救援程序，不是业务程序。第一版不要追求功能完整，先保证：

```text
能启动
能判断 APP
能跳转 APP
能接收固件
能写 APP 区
失败时不擦坏自己
```

第一版建议不要加入：

```text
RTOS
复杂 C++
外部 SPI Flash
factory 分区
恢复出厂
复杂加密
复杂日志系统
```

---

## 2. mOTA 默认升级的是 fpk，不是普通 bin

这是官方 README 里最重要的点之一。

mOTA 默认不能直接发送普通 `.bin` 升级，而是需要：

```text
APP.bin
↓
Firmware_Packager 打包
↓
APP.fpk
↓
YModem_Sender 发送 APP.fpk
```

也就是说，阶段三测试时不能直接发：

```text
APP.bin
```

而应该发：

```text
APP.fpk
```

---

## 3. YModem-1K 下 fpk 表头尺寸必须选 1024 byte

官方 README 明确说明：

```text
本案例采用 YModem-1K 协议，固件打包器表头尺寸需要选择 1024 byte。
```

原因是：

```text
mOTA 希望固件包头单独成一帧发送。
YModem-1K 每帧数据区是 1024 byte。
所以 fpk 表头也要设置为 1024 byte。
```

否则固件包头解析可能失败。

---

## 4. APP 必须修改起始地址和中断向量表

bootloader 在 `0x08000000`，APP 不再从默认地址运行。

APP 需要：

```text
1. 修改链接脚本，让 APP 从 0x08010000 开始
2. APP 启动时设置 SCB->VTOR = 0x08010000
3. APP 侧重新打开全局中断 __enable_irq()
```

因为 bootloader 跳转 APP 前通常会关闭中断：

```c
__disable_irq();
```

所以 APP 启动后需要：

```c
SCB->VTOR = 0x08010000;
__enable_irq();
```

---

## 5. 第一版建议使用主机命令触发升级，不用 update_flag

mOTA 支持两种进入升级方式：

```text
USING_HOST_CMD_UPDATE      上位机命令触发
USING_APP_SET_FLAG_UPDATE  APP 设置标志位后复位进入 bootloader
```

你的环境是 GCC + CMake，不是 Keil AC5/AC6。官方 README 中 `update_flag` 的固定地址写法主要面向 Keil。

如果你使用：

```c
USING_APP_SET_FLAG_UPDATE
```

那么 GCC 下需要专门在链接脚本里做 `.noinit` 段，防止 RAM 标志位被初始化清零。

所以第一版建议使用：

```c
#define USING_IS_NEED_UPDATE_PROJECT USING_HOST_CMD_UPDATE
```

等完整流程跑通后，再考虑 `update_flag + .noinit`。

---

# 二、推荐 Flash 分区

你的芯片是 STM32F407ZGTx，片内 Flash 1MB。

第一版推荐：

```text
Bootloader 区：
起始地址：0x08000000
大小：64KB
范围：0x08000000 ~ 0x0800FFFF

APP 区：
起始地址：0x08010000
大小：960KB
范围：0x08010000 ~ 0x080FFFFF
```

为什么选 64KB bootloader：

```text
STM32F407 前 4 个 sector 分别是 16KB
Sector0: 0x08000000, 16KB
Sector1: 0x08004000, 16KB
Sector2: 0x08008000, 16KB
Sector3: 0x0800C000, 16KB
合计 64KB

APP 从 Sector4 开始：0x08010000
这样地址对齐，擦除安全，逻辑清晰。
```

注意：

```text
官方 STM32F407ZET6 例程是 512KB Flash，默认 APP 地址可能是 0x08008000。
你的 ZGTx 是 1MB Flash，不建议直接照搬例程分区。
```

---

# 阶段一：跑通 bootloader + APP 跳转

## 阶段目标

先不移植完整 mOTA，只实现：

```text
bootloader 启动
判断 APP 是否存在
跳转 APP
APP 从 0x08010000 正常运行
```

这一阶段是所有后续工作的基础。

---

## 1.1 修改 bootloader 链接脚本

你的 `XY_bootloadr` 链接脚本：

```text
STM32F407ZGTX_FLASH.ld
```

把 Flash 从整片 1MB 改成 bootloader 专用 64KB：

```ld
FLASH : ORIGIN = 0x08000000, LENGTH = 64K
```

目的：

```text
防止 bootloader 编译结果占用 APP 区。
```

---

## 1.2 新建 bootloader 配置文件

建议新增：

```text
Core/Inc/boot_config.h
```

内容建议：

```c
#ifndef BOOT_CONFIG_H
#define BOOT_CONFIG_H

#define BOOTLOADER_ADDR         0x08000000U
#define BOOTLOADER_SIZE         0x00010000U

#define APP_ADDRESS             0x08010000U
#define APP_PART_SIZE           0x000F0000U

#define SRAM_START_ADDR         0x20000000U
#define SRAM_SIZE               0x00020000U
#define SRAM_END_ADDR           (SRAM_START_ADDR + SRAM_SIZE)

#endif
```

---

## 1.3 新增 APP 合法性判断

建议新增：

```text
Core/Inc/boot_jump.h
Core/Src/boot_jump.c
```

核心判断：

```c
int Boot_IsValidApp(uint32_t app_addr)
{
    uint32_t stack_addr = *(volatile uint32_t *)app_addr;

    if ((stack_addr >= SRAM_START_ADDR) && (stack_addr <= SRAM_END_ADDR))
    {
        return 1;
    }

    return 0;
}
```

判断依据：

```text
APP 首地址的前 4 字节是初始 MSP。
合法 APP 的 MSP 应该落在 SRAM 区域。
```

---

## 1.4 新增跳转 APP 函数

核心逻辑参考 mOTA 的 `Bootloader_Port_JumpToAPP()`。

建议实现：

```c
typedef void (*pFunction)(void);

void Boot_JumpToApp(uint32_t app_addr)
{
    uint32_t app_stack = *(volatile uint32_t *)app_addr;
    uint32_t app_reset = *(volatile uint32_t *)(app_addr + 4);

    __disable_irq();

    HAL_RCC_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    HAL_DeInit();

    SCB->VTOR = app_addr;
    __set_MSP(app_stack);
    __set_CONTROL(0);

    pFunction app_entry = (pFunction)app_reset;
    app_entry();

    while (1) {}
}
```

注意：

```text
第一版可以先用 HAL_DeInit()。
后续接入 UART/DMA 后，再根据实际使用的外设做更精细的 DeInit。
```

---

## 1.5 修改 bootloader main.c

第一阶段建议主循环先保持简单：

```c
if (Boot_IsValidApp(APP_ADDRESS))
{
    Boot_JumpToApp(APP_ADDRESS);
}

while (1)
{
    HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
    HAL_Delay(500);
}
```

如果需要调试串口，可以打印：

```text
bootloader start
APP valid, jump
APP invalid, stay bootloader
```

---

## 1.6 准备 APP 测试工程

APP 工程链接脚本改成：

```ld
FLASH : ORIGIN = 0x08010000, LENGTH = 960K
```

APP 启动时设置向量表：

```c
__disable_irq();
SCB->VTOR = 0x08010000;
__enable_irq();
```

建议 APP 用不同 LED 频率或串口打印区分：

```text
bootloader：500ms 闪烁
APP：100ms 闪烁，或打印 APP started
```

---

## 阶段一验收标准

阶段一完成后必须满足：

- [ ] `XY_bootloadr` 可以正常编译。
- [ ] bootloader 链接脚本 Flash 限制为 64KB。
- [ ] bootloader 可以烧录运行。
- [ ] APP 链接地址为 `0x08010000`。
- [ ] APP 已设置 `SCB->VTOR = 0x08010000`。
- [ ] APP 启动后执行了 `__enable_irq()`。
- [ ] bootloader 可以判断 APP 是否有效。
- [ ] bootloader 可以成功跳转 APP。
- [ ] 跳转后 APP 不 HardFault。
- [ ] 可以通过 LED 或串口确认 APP 正在运行。

阶段一不通过，不进入阶段二。

---

# 阶段二：移植 mOTA 核心和 Flash 分区管理

## 阶段目标

第二阶段引入 mOTA 核心，但还不急着跑完整 YModem。

目标：

```text
mOTA 核心源码加入工程
bootloader_config.h 适配你的 Flash
片内 Flash 分区管理可用
APP 区可以读、擦、写
工程能编译通过
```

---

## 2.1 建议目录结构

建议在 `XY_bootloadr` 下建立：

```text
MOTA/
├─ Core/
├─ Config/
├─ Module/
├─ Port/
├─ BSP/
│  ├─ inc/
│  └─ src/
└─ Component/
```

也可以保持 mOTA 原始结构，但建议不要把 Keil 工程目录整体复制进来。

---

## 2.2 复制必需核心文件

从 mOTA 公共源码复制：

```text
mOTA/source/bootloader/Core/bootloader.c
mOTA/source/bootloader/Core/bootloader.h
mOTA/source/bootloader/Core/bootloader_define.h
mOTA/source/bootloader/Core/firmware_manage.c
mOTA/source/bootloader/Core/firmware_manage.h
```

从 STM32F407 例程复制：

```text
STM32F407/bootloader_ymodem/Config/bootloader_config.h
```

---

## 2.3 修改 bootloader_config.h

第一版建议使用单分区，便于快速跑通：

```c
#define USING_PART_PROJECT                  ONE_PART_PROJECT

#define ONCHIP_FLASH_SIZE                   (1024 * 1024)
#define BOOTLOADER_SIZE                     (64 * 1024)
#define APP_PART_SIZE                       (ONCHIP_FLASH_SIZE - BOOTLOADER_SIZE)
#define DOWNLOAD_PART_SIZE                  0
#define FACTORY_PART_SIZE                   0
```

第一版建议关闭复杂功能：

```c
#define ENABLE_DECRYPT                      0
#define ENABLE_FACTORY_FIRMWARE_BUTTON      0
```

第一版建议使用主机命令触发升级：

```c
#define USING_IS_NEED_UPDATE_PROJECT        USING_HOST_CMD_UPDATE
#define WAIT_HOST_DATA_MAX_TIME             (5000)
```

如果调试时希望一直停在 bootloader 等待升级，可临时设置：

```c
#define WAIT_HOST_DATA_MAX_TIME             0
```

---

## 2.4 暂时不启用 SPI Flash、FAL、SFUD

第一版只使用片内 Flash。

确保最终走的是：

```c
BSP_Flash_Read()
BSP_Flash_Write()
BSP_Flash_Erase()
```

而不是：

```c
fal_partition_read()
fal_partition_write()
fal_partition_erase()
```

FAL / SFUD 后续如果做外部 SPI Flash 再引入。

---

## 2.5 移植 BSP Flash

需要文件：

```text
mOTA/source/BSP/inc/bsp_flash.h
mOTA/source/BSP/src/bsp_flash.c
```

`bsp_flash.c` 依赖三个底层函数：

```c
int read(long offset, uint8_t *buf, size_t size);
int write(long offset, const uint8_t *buf, size_t size);
int erase(long offset, size_t size);
```

你需要为 STM32F407 实现这三个函数。

建议新增：

```text
MOTA/Port/flash_port_stm32f4.c
```

---

## 2.6 Flash read/write/erase 要求

### read

可以直接从 Flash 地址 memcpy：

```c
memcpy(buf, (const void *)offset, size);
```

### write

STM32F407 片内 Flash 建议按 32bit 写入：

```c
HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data32);
```

要求：

```text
地址 4 字节对齐
长度 4 字节对齐
写入前已擦除
写入后读回校验
禁止写入 bootloader 区
```

### erase

STM32F407 是按 Sector 擦除。

APP 区从：

```text
0x08010000
```

开始，对应 Sector 4。

只允许擦除：

```text
Sector 4 ~ Sector 11
```

严禁擦除：

```text
Sector 0 ~ Sector 3
```

---

## 2.7 修改 CMakeLists.txt

添加 include 路径：

```cmake
include_directories(
    Core/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    MOTA/Core
    MOTA/Config
    MOTA/Module
    MOTA/Port
    MOTA/BSP/inc
    MOTA/Component/crc-lib-c
    MOTA/Component/tinyAES
    MOTA/Component/perf_counter
)
```

源码可以先加：

```cmake
file(GLOB_RECURSE SOURCES "Core/*.*" "Drivers/*.*" "MOTA/*.*")
```

后续稳定后再精简。

---

## 2.8 先解决编译，不急着跑完整逻辑

第二阶段重点是：

```text
mOTA core 编译通过
Flash 接口能单独验证
```

可以先不立即调用完整：

```c
Bootloader_Init();
Bootloader_Loop();
```

等 Flash 和依赖都稳定后再接入。

---

## 阶段二验收标准

阶段二完成后必须满足：

- [ ] mOTA core 已加入工程。
- [ ] `bootloader_config.h` 已适配 STM32F407ZGTx 1MB Flash。
- [ ] 当前配置为 `ONE_PART_PROJECT`。
- [ ] 第一版未启用 SPI Flash / FAL / SFUD。
- [ ] 第一版未启用 AES，或 AES 已确认编译正常。
- [ ] `BSP_Flash_Read()` 可以读取 Flash。
- [ ] `BSP_Flash_Erase()` 只会擦 APP 区 Sector 4 ~ Sector 11。
- [ ] `BSP_Flash_Write()` 可以写入 APP 区。
- [ ] 写入后读回校验一致。
- [ ] 工程完整编译通过。
- [ ] bootloader 区不会被擦除或写坏。

Flash 没验证通过，不进入阶段三。

---

# 阶段三：移植 YModem 接收并跑通 fpk 升级

## 阶段目标

第三阶段接入完整升级流程：

```text
bootloader 上电等待升级
周期发送字符 C
上位机通过 YModem-1K 发送 APP.fpk
bootloader 接收 fpk
解析 fpk 表头
写入 APP 区
校验固件
跳转 APP
```

---

## 3.1 串口分配建议

你当前 `XY_bootloadr` 已有：

```text
USART1：PA9 / PA10，无 DMA
USART2：PA2 / PA3，有 DMA1_Stream5 RX
```

建议第一版：

```text
USART2：YModem 固件接收
USART1：日志打印
```

这样可以利用你已经配置好的 USART2 RX DMA。

---

## 3.2 复制 YModem 模块

从例程复制：

```text
STM32F407/bootloader_ymodem/APP/Module/data_transfer.c
STM32F407/bootloader_ymodem/APP/Module/data_transfer.h
STM32F407/bootloader_ymodem/APP/Module/data_transfer_port.c
STM32F407/bootloader_ymodem/APP/Module/data_transfer_port.h
STM32F407/bootloader_ymodem/APP/Module/protocol_parser.c
STM32F407/bootloader_ymodem/APP/Module/protocol_parser.h
```

建议放到：

```text
MOTA/Module/
```

---

## 3.3 复制并修改 bootloader_port.c

从例程复制：

```text
STM32F407/bootloader_ymodem/APP/User/bootloader_port.c
```

建议放到：

```text
MOTA/Port/bootloader_port.c
```

关键修改：

原例程：

```c
DT_Init(&_data_if, BSP_UART1, _dev_rx_buff, &_dev_rx_len, PP_MSG_BUFF_SIZE + 16);
```

你的工程建议改成：

```c
DT_Init(&_data_if, BSP_UART2, _dev_rx_buff, &_dev_rx_len, PP_MSG_BUFF_SIZE + 16);
```

---

## 3.4 移植 BSP UART

从公共 mOTA 源码复制：

```text
mOTA/source/BSP/inc/bsp_uart.h
mOTA/source/BSP/inc/bsp_timer.h
mOTA/source/BSP/inc/bsp_common.h
mOTA/source/BSP/inc/common.h
mOTA/source/BSP/src/bsp_uart.c
mOTA/source/BSP/src/bsp_uart_stm32.c
mOTA/source/BSP/src/bsp_timer.c
mOTA/source/BSP/src/bsp_gpio_stm32.c
```

从例程复制或参考：

```text
STM32F407/bootloader_ymodem/BSP/inc/bsp_config.h
STM32F407/bootloader_ymodem/BSP/inc/bsp_uart_stm32.h
```

---

## 3.5 修改 bsp_config.h

建议第一版：

```c
#define BSP_PRINTF_BUFF_SIZE                256
#define BSP_PRINTF_HANDLE                   UART(1)

#define BSP_UART_BUFF_SIZE                  64

#define BSP_USING_UART1                     1
#define BSP_USING_UART2                     1
#define BSP_USING_UART2_RE                  0
#define BSP_USING_UART3                     0
#define BSP_USING_UART3_RE                  0
#define BSP_USING_UART4                     0
#define BSP_USING_UART5                     0
#define BSP_USING_UART6                     0
```

含义：

```text
UART1：日志
UART2：YModem
```

---

## 3.6 检查 USART2 DMA 和中断

USART2 RX DMA 应为：

```text
DMA1_Stream5
Channel 4
```

中断里需要：

```c
void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart2_rx);
}
```

还需要 USART2 中断：

```c
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}
```

并确保初始化时开启：

```c
HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
HAL_NVIC_EnableIRQ(USART2_IRQn);
```

---

## 3.7 DMA 模式建议改成 CIRCULAR

原 STM32F407 例程的 USART1 RX DMA 是：

```c
DMA_CIRCULAR
```

你当前 USART2 RX DMA 如果是：

```c
DMA_NORMAL
```

建议改成：

```c
hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
```

这样更适合 UART IDLE 中断判断一帧结束。

---

## 3.8 必须接入 UART IDLE 中断

mOTA 的数据接收判断依赖：

```c
BSP_UART_IsFrameEnd()
```

它本质上依赖 UART 空闲中断设置：

```c
uart->is_idle_int = true;
```

所以必须确认 `bsp_uart_stm32.c` 中正确处理了：

```text
USART IDLE interrupt
DMA 当前计数
拷贝接收数据到用户缓冲区
设置一帧结束标志
```

如果 IDLE 中断没工作，YModem 收包会卡住。

---

## 3.9 移植并接入 BSP Timer

BSP Timer 用于：

```text
周期发送字符 C
等待主机超时
断帧检测
LED 闪烁
```

需要在 `SysTick_Handler()` 中加入：

```c
extern void BSP_Timer_Handler(uint8_t ms);
BSP_Timer_Handler(1);
HAL_IncTick();
```

如果 BSP Timer 不工作，上位机可能等不到 `'C'`，YModem 不会开始传输。

---

## 3.10 接入 Bootloader_Init 和 Bootloader_Loop

最终 `main.c` 建议变成：

```c
HAL_Init();
SystemClock_Config();

MX_GPIO_Init();
MX_DMA_Init();
MX_USART1_UART_Init();
MX_USART2_UART_Init();

Bootloader_Init();

while (1)
{
    Bootloader_Loop();
}
```

注意：

```text
Bootloader_Loop() 必须不间断循环调用。
不要在主循环里加入长时间 HAL_Delay()。
```

---

## 3.11 使用 fpk 进行升级测试

测试流程：

```text
1. 编译 APP 工程，生成 APP.bin
2. 使用 Firmware_Packager 打包 APP.bin
3. 表头尺寸选择 1024 byte
4. 生成 APP.fpk
5. 使用 YModem_Sender 发送 APP.fpk
6. bootloader 接收并写入 APP 区
7. 校验成功后跳转 APP
```

注意：

```text
不要直接发送 APP.bin。
mOTA 官方 README 明确说明目前不能直接用 bin 更新。
```

---

## 3.12 调试时重点看这些现象

串口日志重点关注：

```text
是否打印 mOTA 初始化信息
是否周期发送字符 C
是否收到 SOH/STX
是否打印 file name / file size
是否进入 FIRMWARE_HEAD
是否进入 FIRMWARE_PKG
是否擦除 APP 区
是否写入 APP 区
是否校验成功
是否跳转 APP
```

如果失败，优先检查：

```text
1. fpk 表头是否为 1024 byte
2. 是否误发了 bin
3. USART2 DMA 是否工作
4. USART2 IDLE 中断是否触发
5. BSP_Timer_Handler 是否接入 SysTick
6. APP 地址是否和 bootloader_config.h 一致
7. Flash 擦写是否越界
```

---

## 阶段三验收标准

阶段三完成后必须满足：

- [ ] USART2 作为 YModem 接收口可用。
- [ ] USART1 作为日志口可用。
- [ ] USART2 DMA RX 正常工作。
- [ ] USART2 IDLE 中断可以触发一帧结束。
- [ ] BSP Timer 已接入 SysTick。
- [ ] bootloader 可以周期发送字符 `'C'`。
- [ ] YModem_Sender 可以发送 `.fpk` 文件。
- [ ] `.fpk` 表头尺寸为 1024 byte。
- [ ] bootloader 可以收到 SOH/STX/EOT。
- [ ] 固件包头校验通过。
- [ ] APP 区 Sector 4 ~ Sector 11 可以正确擦除。
- [ ] 固件数据写入 `0x08010000` 后读回正常。
- [ ] 固件完整性校验通过。
- [ ] 升级完成后可以跳转 APP。
- [ ] 重启后可以识别 APP 有效并跳转 APP。
- [ ] 升级失败不会破坏 bootloader 自身。

---

# 四、最终完成标准

完整移植完成后，应达到：

```text
1. bootloader 位于 0x08000000，大小控制在 64KB 内。
2. APP 位于 0x08010000。
3. APP 设置 SCB->VTOR = 0x08010000，并重新开启中断。
4. bootloader 上电后等待 YModem 升级。
5. 未收到升级数据时，可以跳转已有 APP。
6. 收到 APP.fpk 后，可以写入 APP 区。
7. 写入完成后校验 APP。
8. 校验成功后跳转 APP。
9. APP 不存在或损坏时，停留 bootloader 等待升级。
10. bootloader 自身不会被擦除或写坏。
```

---

# 五、建议你每阶段完成后让我检查的内容

## 阶段一完成后发我检查

我重点检查：

```text
链接脚本
APP 地址
跳转函数
SCB->VTOR
__enable_irq
是否可能 HardFault
```

## 阶段二完成后发我检查

我重点检查：

```text
bootloader_config.h
Flash 分区
read/write/erase
是否会误擦 bootloader
CMake include/source
mOTA core 编译依赖
```

## 阶段三完成后发我检查

我重点检查：

```text
USART2 DMA
USART2 IDLE 中断
BSP UART
BSP Timer
YModem 流程
fpk 表头尺寸
固件写入和校验流程
```

---

# 六、推荐移植顺序总结

最稳的执行顺序：

```text
1. bootloader 限制 64KB
2. APP 改到 0x08010000
3. 跑通 bootloader 跳转 APP
4. 移植 mOTA core
5. 适配 bootloader_config.h
6. 实现片内 Flash read/write/erase
7. 验证 APP 区擦写
8. 移植 BSP UART / Timer
9. USART2 DMA + IDLE 跑通
10. 发送 APP.fpk 测试升级
11. 校验成功后跳转 APP
```

不要跳过阶段一和阶段二直接做阶段三。
