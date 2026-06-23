# XY7320 STM32F407 Bootloader 串口升级工程

本仓库是 `STM32F407ZGTx` 的 Bootloader + APP 双工程，当前重点是通过串口把 APP 固件发送到 Bootloader，由 Bootloader 擦写 APP 分区，校验 APP 首地址后跳转运行。当前仓库还包含一个正在逐步分层重构的 APP 工程，以及对应的 Qt 串口升级上位机。

## 工程结构

```text
XY7320_ST/
├─ XY_bootloadr/                 Bootloader 工程
├─ XY7320/                       APP 工程，链接地址 0x08010000
├─ tools/XY7320Host/             Qt6 + CMake + QML + C++ 串口升级上位机
├─ docs/                         Obsidian 知识库
│  ├─ 00-Index/                  索引与总览
│  ├─ 10-项目规范/               架构规范、命名规范、Git 管理
│  ├─ 20-开发记录/               开发日志、踩坑记录、决策记录
│  ├─ 30-工程资料/               STM32、升级协议、系统设计
│  ├─ 40-上位机/                 Qt 上位机相关文档
│  └─ 90-归档/                   旧目录迁移说明
└─ README.md
```

根目录 `mOTA/` 是官方/参考源码目录，本仓库默认不提交它，避免把大体积参考仓库混进项目源码。

## APP 当前结构

`XY7320/` 采用分层式架构：

```text
XY7320/
├─ App/                          应用层（业务编排、任务入口）
├─ BSP/                          板级支持层（硬件驱动封装）
│  ├─ flash/                     内部 Flash 擦/读/写
│  ├─ gpio/                      GPIO 控制
│  ├─ tim/                       TIM14 驱动 OS Tick
│  └─ uart/                      串口 DMA 接收、通用发送
├─ Common/                       跨层共享
│  └─ config/                    配置集中管理
│     ├─ os_config.h             OS 配置
│     ├─ bsp_config.h            硬件配置（串口、GPIO 等）
│     └─ app_config.h            APP 功能配置（版本号、调试开关）
├─ Core/                         CubeMX 生成代码
├─ Domain/                       领域层（纯算法、协议解析）
│  └─ protocol/                  主协议编解码与帧处理
├─ Drivers/                      HAL 驱动
├─ Middleware/                    中间件层（第三方库）
│  └─ nanoprintf/                轻量级 printf
├─ Services/                     服务层（能力翻译、状态封装）
│  ├─ led_service/               LED 服务
│  ├─ log/                       日志服务
│  └─ upgrade/                   升级服务
│     ├─ update_service          升级流程编排
│     └─ version_store           版本配置存储（A1/A2 槽位）
└─ System/                       系统层（调度器、事件队列）
```

### 已落地的关键模块

| 模块 | 位置 | 职责 |
|------|------|------|
| LED 任务 | `App/usecase/task_led.cpp` | LED 周期任务入口 |
| 升级任务 | `App/usecase/task_update.cpp` | 升级轮询任务入口 |
| LED 服务 | `Services/led_service/` | LED 模式管理（OFF/ON/BLINK） |
| 日志服务 | `Services/log/` | nanoprintf + 串口输出 |
| 升级服务 | `Services/upgrade/update_service/` | 升级流程编排 |
| 版本存储 | `Services/upgrade/version_store/` | A1/A2 版本槽位管理 |
| 主协议编解码 | `Domain/protocol/` | 主协议帧编解码与 CRC/转义处理 |
| 串口接收 | `BSP/uart/bsp_uart_rcv.c` | DMA + IDLE 中断接收 |
| Flash 操作 | `BSP/flash/bsp_flash.c` | 内部 Flash 擦/读/写 |

### 配置集中管理

所有配置集中在 `Common/config/`：

- `bsp_config.h` — 硬件配置（串口选择、GPIO 引脚等）
- `app_config.h` — APP 功能配置（版本号、调试开关等）
- `os_config.h` — OS 配置（时基、任务数等）

## Flash 分区

| 分区 | 起始地址 | 大小 | 说明 |
|---|---:|---:|---|
| Bootloader | `0x08000000` | `64KB` | Sector 0~3 |
| APP | `0x08010000` | `960KB` | Sector 4~11 |
| 版本配置 | `0x080E0000` | `128KB` | Sector 11（A1/A2 槽位） |

APP 工程必须链接到 `0x08010000`。APP 启动后设置：

```c
SCB->VTOR = 0x08010000U;
__enable_irq();
```

