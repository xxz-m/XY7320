# AGENTS.md

本文件是后续代理进入本仓库时必须先读的约束文档，作用范围为整个仓库。

## 读取与编码

- 本项目文档和脚本按 UTF-8 读取与维护。
- PowerShell 读取中文文件时先设置输出编码，并显式指定 `-Encoding UTF8`：

```powershell
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
Get-Content -Raw -Encoding UTF8 README.md
```

- 搜索优先使用 `rg` / `rg --files`。读取 Markdown、C/C++、QML、PowerShell 文件时都按 UTF-8 处理。

## 进入项目先读

按下面顺序建立上下文，不要只看代码就直接改：

1. `README.md`
2. `docs/00-Index/当前进度.md`
3. `docs/00-Index/项目结构.md`
4. `docs/10-项目规范/架构规范.md`
5. `docs/10-项目规范/命名与注释规范.md`

涉及上位机时补读：

- `docs/40-上位机/XY7320Host串口升级上位机说明.md`
- `docs/40-上位机/Qt6+CMake+QML工业项目架构说明.md`

涉及升级链路、Flash、跳转、CMake 时补读：

- `docs/30-工程资料/升级协议/`
- `docs/30-工程资料/STM32/CMake笔记.md`
- `docs/30-工程资料/STM32/链接脚本与启动流程.md`

## 仓库结构

```text
XY_bootloadr/                 STM32F407 Bootloader 工程
XY7320/                       STM32F407 APP 工程，链接地址 0x08010000
tools/XY7320Host/             Qt6 + QML + C++ 串口升级上位机
docs/                         Obsidian 项目文档库
```

`mOTA/`、`.uploads/`、`.trae/`、`cmake-build-*`、`tools/XY7320Host/build/` 等属于参考资料、本地草稿或构建产物，默认不要纳入功能修改。

## 工作区约束

- 开始前执行 `git status --short`，确认已有改动。
- 工作区可能存在用户未提交修改。不要回滚、覆盖、格式化无关文件。
- 修改同一文件前先读当前内容，基于现状增量编辑。
- 不使用 `git reset --hard`、`git checkout --`、`git clean -fd` 这类破坏性命令，除非用户明确要求。

## 固件架构约束

`XY7320/` APP 按分层架构维护：

```text
App -> Domain -> Services -> Middleware / BSP
System -> App / Services
BSP -> HAL / Drivers
```

硬规则：

- `Core/` 保留 CubeMX 生成代码、启动文件和装配入口；`main.c` 只做初始化、任务创建和启动调度，不堆业务流程。
- `BSP` 只封装硬件访问，不写产品业务、状态机、协议语义。
- `Middleware` 放第三方库或通用协议栈，尽量少改原库。
- `Services` 负责把硬件/中间件能力翻译为项目服务，业务层不得直接散落调用第三方 API。
- `Domain` 放算法、模型、协议纯处理，禁止依赖 `BSP`、`Middleware`、HAL。
- `App` 做业务编排、任务入口、模式切换，模式切换统一走 `ModeManager`。
- `System` 做调度、事件、队列、Tick 等基础设施，不承载具体业务。
- 中断只采集数据、更新最小状态、置标志或投递事件；不要在中断里跑完整协议解析、Flash 业务、文件系统或模式切换。
- 跨模块共享状态必须收口到明确上下文或服务接口，不要自由散布全局变量。
- **USART2 正常发送的唯一入口是 `Services/uart_service/UartTxService::EnqueueControl` / `PublishModeData`。业务模块不得直接调用 `HAL_UART_Transmit`、`HAL_UART_Transmit_DMA` 或 `BspUartRcv_SendAck`。** `BspUartRcv_SendAckDirect` 仅供 Bootloader 异常升级链路兜底。

配置集中在 `XY7320/Common/config/`：

- `bsp_config.h`：硬件配置
- `app_config.h`：APP 功能配置
- `os_config.h`：调度/OS 配置

