---
title: mOTA移植步骤
tags:
  - docs/reference
  - bootloader
  - mota
status: active
---

# XY_bootloadr 移植 mOTA bootloader_ymodem 三阶段指南

本文档放在项目总资料区：

```text
E:\XY7320\XY7320\docs\mota\mOTA移植步骤.md
```

它基于原来的《mOTA移植步骤.md》整理，并根据当前 `XY_bootloadr` 阶段二已经编译通过的真实移植过程补充细节。

当前环境：

```text
目标工程：XY_bootloadr
开发环境：CLion + STM32CubeCLT + CMake + arm-none-eabi-gcc
目标芯片：STM32F407ZGTx，片内 Flash 1MB
参考例程：STM32F407/bootloader_ymodem
参考组件：mOTA v2.0
```

移植分为三个阶段：

```text
阶段一：先跑通 bootloader + APP 跳转
阶段二：移植 mOTA 核心和 Flash 分区管理
阶段三：移植串口 / YModem 接收，跑通 fpk 固件升级
```

建议每完成一个阶段就停下来检查，不要一次性全量移植。

---

# 一、移植前必须明确的原则

## 1. Bootloader 以简单稳定为主

Bootloader 是救援程序，不是业务程序。第一版不要追求功能完整，先保证：

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

## 2. mOTA 默认升级的是 fpk，不是普通 bin

mOTA 默认不是直接发送普通 `.bin`，而是需要：

```text
APP.bin
↓
Firmware_Packager 打包
↓
APP.fpk
↓
YModem_Sender 发送 APP.fpk
```

阶段三测试时不要直接发：

```text
APP.bin
```

应该发：

```text
APP.fpk
```

## 3. YModem-1K 下 fpk 表头尺寸必须选 1024 byte

本案例采用 YModem-1K 协议，固件打包器表头尺寸建议选择 1024 byte。

原因：

```text
YModem-1K 每帧数据区是 1024 byte。
mOTA 希望固件包头单独成一帧发送。
所以 fpk 表头也要设置为 1024 byte。
```

否则固件包头解析可能失败。

## 4. APP 必须修改起始地址和中断向量表

Bootloader 在：

```text
0x08000000
```

APP 不再从默认地址运行，而是从：

```text
0x08010000
```

APP 侧需要：

```text
1. 修改链接脚本，让 APP 从 0x08010000 开始。
2. APP 启动时设置 SCB->VTOR = 0x08010000。
3. APP 侧重新打开全局中断 __enable_irq()。
```

因为 Bootloader 跳转 APP 前通常会关闭中断：

```c
__disable_irq();
```

所以 APP 启动后需要：

```c
SCB->VTOR = 0x08010000;
__enable_irq();
```

## 5. 第一版建议先不依赖 update_flag

mOTA 支持两种进入升级方式：

```text
USING_HOST_CMD_UPDATE      上位机命令触发
USING_APP_SET_FLAG_UPDATE  APP 设置标志位后复位进入 bootloader
```

当前工程是 GCC + CMake，不是 Keil AC5/AC6。官方 README 里 `update_flag` 的固定地址写法更偏 Keil 工程。

如果使用：

```c
USING_APP_SET_FLAG_UPDATE
```

GCC 下需要处理 `.noinit` 段，防止 RAM 标志位被启动文件清零。

当前阶段二为了编译通过，已经给 GCC 补了：

```c
volatile uint64_t update_flag __attribute__((section(".noinit"), aligned(8)));
```

但如果后续真正依赖复位不清零，还要检查链接脚本里的 `.noinit` 是否完整配置。

第一版完整升级流程跑通前，建议优先用主机命令触发升级；等流程稳定后再考虑 `update_flag + .noinit`。

---

# 二、推荐 Flash 分区

STM32F407ZGTx 片内 Flash 为 1MB。

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

为什么 Bootloader 选 64KB：

