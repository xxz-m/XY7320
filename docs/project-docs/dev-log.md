# 编写日志

## 2026-06-03：mOTA 阶段二编译通过

本次目标：

```text
在 STM32F407 Bootloader 工程 XY_bootloadr 中接入 mOTA core 和 Flash 分区管理，让阶段二先编译通过。
```

本阶段暂时不做：

```text
串口接入
YModem 接入
完整协议解析
上位机升级流程
```

## 已完成事项

### 1. 接入 mOTA 目录

当前 mOTA 放在：

```text
XY_bootloadr/MOTA/
```

目录分为：

```text
MOTA/Core
MOTA/Config
MOTA/Bsp
MOTA/Port
```

CMake 中已经把相关目录加入 include 和 sources。

### 2. 配置 Bootloader 和 APP 分区

当前 Bootloader 占用 64KB：

```text
Bootloader: 0x08000000 ~ 0x0800FFFF
APP:        0x08010000 ~ 0x080FFFFF
```

关键配置：

```c
#define ONCHIP_FLASH_SIZE   (1024 * 1024)
#define BOOTLOADER_SIZE     (64 * 1024)
#define APP_PART_SIZE       (ONCHIP_FLASH_SIZE - BOOTLOADER_SIZE)
```

Linker script 中 Bootloader FLASH 长度也限制为 64KB。

### 3. 简化 bootloader_port.c

为了阶段二编译通过，当前只保留最小移植接口：

```text
Bootloader_Port_Init：空实现
Bootloader_Port_HostDataProcess：空实现
Bootloader_Port_Reset：空实现
Bootloader_Port_JumpToAPP：保留真实跳转逻辑
Bootloader_Port_SystemReset：保留系统复位逻辑
```

这不是最终实现。后续接串口和 YModem 时，这个文件会继续补全。

### 4. 接入 Flash Port

新增 STM32F4 Flash 适配：

```text
MOTA/Port/flash_port_stm32f4.c
```

实现：

```c
read()
write()
erase()
```

用于连接 mOTA 的 `BSP_Flash_Read/Write/Erase`。

### 5. 补齐阶段二编译依赖头文件

为了让 mOTA 原始头文件依赖能过编译，补了若干 `.h` 文件。

这些文件目前很多只是占位或声明，不代表对应功能已经完整实现。

主要包括：

```text
data_transfer.h
protocol_parser.h
bsp_uart.h
bsp_timer.h
bsp_board.h
crcLib.h
perf_counter.h
user.h
```

## 遇到并解决的问题

### user.h 缺失

`common.h` 包含 `user.h`，工程里没有，于是补了最小配置。

### RTOS 头文件误包含

因为未定义宏在 `#if` 中会被当成 0，导致误进入 RTOS 分支。

修复方式：

```c
#if defined(RTOS_USING_RTTHREAD) && (USING_RTOS_TYPE == RTOS_USING_RTTHREAD)
```

### GCC 不支持原始 update_flag 放置写法

补了 GCC 分支：

```c
volatile uint64_t update_flag __attribute__((section(".noinit"), aligned(8)));
```

### bootloader_port.c 重复定义

曾同时存在两个 `bootloader_port.c`，导致链接重复定义。

最终保留：

```text
MOTA/Core/bootloader_port.c
```

删除重复文件：

```text
MOTA/Port/bootloader_port.c
```

## 当前结论

当前阶段二编译目标已经通过。

下一步建议：

```text
1. 做 APP 区 Flash 擦写读回测试。
2. 再开始接串口。
3. 串口稳定后再接 YModem。
```
