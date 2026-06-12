# XY7320 STM32F407 Bootloader 串口升级工程

本仓库是 `STM32F407ZGTx` 的 Bootloader + APP 双工程，当前重点是通过串口把 APP 固件发送到 Bootloader，由 Bootloader 擦写 APP 分区，校验 APP 首地址后跳转运行。

## 工程结构

```text
XY7320_ST/
├─ XY_bootloadr/                 Bootloader 工程
├─ XY7320/                       APP 工程，链接地址 0x08010000
├─ mOTA/                         第三方 mOTA v2.0 参考源码
├─ docs/                         项目文档（Obsidian 库）
│  ├─ 00-索引/                   项目索引与标签定义
│  ├─ 01-核心参考/               硬件、Bootloader、APP、RTOS 参考
│  ├─ 02-mOTA与升级/             升级协议、mOTA 移植指南
│  ├─ 03-上位机/                 Qt6 架构与上位机工具说明
│  ├─ 04-项目管理/               开发路线、日志、踩坑记录
│  ├─ 05-工程工具/               CMake、GCC 裸机开发笔记
│  └─ _templates/                Obsidian 模板
└─ README.md
```

根目录 `mOTA/` 是官方/参考源码目录，本仓库默认不提交它，避免把大体积参考仓库混进项目源码。

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

## 文档入口

项目文档使用 Obsidian 管理，入口位于 `docs/00-索引/00-项目索引.md`。

主要文档：

| 文档 | 内容 |
|---|---|
| `01-核心参考/硬件参考.md` | Flash 分区、Sector 表、串口、引脚 |
| `01-核心参考/Bootloader开发指南.md` | 链接脚本、跳转流程、Flash 操作 |
| `01-核心参考/APP工程配置.md` | APP 链接脚本、VTOR、GCC 裸机配置 |
| `02-mOTA与升级/简化升级协议.md` | 当前已实现的升级协议 |
| `02-mOTA与升级/mOTA移植指南.md` | mOTA 移植技术参考 |
| `04-项目管理/踩坑记录.md` | 关键 Bug 排查与解决 |
| `05-工程工具/GCC裸机开发笔记.md` | GCC/newlib 裸机开发知识 |

## 构建提示

Bootloader 和 APP 都是 CLion / STM32CubeCLT / arm-none-eabi-gcc 工程。

```powershell
cmake --build XY_bootloadr/cmake-build-debug
cmake --build XY7320/cmake-build-debug
```

Qt 上位机位于 `tools/XY7320Host/`，使用 Qt6 + CMake 构建，需要本机安装 Qt 串口模块 `Qt6::SerialPort`。