```text
STM32F407 前 4 个 sector 分别是 16KB。

Sector0: 0x08000000, 16KB
Sector1: 0x08004000, 16KB
Sector2: 0x08008000, 16KB
Sector3: 0x0800C000, 16KB

合计 64KB。
APP 从 Sector4 开始：0x08010000。
这样地址对齐，擦除安全，逻辑清晰。
```

注意：

```text
官方 STM32F407ZET6 例程是 512KB Flash，默认 APP 地址可能是 0x08008000。
你的 ZGTx 是 1MB Flash，不建议直接照搬例程分区。
```

---

# 阶段一：跑通 Bootloader + APP 跳转

## 阶段目标

先不移植完整 mOTA，只实现：

```text
Bootloader 启动
判断 APP 是否存在
跳转 APP
APP 从 0x08010000 正常运行
```

这一阶段是所有后续工作的基础。

## 1.1 修改 Bootloader 链接脚本

你的 `XY_bootloadr` 链接脚本：

```text
XY_bootloadr/STM32F407ZGTX_FLASH.ld
```

把 Flash 从整片 1MB 改成 Bootloader 专用 64KB：

```ld
FLASH : ORIGIN = 0x08000000, LENGTH = 64K
```

目的：

```text
防止 Bootloader 编译结果占用 APP 区。
```

## 1.2 新建 Bootloader 配置文件

建议配置：

```text
Core/bootloader/boot_config.h
```

关键内容：

```c
#define BOOTLOADER_ADDR         0x08000000U
#define BOOTLOADER_SIZE         0x00010000U

#define APP_ADDRESS             0x08010000U
#define APP_PART_SIZE           0x000F0000U

#define SRAM_START_ADDR         0x20000000U
#define SRAM_SIZE               0x00020000U
#define SRAM_END_ADDR           (SRAM_START_ADDR + SRAM_SIZE)
```

## 1.3 APP 合法性判断

APP 首地址的前 4 字节是初始 MSP。

合法 APP 的 MSP 应该落在 SRAM 区域。

核心判断：

```c
int Boot_IsValidApp(uint32_t app_addr)
{
    uint32_t stack_addr = *(volatile uint32_t *)app_addr;

    if ((stack_addr >= SRAM_START_ADDR) && (stack_addr <= SRAM_END_ADDR)) {
        return 1;
    }

    return 0;
}
```

## 1.4 跳转 APP 函数

跳转步骤：

```text
1. 读取 APP 首地址作为 MSP。
2. 读取 APP 首地址 + 4 作为 Reset_Handler。
3. 关闭全局中断。
4. 关闭 SysTick。
5. 清除 NVIC 中断使能和挂起。
6. 设置 SCB->VTOR = APP 地址。
7. 设置 MSP。
8. 跳转 APP Reset_Handler。
```

示例：

```c
typedef void (*pFunction)(void);

void Boot_JumpToApp(uint32_t app_addr)
{
    uint32_t app_stack = *(volatile uint32_t *)app_addr;
    uint32_t app_reset = *(volatile uint32_t *)(app_addr + 4);

    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    for (uint8_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    SCB->VTOR = app_addr;
    __set_MSP(app_stack);
    __set_CONTROL(0);

    ((pFunction)app_reset)();

    while (1) {}
}
```

## 阶段一验收标准

- [ ] `XY_bootloadr` 可以正常编译。
- [ ] Bootloader 链接脚本 Flash 限制为 64KB。
- [ ] Bootloader 可以烧录运行。
- [ ] APP 链接地址为 `0x08010000`。
- [ ] APP 已设置 `SCB->VTOR = 0x08010000`。
- [ ] APP 启动后执行了 `__enable_irq()`。
- [ ] Bootloader 可以判断 APP 是否有效。
- [ ] Bootloader 可以成功跳转 APP。
- [ ] 跳转后 APP 不 HardFault。

阶段一不通过，不进入阶段二。

---

# 阶段二：移植 mOTA 核心和 Flash 分区管理

## 阶段目标

第二阶段引入 mOTA 核心，但还不跑完整 YModem。

本阶段目标：

