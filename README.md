# XY7320 STM32F407 Bootloader 串口升级工程

本仓库是 `STM32F407ZGTx` 的 Bootloader + APP 双工程，当前重点是通过串口把 APP 固件发送到 Bootloader，由 Bootloader 擦写 APP 分区，校验 APP 首地址后跳转运行。当前仓库还包含一个正在逐步分层重构的 APP 工程，以及对应的 Qt 串口升级上位机。

## 工程结构

```text
XY7320_ST/
├─ XY_bootloadr/                 Bootloader 工程
├─ XY7320/                       APP 工程，链接地址 0x08010000
├─ tools/XY7320Host/             Qt6 + CMake + QML + C++ 串口升级上位机
├─ docs/
│  ├─ project-docs/              项目记录、踩坑、结构说明
│  ├─ cmake-stm32/               STM32 CMake、链接脚本、启动流程笔记
│  └─ mota/                      mOTA 移植规划和串口升级流程
└─ README.md
```

根目录 `mOTA/` 是官方/参考源码目录，本仓库默认不提交它，避免把大体积参考仓库混进项目源码。

## APP 当前结构

`XY7320/` 当前已经不再是单纯的 CubeMX 平铺结构，而是开始过渡到分层式 APP 架构：

```text
XY7320/
├─ App/
├─ BSP/
├─ Common/
├─ Core/
├─ Domain/
├─ Drivers/
├─ Middleware/
├─ Services/
└─ System/
```

当前已落地的关键模块包括：

- `App/usecase/task_led.cpp`：LED 周期任务
- `App/usecase/task_update.cpp`：升级轮询任务
- `Services/led_service/`：LED 服务
- `BSP/gpio/`：GPIO 板级适配
- `BSP/tim/`：TIM14 驱动 OS Tick
- `System/scheduler/`：轻量级协作式调度器

当前 `main.c` 已经收敛为系统装配入口，主要负责外设初始化、OS 初始化、升级接收初始化、App 初始化和启动任务调度。

## Flash 分区

| 分区 | 起始地址 | 大小 | 说明 |
|---|---:|---:|---|
| Bootloader | `0x08000000` | `64KB` | Sector 0~3 |
| APP | `0x08010000` | `960KB` | Sector 4~11 |

APP 工程必须链接到 `0x08010000`。APP 启动后建议在 `main()` 早期设置：

```c
SCB->VTOR = 0x08010000U;
__enable_irq();
```

## 当前升级协议

上位机先发送 12 字节小端头包：

```text
magic    4 字节：0x41505055，对应 HEX 为 55 50 50 41
app_size 4 字节：APP bin 文件大小
crc32    4 字节：APP bin 文件 CRC32
```

Bootloader 收到头包后擦除 APP 分区，打印 `OK` 后等待 APP 原始 bin 数据。上位机按设置的包大小和包间隔继续发送固件数据。

当前 APP / Bootloader / 上位机之间还引入了固定码握手流程：

```text
XYA1  APP 已接收版本帧，即将写配置并复位
XYB1  Bootloader 已进入升级模式，USART2 已准备接收
XYB2  Bootloader 已接收头包并擦除 APP 区，正在等待 APP 数据
XYB3  Bootloader 已接收完 APP，校验有效，即将跳转 APP
```

当前 Qt 上位机界面会自动计算并显示头包 HEX，方便手动比对，例如：

```text
55 50 50 41 18 13 00 00 D8 22 25 85
```

## 已处理的问题

### 1. Bootloader 接收数据被覆盖

之前 USART2 DMA 接收缓冲和 Flash 写入处理共用同一个 buffer，主循环写 Flash 时，DMA 可能已经把下一包数据覆盖进来，导致 APP 首地址被写坏。

现在采用两级 buffer：

- `uart2_rx_buf`：DMA 接收专用
- `uart2_proc_buf`：主循环业务处理专用

IDLE 中断里先停止 DMA，计算长度，把 DMA buffer 快速复制到处理 buffer，然后立即恢复 DMA 接收。主循环只处理 `uart2_proc_buf`。

### 2. APP 从 Bootloader 跳转后卡在 `__libc_init_array`

