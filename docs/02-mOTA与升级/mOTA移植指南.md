---
title: mOTA 移植指南
module: mota
type: guide
tags: [mota, bootloader, flash, type/guide]
created: 2026-06-11
related: "[[升级协议概述]], [[Bootloader开发指南]], [[硬件参考]], [[mOTA移植验收清单]]"
---

# mOTA 移植指南

> [!info] 文档定位
> 本文档提取自 mOTA 移植过程中的**永久性参考内容**，去除了一次性编译排错记录。重复的分区表、跳转流程等内容使用 wikilink 替代，避免重复维护。

## 移植原则

### 1. Bootloader 以简单稳定为主

Bootloader 是救援程序，不是业务程序。第一版不要追求功能完整，先保证：

```text
能启动
能判断 APP
能跳转 APP
能接收固件
能写 APP 区
失败时不擦坏自己
```

第一版建议**不要加入**：

```text
RTOS、复杂 C++、外部 SPI Flash、factory 分区、
恢复出厂、复杂加密、复杂日志系统
```

### 2. 升级的是 fpk 而非 bin

mOTA 默认不是直接发送普通 `.bin`，而是需要打包：

```text
APP.bin
  ↓  Firmware_Packager 打包
APP.fpk
  ↓  YModem_Sender 发送
Bootloader 接收
```

> [!caution] 测试注意
> 阶段三测试时**不要直接发 APP.bin**，应该发 **APP.fpk**。

### 3. YModem-1K 表头尺寸选 1024 byte

本案例采用 YModem-1K 协议，固件打包器表头尺寸必须选择 **1024 byte**。

原因：

```text
YModem-1K 每帧数据区是 1024 byte。
mOTA 希望固件包头单独成一帧发送。
所以 fpk 表头也要设置为 1024 byte。
```

否则固件包头解析可能失败。

### 4. APP 必须改地址

Bootloader 在 `0x08000000`，APP 不再从默认地址运行，而是从 `0x08010000` 开始。

APP 侧需要：

```text
1. 修改链接脚本，让 APP 从 0x08010000 开始
2. APP 启动时设置 SCB->VTOR = 0x08010000
3. APP 侧重新打开全局中断 __enable_irq()
```