## 升级协议

### APP 前置升级握手协议

上位机发送主协议升级握手命令，APP 收到后写 A2 槽位、回主协议 ACK、复位进入 Bootloader：

```text
主协议字段：
origin_port = 0x01 (PC)
goal_port   = 0x22 (XY7320)
model       = 0x02 (unWrite)
cmd         = 0xF0
payload     = yyyyMMddHHmm(12位 ASCII) + flag(1字节)

flag:
  0x00 = NEED_DOWNLOAD（需要下载升级）
  0x01 = DOWNLOADED（已下载完成）
```

### 固定码握手流程

```text
主协议 ACK  APP 已接收升级握手命令，即将写配置并复位
XYB1  Bootloader 已进入升级模式，USART2 已准备接收
XYB2  Bootloader 已接收头包并擦除 APP 区，正在等待 APP 数据
XYB3  Bootloader 已接收完 APP，校验有效，即将跳转 APP
```

### 升级流程

```text
1. 上位机发主协议 `0xF0` 升级握手 → APP 收到
2. APP 写 A2（目标版本）→ 回主协议 ACK → 复位
3. Bootloader 启动 → 检测 A2 有效 → 进入升级模式
4. Bootloader 回 XYB1 → 等待头包
5. 上位机发头包 → Bootloader 擦除 APP 区 → 回 XYB2
6. 上位机发 APP 数据 → Bootloader 写入 Flash
7. 校验通过 → 回 XYB3 → 跳转新 APP
```

## 调用链路

### 升级链路

```text
main.c
  └── App_Main_Init()
        └── UpdateService::Init()
              └── VersionStore::WriteA1() # 写当前版本

Task_UpdateConfig (每 10ms)
  └── ProtocolService::Update()
        ├── Protocol::DecodeBuffer()                    # 主协议解包
        ├── DispatchPacket(cmd=0xF0)                   # 分发升级握手命令
        ├── UpdateService::HandleProtocolUpgradeRequest()
        ├── VersionStore::WriteA2()                    # 写目标版本
        └── ResetToBootloaderAfterAck()                # ACK 后复位

USART2_IRQHandler
  └── BspUartRcv_HandleIdleIrq()          # IDLE 中断处理
```

### 日志链路

```text
LOG_Printf(...)
  └── LogService::Printf()
        ├── nanoprintf           # Middleware: 格式化
        └── BspUart_LogPutChar   # BSP: 串口发送（USART1）
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

### 3. DMA 接收模式下 ACK 发送失败

APP 在 DMA 接收模式下调用 `HAL_UART_Transmit` 发送 ACK 失败，因为 HAL 状态机冲突。

解决方案：在 BSP 层新增 `BspUartRcv_SendAckDirect()`，内部先停止 DMA、禁用中断，再用寄存器直接发送，绕过 HAL 状态机。

### 4. APP 后续接入 RTOS

后续 APP 如果会上 FreeRTOS，要特别注意：

- APP 自己设置 `VTOR`
- Bootloader 跳转前清理 SysTick、NVIC、DMA、外设中断
- `SysTick / PendSV / SVC` 必须来自 APP 向量表
- 中断优先级要符合 `configMAX_SYSCALL_INTERRUPT_PRIORITY`
- 重新规划 MSP、任务栈和 FreeRTOS heap

## 文档入口

| 文档 | 内容 |
|------|------|
| [首页](docs/00-Index/首页.md) | 知识库入口 |
| [当前进度](docs/00-Index/当前进度.md) | 主线进度和最近变更 |
| [架构规范](docs/10-项目规范/架构规范.md) | 分层设计原则与落地约束 |
| [命名与注释规范](docs/10-项目规范/命名与注释规范.md) | 命名规则、注释风格 |
| [开发日志](docs/20-开发记录/开发日志.md) | 阶段性开发记录 |
| [踩坑记录](docs/20-开发记录/踩坑记录.md) | 问题复盘 |
| [升级模块拆分方案](docs/30-工程资料/系统设计/升级模块拆分方案.md) | 升级模块四层拆分设计 |
| [STM32 CMake 笔记](docs/30-工程资料/STM32/CMake笔记.md) | CMake 工程配置 |
| [链接脚本与启动流程](docs/30-工程资料/STM32/链接脚本与启动流程.md) | Flash 分区、跳转流程 |
| [XY7320Host 上位机说明](docs/40-上位机/XY7320Host串口升级上位机说明.md) | Qt 上位机构建说明 |

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