CLion + arm-none-eabi-gcc 使用 GCC/newlib 启动链路，和 Keil 的启动/运行库不同。当前 APP CMake 已补充裸机链接配置：

```cmake
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)
add_link_options(--specs=nano.specs --specs=nosys.specs -nostartfiles)
```

同时新增 `minimal_runtime.c` 提供空的 `_init()` / `_fini()`，这样可以保留启动文件里的 `bl __libc_init_array`，后续 APP 接入 C++ 全局对象或 FreeRTOS 时更稳。

### 3. APP 后续接入 RTOS

后续 APP 如果会上 FreeRTOS，要特别注意：

- APP 自己设置 `VTOR`
- Bootloader 跳转前清理 SysTick、NVIC、DMA、外设中断
- `SysTick / PendSV / SVC` 必须来自 APP 向量表
- 中断优先级要符合 `configMAX_SYSCALL_INTERRUPT_PRIORITY`
- 重新规划 MSP、任务栈和 FreeRTOS heap

### 4. APP 升级逻辑已挂入任务调度框架

当前 APP 侧升级接收与处理链路如下：

```text
main.c
  -> App_UpdateConfig_StartReceive()
  -> App_Main_Init()
  -> App_Man_Start()
      -> Task_UpdateConfig()
          -> App_UpdateConfig_Poll()
```

串口接收采用 `USART2 + DMA + IDLE`：

- `HAL_UART_Receive_DMA()` 在 `App_UpdateConfig_StartReceive()` 中开启
- `USART2_IRQHandler()` 中调用 `App_UpdateConfig_HandleUartIdleIrq()`
- IDLE 中断里复制接收缓冲区、置位完成标志，再由任务上下文处理版本帧

这说明升级逻辑已经进入任务化框架，但 `App_UpdateConfig_Poll()` 内部仍处于过渡态，后续还可以继续拆分。

### 5. APP 与 Bootloader 的版本配置模块已开始分离

当前仓库不再继续共用根目录 `Common/app_version_config.*`。

版本配置代码已分别放在：

```text
XY7320/Core/Src/app_version_config.c
XY_bootloadr/Core/Src/app_version_config.c
```

这样做是为了避免 APP 与 Bootloader 在后续演进中继续强耦合到同一个源码实现文件。

## 文档入口

| 文档 | 内容 |
|---|---|
| [项目笔记.md](docs/project-docs/项目笔记.md) | 后续开发约束、Bootloader 跳 APP、RTOS 注意事项 |
| [踩坑记录.md](docs/project-docs/踩坑记录.md) | 本轮升级调试复盘，包括 `__libc_init_array` 和 DMA 覆盖问题 |
| [开发日志.md](docs/project-docs/开发日志.md) | 项目阶段性开发记录 |
| [项目结构.md](docs/project-docs/项目结构.md) | 仓库结构说明 |
| [嵌入式项目分层架构规范.md](docs/cmake-stm32/嵌入式项目分层架构规范.md) | 当前 APP 分层设计原则与落地约束 |
| [STM32_CMake笔记.md](docs/cmake-stm32/STM32_CMake笔记.md) | STM32 CMake 工程配置笔记 |
| [链接脚本与启动流程.md](docs/cmake-stm32/链接脚本与启动流程.md) | Flash 分区、链接脚本、启动与跳转流程 |
| [串口移植与升级流程规划.md](docs/mota/串口移植与升级流程规划.md) | 串口升级流程规划 |
| [XY7320Host串口升级上位机说明.md](docs/qt/XY7320Host串口升级上位机说明.md) | Qt 上位机串口、手动发包、UI 布局和构建说明 |

## 构建提示

Bootloader 和 APP 都是 CLion / STM32CubeCLT / arm-none-eabi-gcc 工程。

```powershell
cmake --build XY_bootloadr/cmake-build-debug
cmake --build XY7320/cmake-build-debug
```

Qt 上位机位于：

```text
tools/XY7320Host/
```

使用 Qt6 + CMake 构建，需要本机安装 Qt 串口模块 `Qt6::SerialPort`。