```text
mOTA 核心源码加入工程
bootloader_config.h 适配你的 Flash
片内 Flash 分区管理接入
APP 区可以读、擦、写
工程能编译通过
```

结合当前工程实际，阶段二验收可以拆成两层：

```text
第一层：编译通过。
第二层：Flash 擦写读回验证通过。
```

当前状态：

```text
阶段二编译目标：已通过
阶段二 Flash 功能验证：建议补测
串口接入：未做
YModem 接入：未做
```

## 2.1 当前建议目录结构

当前 `XY_bootloadr` 下的 mOTA 目录结构为：

```text
XY_bootloadr/
└─ MOTA/
   ├─ Core/
   ├─ Config/
   ├─ Bsp/
   │  ├─ inc/
   │  └─ src/
   └─ Port/
```

当前阶段二真实使用到的核心文件：

```text
MOTA/Core/bootloader.c
MOTA/Core/bootloader.h
MOTA/Core/bootloader_define.h
MOTA/Core/firmware_manage.c
MOTA/Core/firmware_manage.h
MOTA/Core/bootloader_port.c
MOTA/Config/bootloader_config.h
MOTA/Bsp/inc/bsp_flash.h
MOTA/Bsp/src/bsp_flash.c
MOTA/Port/flash_port_stm32f4.c
```

当前补齐但主要还是占位/声明的头文件：

```text
MOTA/Core/data_transfer.h
MOTA/Core/protocol_parser.h
MOTA/Bsp/inc/bsp_board.h
MOTA/Bsp/inc/bsp_timer.h
MOTA/Bsp/inc/bsp_uart.h
MOTA/Bsp/inc/crcLib.h
MOTA/Bsp/inc/perf_counter.h
MOTA/Bsp/inc/user.h
```

这些占位头文件的作用是让阶段二先编译通过，不代表功能已经完整实现。

## 2.2 复制 mOTA 核心文件

从 mOTA 公共源码复制：

```text
bootloader.c
bootloader.h
bootloader_define.h
firmware_manage.c
firmware_manage.h
```

放到当前工程：

```text
XY_bootloadr/MOTA/Core/
```

从 STM32F407 例程复制或参考：

```text
bootloader_config.h
```

放到：

```text
XY_bootloadr/MOTA/Config/
```

## 2.3 修改 bootloader_config.h

第一版使用单分区，便于快速跑通：

```c
#define USING_PART_PROJECT                  ONE_PART_PROJECT

#define ONCHIP_FLASH_SIZE                   (1024 * 1024)
#define BOOTLOADER_SIZE                     (64 * 1024)
#define APP_PART_SIZE                       (ONCHIP_FLASH_SIZE - BOOTLOADER_SIZE)
#define DOWNLOAD_PART_SIZE                  0
#define FACTORY_PART_SIZE                   0
```

当前分区结果：

```text
APP_ADDRESS = FLASH_BASE + BOOTLOADER_SIZE
APP_ADDRESS = 0x08000000 + 0x10000
APP_ADDRESS = 0x08010000
```

第一版建议关闭复杂功能：

```c
#define ENABLE_DECRYPT                      0
#define DOWNLOAD_PART_SIZE                  0
#define FACTORY_PART_SIZE                   0
```

当前阶段二暂时不启用外部 SPI Flash / FAL / SFUD，最终应该走：

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

## 2.4 bootloader_define.h 中的地址关系

`bootloader_define.h` 中关键宏：

```c
#define ONCHIP_FLASH_END_ADDRESS ((uint32_t)(FLASH_BASE + ONCHIP_FLASH_SIZE))
#define APP_ADDRESS              ((uint32_t)(FLASH_BASE + BOOTLOADER_SIZE))
#define DOWNLOAD_ADDRESS         ((uint32_t)(APP_ADDRESS + APP_PART_SIZE))
#define FACTORY_ADDRESS          ((uint32_t)(DOWNLOAD_ADDRESS + DOWNLOAD_PART_SIZE))
```

当前是单分区方案，所以真正关心的是：

```text
APP_ADDRESS
APP_PART_SIZE
```

