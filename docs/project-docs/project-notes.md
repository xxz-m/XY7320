# 项目知识补充

## 阶段二不是最终结构

当前阶段二的结构是为了让 mOTA 先进入工程并编译通过。

它不是最终项目结构。

当前补进来的 `.h` 文件，有些只是为了满足 mOTA 原始代码的 include 依赖。

后续要根据真实使用情况裁剪。

## 如何判断一个文件要不要保留

裁剪时不要只看文件名，要看它是否被真实调用。

### 必须保留

这些是当前 mOTA core 和 Flash 链路的关键文件：

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

### 后续接串口/YModem 大概率要补实现

这些当前可能只是头文件，后续会变成真实模块：

```text
bsp_uart.h / bsp_uart.c
bsp_timer.h / bsp_timer.c
data_transfer.h / data_transfer.c
protocol_parser.h / protocol_parser.c
```

### 可按项目风格裁剪

这些更偏 demo 支撑或公共配置：

```text
bsp_board.h
bsp_common.h
bsp_config.h
common.h
crcLib.h
perf_counter.h
user.h
bsp_uart_stm32.h
```

如果后续自己的工程有更清晰的配置系统，可以把它们合并或删减。

## .h 和 .c 的关系

`.h` 主要放声明，告诉编译器：

```text
有哪些类型
有哪些宏
有哪些函数
```

`.c` 才是真正实现函数逻辑的地方。

阶段二很多 `.h` 已经存在，但没有对应 `.c`，这是因为当前没有真正调用那些函数。

如果后续开始调用，例如：

```c
BSP_UART_Init();
```

但是没有对应实现，就会链接报错：

```text
undefined reference to BSP_UART_Init
```

所以阶段二的占位头文件不是错，只是还没进入实现阶段。

## 串口和 YModem 的关系

串口是底层通道。

YModem 是上层协议。

可以先接串口，不接 YModem。

建议顺序：

```text
1. UART 能初始化。
2. UART 能发送调试信息。
3. UART 能接收字节。
4. UART 能回显收到的数据。
5. 再把收到的数据交给 data_transfer。
6. 最后接 protocol_parser / YModem。
```

## mOTA 的大致层级

```text
PC 上位机
   |
   | 串口字节流
   v
UART 驱动层
   |
   v
Data Transfer 数据传输层
   |
   v
Protocol Parser / YModem 协议层
   |
   v
Firmware Manage 固件管理层
   |
   v
BSP Flash 抽象层
   |
   v
STM32F4 Flash Port
   |
   v
内部 Flash APP 分区
```

阶段二目前做到：

```text
Firmware Manage
   |
   v
BSP Flash
   |
   v
STM32F4 Flash Port
```

串口和协议层还没有接。

## APP 跳转要注意什么

Bootloader 跳转 APP 时，核心动作是：

```text
1. 读取 APP 首地址处的 MSP。
2. 读取 APP 首地址 + 4 处的 Reset_Handler。
3. 关闭中断。
4. 关闭 SysTick。
5. 清除 NVIC 中断使能和挂起状态。
6. 设置 MSP。
7. 设置 VTOR 到 APP 地址。
8. 跳转 APP Reset_Handler。
```

APP 工程也要配合：

```text
APP linker script 的 FLASH ORIGIN 要改成 0x08010000。
APP 的向量表偏移也要设置到 0x08010000。
```

否则 Bootloader 跳过去也可能跑不起来。

## Flash 写入要注意什么

STM32F407 内部 Flash 是按 Sector 擦除。

前几个 Sector 大小不同：

```text
Sector0: 0x08000000, 16KB
Sector1: 0x08004000, 16KB
Sector2: 0x08008000, 16KB
Sector3: 0x0800C000, 16KB
Sector4: 0x08010000, 64KB
Sector5~11: 每个 128KB
```

当前 APP 从 Sector4 开始。

Flash 写入前必须先擦除。

Flash 只能把 bit 从 1 写成 0，不能直接从 0 写回 1。

要从 0 变回 1，必须擦除整个 Sector。

## 后续裁剪建议

不要一次性裁剪。

建议按阶段来：

```text
阶段二结束：保留当前结构，先验证 Flash。
阶段三接串口：实现 UART 相关文件，清理不需要的 UART demo 写法。
阶段四接协议：实现 data_transfer 和 protocol_parser。
阶段五稳定后：删除没有被 include、没有被调用、没有配置意义的文件。
```

裁剪前可以先做搜索：

```text
搜索文件名是否被 include
搜索函数名是否被调用
搜索宏是否被使用
```

确认没有真实依赖后再删。