详见 [[Bootloader开发指南#启动跳转流程]]。

### 5. 第一版不依赖 update_flag

mOTA 支持两种进入升级方式：

```text
USING_HOST_CMD_UPDATE      上位机命令触发
USING_APP_SET_FLAG_UPDATE  APP 设置标志位后复位进入 Bootloader
```

> [!warning] GCC 下的风险
> 当前工程是 GCC + CMake，不是 Keil AC5/AC6。GCC 下 `__attribute__((section(".noinit")))` 不一定真的固定在 `0x20000000`。如果链接脚本没有明确 `.noinit` 段的位置，APP 和 Bootloader 可能读写的不是同一个地址。

第一版完整升级流程跑通前，建议优先用**主机命令触发升级**；等流程稳定后再考虑 `update_flag + .noinit`。详见 [[升级协议概述#升级标志机制]]。

---

## Flash 分区表

![[硬件参考#Flash 分区表]]

---

## 目录结构与文件说明

### 目录结构

```text
XY_bootloadr/
└─ MOTA/
   ├─ Core/          mOTA 核心逻辑（bootloader 状态机、固件管理、协议解析）
   ├─ Config/        配置文件（bootloader_config.h，分区方案、功能开关）
   ├─ Bsp/
   │  ├─ inc/        BSP 抽象层头文件（Flash、UART、Timer、Board 等声明）
   │  └─ src/        BSP 抽象层实现（当前主要是 bsp_flash.c）
   └─ Port/          硬件移植层（flash_port_stm32f4.c、bootloader_port.c）
```

### 核心文件清单

**阶段二真实使用的核心文件：**

| 文件 | 职责 |
|---|---|
| `MOTA/Core/bootloader.c` | Bootloader 状态机主循环 |
| `MOTA/Core/bootloader.h` | Bootloader 对外接口声明 |
| `MOTA/Core/bootloader_define.h` | 地址宏、分区地址计算 |
| `MOTA/Core/firmware_manage.c` | 固件管理：分包写入、完整性校验 |
| `MOTA/Core/firmware_manage.h` | 固件管理接口声明 |
| `MOTA/Core/bootloader_port.c` | 移植适配层：初始化、串口数据处理、跳转、复位 |
| `MOTA/Config/bootloader_config.h` | 分区方案选择、功能配置开关 |
| `MOTA/Bsp/inc/bsp_flash.h` | Flash BSP 抽象层头文件 |
| `MOTA/Bsp/src/bsp_flash.c` | Flash BSP 抽象层实现，依赖底层 read/write/erase |
| `MOTA/Port/flash_port_stm32f4.c` | STM32F4 Flash 硬件移植，实现 read/write/erase |

**后续接串口/YModem 需要补实现的文件：**

| 文件 | 职责 |
|---|---|
| `MOTA/Bsp/inc/bsp_uart.h` / `bsp_uart.c` | 串口 BSP 抽象 |
| `MOTA/Bsp/inc/bsp_timer.h` / `bsp_timer.c` | 定时器 BSP 抽象 |
| `MOTA/Core/data_transfer.h` / `data_transfer.c` | 数据传输层 |
| `MOTA/Core/protocol_parser.h` / `protocol_parser.c` | 协议解析 / YModem |

### 占位头文件说明

当前为了编译通过，补了一批占位头文件：

```text
MOTA/Core/data_transfer.h        数据传输层类型和接口声明
MOTA/Core/protocol_parser.h      协议解析/YModem 命令和结果类型声明
MOTA/Bsp/inc/bsp_board.h         板级配置声明
MOTA/Bsp/inc/bsp_timer.h         定时器抽象声明
MOTA/Bsp/inc/bsp_uart.h          串口 BSP 抽象声明
MOTA/Bsp/inc/crcLib.h            CRC 库声明（满足 common.h include）
MOTA/Bsp/inc/perf_counter.h      性能计数器（版本宏和 include）
MOTA/Bsp/inc/user.h              用户配置（断言、debug、RTOS 类型等）
```

> [!important] 占位不等于实现
> 这些占位头文件的作用是让阶段二先编译通过，**不代表功能已经完整实现**。只要 `bootloader_port.c` 不真正调用这些模块，阶段二可以只有 `.h`。后续一旦调用（如 `BSP_UART_Init()`、`DT_Init()`），就必须补对应 `.c`，否则链接报错 `undefined reference to xxx`。

---

## bootloader_config.h 配置说明

### 分区方案选择

第一版使用单分区，便于快速跑通：

```c
#define USING_PART_PROJECT                  ONE_PART_PROJECT
```

可选方案：

| 宏值 | 说明 |
|---|---|
| `ONE_PART_PROJECT` | 单分区：仅 Bootloader + APP |
| `TWO_PART_PROJECT` | 双分区：Bootloader + APP + Download |
| `THREE_PART_PROJECT` | 三分区：Bootloader + APP + Download + Factory |

### 关键配置项

```c
// Flash 基本参数
#define ONCHIP_FLASH_SIZE                   (1024 * 1024)   // STM32F407ZGTx 1MB Flash
#define BOOTLOADER_SIZE                     (64 * 1024)     // Bootloader 64KB

// APP 分区
#define APP_PART_SIZE                       (ONCHIP_FLASH_SIZE - BOOTLOADER_SIZE)  // 960KB
#define DOWNLOAD_PART_SIZE                  0               // 第一版无 download 区
#define FACTORY_PART_SIZE                   0               // 第一版无 factory 区

// 功能开关
#define ENABLE_DECRYPT                      0               // 第一版不启用解密
```

地址计算结果：

```text
APP_ADDRESS = FLASH_BASE + BOOTLOADER_SIZE
            = 0x08000000 + 0x10000
            = 0x08010000
```

### bootloader_define.h 中的地址关系

```c
#define ONCHIP_FLASH_END_ADDRESS ((uint32_t)(FLASH_BASE + ONCHIP_FLASH_SIZE))
#define APP_ADDRESS              ((uint32_t)(FLASH_BASE + BOOTLOADER_SIZE))
#define DOWNLOAD_ADDRESS         ((uint32_t)(APP_ADDRESS + APP_PART_SIZE))
#define FACTORY_ADDRESS          ((uint32_t)(DOWNLOAD_ADDRESS + DOWNLOAD_PART_SIZE))
```

单分区方案下，真正关心的只有 `APP_ADDRESS` 和 `APP_PART_SIZE`。只要 `BOOTLOADER_SIZE = 64KB`，APP 起始地址就是 `0x08010000`。

---

## BSP Flash 移植

### bsp_flash.h/c 的 Flash 抽象

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

`bsp_flash.c` 是 mOTA 的 Flash 抽象层，不直接操作 STM32 Flash，而是依赖三个底层函数：

```c
extern int read(long offset, uint8_t *buf, size_t size);
extern int write(long offset, const uint8_t *buf, size_t size);
extern int erase(long offset, size_t size);
```

### flash_port_stm32f4.c 实现要求

需要实现上述三个函数，注意**不能写成 `static`**（`bsp_flash.c` 通过 `extern` 声明寻找这些函数）：

**read**：可以直接从 Flash 地址读取

```c
memcpy(buf, (const void *)offset, size);
```

要求：buf 不为空、size 不为 0、地址范围在 APP 区内。

**write**：STM32F407 片内 Flash 按 32bit 写入

```c
HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data32);
```

要求：地址 4 字节对齐、写入前已擦除、写入范围不越过 APP 区、写入后读回校验。

**erase**：STM32F407 按 Sector 擦除

```text
APP 区从 0x08010000 开始，对应 Sector4
只允许擦除 Sector4 ~ Sector11
严禁擦除 Sector0 ~ Sector3（Bootloader 区）
```

### Flash 保护逻辑

```c
#define APP_START_ADDR  0x08010000U
#define FLASH_END_ADDR  0x08100000U
```

所有操作必须检查地址范围，严禁擦写 Bootloader 区。

### mOTA Flash 调用链

```text
Bootloader_Init()
   ↓
FM_Init()
   ↓
BSP_Flash_Init(&_flash_app_part, APP_PART_NAME, APP_ADDRESS, APP_PART_SIZE)
```

固件写入调用链：

```text
firmware_manage.c
   ↓
BSP_Flash_Read / BSP_Flash_Write / BSP_Flash_Erase
   ↓
read / write / erase（flash_port_stm32f4.c）
   ↓
HAL_FLASH_Program / HAL_FLASHEx_Erase
```

---

## bootloader_port.c 移植

### 移植接口

mOTA core 需要以下移植接口：

```c
void Bootloader_Port_Init(void);
void Bootloader_Port_HostDataProcess(void);
void Bootloader_Port_Reset(void);
void Bootloader_Port_JumpToAPP(void);
void Bootloader_Port_SystemReset(void);
```

### 阶段二简化版

阶段二不接串口和 YModem，简化为：

| 函数 | 实现 |
|---|---|
| `Bootloader_Port_Init` | 空实现 |
| `Bootloader_Port_HostDataProcess` | 空实现 |
| `Bootloader_Port_Reset` | 空实现 |
| `Bootloader_Port_JumpToAPP` | 保留真实 APP 跳转 |
| `Bootloader_Port_SystemReset` | 保留系统复位 |

目的：先让 mOTA core 编译进工程，先验证 Flash 链路，避免被 UART/YModem 依赖拖住。

### 完整版（阶段三/四）

后续接串口时，逐步恢复：

```c
void Bootloader_Port_Init(void)
{
    BSP_UART_Init(BSP_UART1);       // 日志
    BSP_UART_Init(BSP_UART2);       // 升级接收
    BSP_UART_EnableReceive(...);    // 启动接收
    DT_Init(...);
    PP_Init(...);
}

void Bootloader_Port_HostDataProcess(void)
{
    if (BSP_UART_IsFrameEnd(BSP_UART2) == BSP_UART_ERR_OK)
    {
        // 取出一帧串口数据
        // 交给 data_transfer / protocol_parser
        // 推动 mOTA 状态机
    }
}
```

> [!note] 注意事项
> 具体函数名以 mOTA 原始工程为准。`bootloader_port.c` 只保留一份，位于 `MOTA/Core/bootloader_port.c`，不要在其他路径重复创建。

---

## 串口移植步骤

### USART1 printf 重定向（日志口）

使用 `_write()` 重定向方式：

```c
#include "usart.h"
#include <stdio.h>

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, 1000);
    return len;
}
```

验证：`printf("bootloader start\r\n");`，串口工具能看到日志即通过。

### USART2 DMA + IDLE 接收（升级口）

**1. 定义接收缓存：**

```c
#define UART2_RX_BUF_SIZE 1024

uint8_t uart2_rx_buf[UART2_RX_BUF_SIZE];      // DMA 专用
uint8_t uart2_proc_buf[UART2_RX_BUF_SIZE];    // 业务处理专用
volatile uint16_t uart2_rx_len = 0;
volatile uint8_t uart2_rx_done = 0;
```

**2. 启动接收：**

```c
HAL_UART_Receive_DMA(&huart2, uart2_rx_buf, UART2_RX_BUF_SIZE);
__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
```

**3. IDLE 中断处理：**

```c
void USART2_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);
        HAL_UART_DMAStop(&huart2);

        uart2_rx_len = UART2_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);

        memcpy(uart2_proc_buf, uart2_rx_buf, uart2_rx_len);
        uart2_rx_done = 1;

        HAL_UART_Receive_DMA(&huart2, uart2_rx_buf, UART2_RX_BUF_SIZE);
    }
    HAL_UART_IRQHandler(&huart2);
}
```

> [!tip] 双 buffer 要点
> 使用 DMA 双 buffer 方案，将 DMA 接收和业务处理隔离。详见 [[简化升级协议#DMA 双 buffer 方案]]。

---

## 完整 mOTA/YModem 接入

### 需要补齐的模块

当前很多 mOTA 文件还是占位或未完整接入，完整接入需要补齐：

```text
MOTA/Bsp/src/bsp_uart.c           串口 BSP 实现
MOTA/Bsp/src/bsp_timer.c          定时器 BSP 实现
MOTA/Core/data_transfer.c         数据传输层实现
MOTA/Core/protocol_parser.c       协议解析 / YModem 实现
MOTA/Core/bootloader_port.c       从简化版升级为完整版
```

### 完整调用链

```text
USART2 DMA + IDLE
    ↓
BSP_UART
    ↓
data_transfer
    ↓
protocol_parser / YModem
    ↓
Bootloader_Port_HostDataProcess
    ↓
Bootloader_Loop 状态机
    ↓
FM_WriteFirmwareSubPackage
    ↓
BSP_Flash_Write / BSP_Flash_Erase
```

### 主循环接入

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

> [!caution] 关键约束
> `Bootloader_Loop()` 必须不间断循环调用。不要在主循环里加入长时间 `HAL_Delay()`。

### 使用 fpk 进行升级测试

```text
1. 编译 APP 工程，生成 APP.bin
2. 使用 Firmware_Packager 打包 APP.bin（表头尺寸选 1024 byte）
3. 生成 APP.fpk
4. 使用 YModem_Sender 发送 APP.fpk
5. Bootloader 接收并写入 APP 区
6. 校验成功后跳转 APP
```

---

## 裁剪建议

### 按阶段裁剪

```text
阶段二结束：保留当前结构，先验证 Flash
阶段三接串口：实现 UART 相关文件，清理不需要的 UART demo 写法
阶段四接协议：实现 data_transfer 和 protocol_parser
阶段五稳定后：删除没有被 include、没有被调用、没有配置意义的文件
```

### 搜索依赖后再删

> [!important] 裁剪前必须搜索
> 裁剪前不要只看文件名，要看它是否被真实调用：
>
> ```text
> 搜索文件名是否被 include
> 搜索函数名是否被调用
> 搜索宏是否被使用
> ```
>
> 确认没有真实依赖后再删。

### 文件分类

**必须保留**（mOTA core 和 Flash 链路关键文件）：

```text
MOTA/Core/bootloader.c / bootloader.h / bootloader_define.h
MOTA/Core/firmware_manage.c / firmware_manage.h
MOTA/Core/bootloader_port.c
MOTA/Config/bootloader_config.h
MOTA/Bsp/inc/bsp_flash.h
MOTA/Bsp/src/bsp_flash.c
MOTA/Port/flash_port_stm32f4.c
```

**后续需要补实现**（当前可能只是头文件）：

```text
bsp_uart.h / bsp_uart.c
bsp_timer.h / bsp_timer.c
data_transfer.h / data_transfer.c
protocol_parser.h / protocol_parser.c
```

**可按项目风格裁剪**（偏 demo 支撑或公共配置）：

```text
bsp_board.h / bsp_common.h / bsp_config.h
common.h / crcLib.h / perf_counter.h / user.h
bsp_uart_stm32.h
```

---

## 相关文档

- [[升级协议概述]] -- 升级体系总览、串口分配、升级标志机制
- [[Bootloader开发指南]] -- Bootloader 启动跳转流程
- [[硬件参考]] -- Flash 分区表
- [[mOTA移植验收清单]] -- 三阶段验收标准