只要 `BOOTLOADER_SIZE = 64KB`，APP 起始地址就是 `0x08010000`。

## 2.5 移植 BSP Flash

需要文件：

```text
MOTA/Bsp/inc/bsp_flash.h
MOTA/Bsp/src/bsp_flash.c
```

`bsp_flash.h` 定义 Flash 分区对象：

```c
struct BSP_FLASH
{
    char name[MAX_NAME_LEN];
    uint32_t addr;
    uint32_t len;
    struct BSP_FLASH *next;
};
```

`bsp_flash.c` 的作用是 mOTA 的 Flash 抽象层。

它不直接操作 STM32 Flash，而是依赖三个底层函数：

```c
extern int read(long offset, uint8_t *buf, size_t size);
extern int write(long offset, const uint8_t *buf, size_t size);
extern int erase(long offset, size_t size);
```

所以需要自己实现 STM32F407 版本。

## 2.6 新增 STM32F4 Flash Port

当前新增：

```text
MOTA/Port/flash_port_stm32f4.c
```

它实现：

```c
int read(long offset, uint8_t *buf, size_t size);
int write(long offset, const uint8_t *buf, size_t size);
int erase(long offset, size_t size);
```

注意：这三个函数不能写成 `static`。

原因：

```text
bsp_flash.c 通过 extern 声明寻找 read/write/erase。
如果 flash_port_stm32f4.c 里写成 static，链接器看不到这些函数。
```

当前 Flash Port 的关键保护：

```c
#define APP_START_ADDR 0x08010000U
#define FLASH_END_ADDR 0x08100000U
```

要求：

```text
只能读写擦 APP 区。
严禁擦写 0x08000000 ~ 0x0800FFFF 的 Bootloader 区。
```

## 2.7 Flash read/write/erase 要求

### read

可以直接从 Flash 地址读取：

```c
memcpy(buf, (const void *)offset, size);
```

但要检查：

```text
buf 不为空
size 不为 0
地址范围在 APP 区内
```

### write

STM32F407 片内 Flash 当前按 32bit 写入：

```c
HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data32);
```

要求：

```text
地址 4 字节对齐
写入前已擦除
写入范围不能越过 APP 区
写入后读回校验
```

当前 `flash_port_stm32f4.c` 里写入函数已经做了读回校验。

### erase

STM32F407 是按 Sector 擦除。

APP 区从：

```text
0x08010000
```

开始，对应 Sector4。

只允许擦除：

```text
Sector4 ~ Sector11
```

严禁擦除：

```text
Sector0 ~ Sector3
```

## 2.8 mOTA Flash 调用链

当前阶段二真正接通的调用链是：

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
read / write / erase
   |
   v