## Bootloader 与 APP 边界

- Bootloader 起始地址 `0x08000000`，大小 `64KB`。
- APP 起始地址 `0x08010000`，APP 启动后必须设置 `SCB->VTOR = 0x08010000U`。
- 版本配置区位于 `0x080E0000`，当前使用 A1/A2 槽位。
- 不要随意修改链接脚本、跳转流程、向量表、Flash 分区和升级固定码；改动前必须核对 README 与 `docs/30-工程资料/`。
- APP 与 Bootloader 可以共享协议约定和数据格式，但不要轻易强依赖同一个源码实现文件。两边版本配置模块当前分别维护。

## 命名与注释

遵守 `docs/10-项目规范/命名与注释规范.md`：

- C 文件名使用 `snake_case`，C 函数使用 `模块前缀_动作`，例如 `BspFlash_Read()`、`OS_CreateTask()`。
- C 宏和枚举值使用全大写并带模块前缀。
- C++ 类名和方法名使用 `PascalCase`，成员变量使用 `m_camelCase`。
- C/C++ 对外头文件 API、类型、宏必须写 Doxygen 兼容注释。
- 协议格式、帧字段、Flash 布局、硬件 workaround 必须注释原因。
- 不要添加重复代码本身的无意义注释。

固件侧使用 C11 / C++17。裸机代码避免异常、RTTI 和不必要动态内存，优先静态对象、固定容量缓冲和显式上下文。

## CMake 与构建

STM32 固件使用 `arm-none-eabi-gcc/g++`、CMake、Ninja/CLion/STM32CubeCLT。

常用构建：

```powershell
cmake --build XY_bootloadr/cmake-build-debug
cmake --build XY7320/cmake-build-debug
```

如果需要重新配置 APP：

```powershell
cmake -S XY7320 -B XY7320/cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build XY7320/cmake-build-debug
```

`CMakeLists.txt` 是实际参与编译的文件，`CMakeLists_template.txt` 用于防止工具重新生成时丢失关键配置。涉及 include、source glob、链接参数、裸机参数时，两个文件都要保持一致，并运行：

```powershell
.\XY7320\check_cmake_sync.ps1
```

不要提交 `.elf`、`.hex`、`.bin`、`.map`、`cmake-build-*` 等构建产物，除非用户明确要求归档发布产物。

## Qt 上位机约束

`tools/XY7320Host/` 使用 Qt6 Quick + SerialPort + QML。

- 业务状态机主要在 `backend/FirmwareUploader.*`。
- 串口枚举/打开关闭在 `backend/SerialPortManager.*`。
- QML 组件在 `components/`，页面在 `pages/`，保持现有 EvolveUI 风格。
- UI 布局优先使用 `RowLayout`、`ColumnLayout`、`GridLayout`，避免绝对定位造成重叠。
- 修改 QML 后至少做静态差异检查，能构建时再构建上位机。

命令行构建示例：

```powershell
cmake -S tools/XY7320Host -B tools/XY7320Host/build -G Ninja -DCMAKE_PREFIX_PATH=D:/QT6/6.11.1/mingw_64
cmake --build tools/XY7320Host/build --target XY7320Host
```

## 文档维护

修改以下内容时要同步更新相关文档：

- Flash 分区、链接脚本、Bootloader 跳转、VTOR
- 升级协议、固定码、主协议命令、上位机升级流程
- APP 分层边界、模块职责、公共配置
- 已验证的踩坑结论和调试方法

优先更新对应主题文档，再在 `docs/00-Index/当前进度.md` 记录主线状态变化。

## 验证要求

- 固件改动优先构建受影响工程：`XY7320`、`XY_bootloadr` 或两者。
- 上位机改动优先构建 `tools/XY7320Host`；环境缺 Qt 时说明无法本机构建。
- Markdown/脚本改动至少检查编码和关键链接路径。
- 如果本机缺少工具链、Qt 或硬件，明确说明未验证项，不要假装通过。