HAL_FLASH_Program / HAL_FLASHEx_Erase
```

也就是说，阶段二的核心不是串口，而是先证明：

```text
mOTA 的固件管理层能找到 APP 分区，并能通过 BSP Flash 操作到 STM32F4 内部 Flash。
```

## 2.9 简化 bootloader_port.c

mOTA core 需要以下移植接口：

```c
void Bootloader_Port_Init(void);
void Bootloader_Port_HostDataProcess(void);
void Bootloader_Port_Reset(void);
void Bootloader_Port_JumpToAPP(void);
void Bootloader_Port_SystemReset(void);
```

原版 `bootloader_port.c` 会依赖：

```text
BSP_UART
BSP_Timer
DataTransfer
ProtocolParser
YModem
```

但阶段二不接串口和 YModem，所以当前简化成：

```text
Bootloader_Port_Init             空实现
Bootloader_Port_HostDataProcess  空实现
Bootloader_Port_Reset            空实现
Bootloader_Port_JumpToAPP        保留真实 APP 跳转
Bootloader_Port_SystemReset      保留系统复位
```

这样做的目的：

```text
先让 mOTA core 编译进工程。
先验证 Flash 链路。
避免阶段二被 UART/YModem 依赖拖住。
```

后续阶段三接串口时，再逐步恢复：

```text
Bootloader_Port_Init 初始化 UART/Timer/DT/PP。
Bootloader_Port_HostDataProcess 接收串口数据并交给协议层。
Bootloader_Port_Reset 清空协议和接收状态。
```

## 2.10 为了阶段二补的占位头文件

当前为了编译通过，补了这些头文件：

```text
MOTA/Core/data_transfer.h
MOTA/Core/protocol_parser.h
MOTA/Bsp/inc/bsp_board.h
MOTA/Bsp/inc/bsp_timer.h
MOTA/Bsp/inc/bsp_uart.h
MOTA/Bsp/inc/crcLib.h
MOTA/Bsp/inc/perf_counter.h
MOTA/Bsp/inc/user.h
```

它们目前的作用：

```text
让 bootloader.h / common.h / bsp_common.h 能 include 成功。
让相关类型、宏、函数声明存在。
避免阶段二因为缺文件而编译失败。
```

它们不等于功能已经实现。

例如：

```text
data_transfer.h       只是声明数据传输层相关类型和接口。
protocol_parser.h     只是声明协议解析/YModem 相关命令和结果类型。
bsp_uart.h            只是串口 BSP 抽象声明。
bsp_timer.h           只是定时器抽象声明。
perf_counter.h        当前主要用于满足版本宏和 include。
crcLib.h              当前主要用于满足 common.h include。
```

只要 `bootloader_port.c` 不真正调用这些模块，阶段二可以只有 `.h`。

后续一旦调用，例如：

```c
BSP_UART_Init();
DT_Init();
PP_Config();
```

就必须补对应 `.c`，否则会链接错误：

```text
undefined reference to xxx
```

## 2.11 本次阶段二实际解决的问题

### 1. 缺少 user.h

`common.h` 依赖：

```c
#include "user.h"
```

工程没有该文件，于是补了最小 `user.h`。

主要配置：

```text
是否启用断言
是否启用 debug print
是否使用 RTT
是否使用 RTOS
MAX_NAME_LEN
```

### 2. RTOS 宏误触发

原始写法类似：

```c
#if (USING_RTOS_TYPE == RTOS_USING_RTTHREAD)
```

如果 `RTOS_USING_RTTHREAD` 没定义，预处理器可能把它当成 0，导致误进入 RTOS 分支。

修复方式：

```c
#if defined(RTOS_USING_RTTHREAD) && (USING_RTOS_TYPE == RTOS_USING_RTTHREAD)
```

`bsp_uart.h` 里的 RTOS 判断也要这样处理。

### 3. GCC 下 update_flag 放置问题

原始 mOTA 没匹配当前 GCC 工具链，导致报错：

```text
variable placement not supported for this compiler
update_flag undeclared
```

阶段二补了 GCC 分支：

```c
volatile uint64_t update_flag __attribute__((section(".noinit"), aligned(8)));
```

这能让当前工程编译通过。

后续如果真的使用 APP 设置标志位升级，还要继续检查 linker script 的 `.noinit`。

### 4. bootloader_port.c 重复定义

曾经同时存在：

```text
MOTA/Core/bootloader_port.c
MOTA/Port/bootloader_port.c
```

它们都定义：

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

最终处理：

```text
保留 MOTA/Core/bootloader_port.c
删除 MOTA/Port/bootloader_port.c
```

### 5. CMake 收集了重复源文件

当前 CMake 使用类似：

```cmake
file(GLOB_RECURSE SOURCES
    "Core/*.*" "Drivers/*.*"
    "MOTA/Core/*.*"
    "MOTA/BSP/src/*.*"
    "MOTA/Port/*.*"
)
```

只要 `.c` 在这些路径下，就可能参与编译。

所以后续如果出现重复定义，要检查：

```text
是不是两个 .c 都被 GLOB 收进去了。
```

## 2.12 修改 CMakeLists.txt

当前需要加入 mOTA include 路径：

```cmake
include_directories(
    MOTA/Core
    MOTA/Config
    MOTA/BSP/inc
    MOTA/Port
)
```

当前需要加入 mOTA 源文件：

```cmake
file(GLOB_RECURSE SOURCES
    "Core/*.*" "Drivers/*.*"
    "MOTA/Core/*.*"
    "MOTA/BSP/src/*.*"
    "MOTA/Port/*.*"
)
```

注意：

```text
阶段二不要把还没处理好的 demo 全量 .c 都丢进来。
否则很容易出现重复定义、缺依赖、RTOS/FAL/SFUD/AES 误启用等问题。
```

## 2.13 阶段二当前真实状态

当前工程可以认为：

```text
mOTA core 已加入工程：是
bootloader_config.h 已适配 1MB Flash：是
ONE_PART_PROJECT：是
内部 Flash BSP 链路：已接入
Flash Port：已实现 STM32F4 read/write/erase
bootloader_port.c：阶段二简化版
串口：未接入
YModem：未接入
工程编译：已通过
Flash 擦写读回：建议补测
```

所以当前可以说：

```text
阶段二编译目标已经通过。
```

但如果要说阶段二功能完整通过，还建议做 Flash 擦写读回测试。

## 2.14 阶段二 Flash 功能验证

建议临时写一个测试，只操作 APP 区。

测试流程：

```text
1. 擦除 APP 起始地址所在 Sector：0x08010000。
2. 写入几个 32bit 测试数据。
3. 读回数据。
4. 断点观察读回数据是否一致。
```

测试地址：

```text
0x08010000
```

测试数据示例：

```text
0x11223344
0x55667788
0xA5A5A5A5
```

注意：

```text
测试会擦除 APP 区。
如果 APP 区里有重要程序，不要直接执行。
```

验证通过后，可以认为：

```text
阶段二编译通过 + Flash 擦写读回通过。
```

这时阶段二才算功能上也比较稳。

## 阶段二验收标准

- [x] mOTA core 已加入工程。
- [x] `bootloader_config.h` 已适配 STM32F407ZGTx 1MB Flash。
- [x] 当前配置为 `ONE_PART_PROJECT`。
- [x] 第一版未启用 SPI Flash / FAL / SFUD。
- [x] 第一版未启用 AES。
- [x] `BSP_Flash_Read/Write/Erase` 已接到底层 `read/write/erase`。
- [x] `flash_port_stm32f4.c` 已限制只能操作 APP 区。
- [x] 工程完整编译通过。
- [ ] `BSP_Flash_Read()` 实机读取验证。
- [ ] `BSP_Flash_Erase()` 实机擦除 APP 区验证。
- [ ] `BSP_Flash_Write()` 实机写入 APP 区验证。
- [ ] 写入后读回校验一致。
- [ ] Bootloader 区不会被擦除或写坏。

Flash 没验证通过前，不建议直接进入完整 YModem。

---

# 阶段三：移植串口 / YModem 接收并跑通 fpk 升级

## 阶段目标

第三阶段接入完整升级流程：

```text
Bootloader 上电等待升级
周期发送字符 C
上位机通过 YModem-1K 发送 APP.fpk
Bootloader 接收 fpk
解析 fpk 表头
写入 APP 区
校验固件
跳转 APP
```

## 3.1 建议先接串口，不急着接 YModem

后续可以先做一个小阶段：

```text
只接串口，不接 YModem。
```

原因：

```text
串口是底层通道。
YModem 是上层协议。
```

建议顺序：

```text
1. UART 初始化正常。
2. UART 能发送日志。
3. UART 能接收字节。
4. UART 能回显收到的数据。
5. 再接 data_transfer。
6. 最后接 protocol_parser / YModem。
```

## 3.2 串口分配建议

当前 `XY_bootloadr` 已有：

```text
USART1：日志打印
USART2：YModem 固件接收，建议使用 DMA RX
```

建议第一版：

```text
USART1：日志
USART2：YModem
```

## 3.3 后续需要移植的模块

后续接 YModem 时需要补齐：

```text
data_transfer.c / data_transfer.h
protocol_parser.c / protocol_parser.h
bsp_uart.c / bsp_uart.h
bsp_timer.c / bsp_timer.h
```

当前阶段二只有部分 `.h`，还没有完整 `.c`，这是正常的。

## 3.4 接入 Bootloader_Init 和 Bootloader_Loop

最终主循环建议变成：

```c
HAL_Init();
SystemClock_Config();

MX_GPIO_Init();
MX_DMA_Init();
MX_USART1_UART_Init();
MX_USART2_UART_Init();

Bootloader_Init();

while (1) {
    Bootloader_Loop();
}
```

注意：

```text
Bootloader_Loop() 必须不间断循环调用。
不要在主循环里加入长时间 HAL_Delay()。
```

## 3.5 使用 fpk 进行升级测试

测试流程：

```text
1. 编译 APP 工程，生成 APP.bin。
2. 使用 Firmware_Packager 打包 APP.bin。
3. 表头尺寸选择 1024 byte。
4. 生成 APP.fpk。
5. 使用 YModem_Sender 发送 APP.fpk。
6. Bootloader 接收并写入 APP 区。
7. 校验成功后跳转 APP。
```

注意：

```text
不要直接发送 APP.bin。
mOTA 默认流程要求发送 APP.fpk。
```

## 阶段三验收标准

- [ ] USART2 作为 YModem 接收口可用。
- [ ] USART1 作为日志口可用。
- [ ] USART2 DMA RX 正常工作。
- [ ] USART2 IDLE 中断可以触发一帧结束。
- [ ] BSP Timer 已接入 SysTick。
- [ ] Bootloader 可以周期发送字符 `'C'`。
- [ ] YModem_Sender 可以发送 `.fpk` 文件。
- [ ] `.fpk` 表头尺寸为 1024 byte。
- [ ] Bootloader 可以收到 SOH/STX/EOT。
- [ ] 固件包头校验通过。
- [ ] APP 区 Sector4 ~ Sector11 可以正确擦除。
- [ ] 固件数据写入 `0x08010000` 后读回正常。
- [ ] 固件完整性校验通过。
- [ ] 升级完成后可以跳转 APP。
- [ ] 重启后可以识别 APP 有效并跳转 APP。
- [ ] 升级失败不会破坏 Bootloader 自身。

---

# 四、最终完成标准

完整移植完成后，应达到：

```text
1. Bootloader 位于 0x08000000，大小控制在 64KB 内。
2. APP 位于 0x08010000。
3. APP 设置 SCB->VTOR = 0x08010000，并重新开启中断。
4. Bootloader 上电后等待 YModem 升级。
5. 未收到升级数据时，可以跳转已有 APP。
6. 收到 APP.fpk 后，可以写入 APP 区。
7. 写入完成后校验 APP。
8. 校验成功后跳转 APP。
9. APP 不存在或损坏时，停留 Bootloader 等待升级。
10. Bootloader 自身不会被擦除或写坏。
```

---

# 五、每阶段完成后检查内容

## 阶段一完成后检查

```text
链接脚本
APP 地址
跳转函数
SCB->VTOR
__enable_irq
是否可能 HardFault
```

## 阶段二完成后检查

```text
bootloader_config.h
Flash 分区
read/write/erase
是否会误擦 Bootloader
CMake include/source
mOTA core 编译依赖
占位 .h 是否只是声明，没有被误认为已经实现
```

## 阶段三完成后检查

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
1. Bootloader 限制 64KB。
2. APP 改到 0x08010000。
3. 跑通 Bootloader 跳转 APP。
4. 移植 mOTA core。
5. 适配 bootloader_config.h。
6. 实现片内 Flash read/write/erase。
7. 让阶段二先编译通过。
8. 验证 APP 区 Flash 擦写读回。
9. 先接串口收发，不急着接 YModem。
10. 移植 BSP UART / Timer。
11. USART2 DMA + IDLE 跑通。
12. 接 data_transfer / protocol_parser。
13. 发送 APP.fpk 测试升级。
14. 校验成功后跳转 APP。
```

不要跳过阶段一和阶段二直接做阶段三。
