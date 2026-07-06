# AGENTS.md

本文件约束当前目录及其所有子目录。后续代理进入本目录后必须先阅读本文件，再执行分析、设计或修改。

## 项目目标

在当前目录中逐步建立一个独立的 Qt 6 + QML + C++ 串口助手。

目标功能按以下顺序推进：

1. 串口枚举、打开、关闭和基础收发。
2. 文本/HEX 收发、时间戳、收发统计和日志。
3. 快捷命令、循环发送和自动应答。
4. 数据过滤和关键字高亮。
5. 协议解析和实时绘图。

每个阶段必须先形成可独立运行、可验证的闭环，再进入下一阶段。不要一次性迁移全部功能。

## 本机路径

- 当前新项目工作目录：`C:\Users\XYKJ\Desktop\esp32\XY7320_ST\usart`
- Serial Studio 参考源码：`D:\githubky\Serial-Studio`
- Qt Creator：`D:\qt\Tools\QtCreator\bin\qtcreator.exe`
- Qt CMake：`D:\qt\Tools\CMake_64\bin\cmake.exe`
- Qt Ninja：`D:\qt\Tools\Ninja\ninja.exe`
- Qt MinGW：`D:\qt\Tools\mingw1310_64\bin`
- Qt 6.8.3：`D:\qt\6.8.3\mingw_64`

配置或构建 Qt 工程时优先使用 Qt 安装目录中的 CMake、Ninja 和 MinGW，不使用 Strawberry Perl 附带的 CMake。

## Serial Studio 的用途与边界

`D:\githubky\Serial-Studio` 仅作为架构、功能和交互参考。

重点参考范围：

- `app/src/IO/`：串口、网络、缓冲和帧读取。
- `app/src/Console/`：终端处理、HEX、时间戳和导出。
- `app/src/DataModel/`：协议解析和数据模型。
- `app/src/UI/Dashboard*`：绘图及仪表盘数据链路。
- `app/src/AppState.h`：运行模式和全局状态。
- `app/qml/`：页面组织与 QML/C++ 绑定方式。

不要把以上目录整体复制到新项目。Serial Studio 模块之间存在较多内部依赖，整体复制会引入无关功能和耦合。

默认采用“理解行为和接口后独立实现”的方式迁移。除非用户明确决定让新项目遵循 GPLv3，否则不要复制 Serial Studio 的 GPL 源码。即使只复制部分源码，也必须先核对对应文件顶部的 SPDX 许可证标识。

不得复制 Serial Studio 的名称、Logo、品牌素材或商业/Pro 模块代码。

## EvolveUI 技术路线

本项目正式采用以下组合：

```text
新串口助手
├─ UI：EvolveUI
├─ 后端：基于 Qt SerialPort 独立实现
├─ 功能与架构参考：Serial Studio
```

- EvolveUI 本地参考源码：`D:\githubky\EvolveUI`
- EvolveUI 上游仓库：`https://github.com/sudoevolve/EvolveUI`
- EvolveUI 使用 MIT 许可证；复用组件时保留其许可证和版权声明。
- 优先复用 EvolveUI 的主题、基础控件、动画和布局组件，不重复制作已有通用控件。
- EvolveUI 只负责表现层，不承载串口状态机、收发缓冲、协议解析或日志业务。
- Serial Studio 仅用于研究功能行为、数据流和性能策略；默认不复制其 GPL 或商业源码。

新功能的分工原则：

| 范围 | 实现方式 |
|---|---|
| 主题、按钮、输入框、卡片、弹窗、导航 | 优先使用或扩展 EvolveUI |
| 串口枚举、打开、关闭、收发 | 使用 Qt SerialPort 独立实现 |
| HEX、时间戳、统计、日志 | 在新项目后端独立实现 |
| 快捷命令、循环发送、自动应答 | 在新项目中按串口助手交互独立设计 |
| 数据过滤、关键字高亮 | 独立实现模型和规则，不塞进 QML 页面 |
| 协议解析、实时绘图 | 参考 Serial Studio 思路，分阶段独立实现 |

引入 EvolveUI 组件时，只复制当前阶段实际使用的最小集合，并在项目内集中放置，避免把演示页面和无关组件全部带入产品代码。

## 新工程建议结构

```text
SerialAssistant/
├─ CMakeLists.txt
├─ main.cpp
├─ backend/
│  ├─ io/
│  ├─ console/
│  ├─ protocol/
│  └─ plot/
├─ models/
└─ qml/
   ├─ Main.qml
   ├─ components/
   ├─ pages/
   └─ theme/
```

C++ 后端负责串口、缓冲、协议解析、日志和绘图数据；QML 负责布局、主题和交互。业务状态不得散落在多个 QML 页面中，应收口到明确的 C++ 对象或 QML 模型。

## 开发约束

- 文件统一按 UTF-8 读取和维护。
- 开始修改前先检查当前文件和工作区状态，不覆盖用户已有改动。
- 搜索优先使用 `rg` / `rg --files`。
- 修改前先读取目标文件当前内容，采用增量编辑。
- 不使用 `git reset --hard`、`git checkout --`、`git clean -fd` 等破坏性命令。
- 不提交构建目录、可执行文件、动态库和下载缓存。
- C++ 使用 C++20；QML 使用 Qt 6 模块。
- Qt Quick Controls 必须全局固定到可自定义的非原生样式，当前项目统一使用 `QQuickStyle::setStyle("Basic")`，且必须在 `QGuiApplication` 创建前调用。
- 自定义 `Button`、`CheckBox`、`ComboBox`、`Slider`、`TextField` 等 Qt Quick Controls 的 `contentItem`、`indicator`、`background`、`handle`、`delegate`、`popup` 等内部部件时，必须确认当前控件使用 `QtQuick.Controls.Basic` 或应用已全局设置 Basic/Fusion/Material 等支持自定义的样式；不得依赖平台原生样式。
- 新增 QML 自定义控件时，优先显式导入 `QtQuick.Controls.Basic`；如果只使用基础 `Item`、`Rectangle`、`Text` 等类型，不引入 Controls。
- 基于已有 QML 组件组合新组件时，不要重复声明基类已经定义的同名 `required property`，尤其是 `theme`，避免遮蔽基类必填属性导致初始化失败。
- C++ 类型使用 `PascalCase`，方法和局部变量使用 `camelCase`，成员变量使用 `m_camelCase`。
- 对外 C++ API 使用 Doxygen 兼容注释；不要添加重复代码含义的注释。
- 串口接收不得在 UI 线程中执行阻塞等待。
- 高频接收数据先进入有界缓冲，再批量通知 UI，避免每字节触发一次 QML 更新。
- UI 不直接持有 `QSerialPort`，统一通过后端服务和信号/槽访问。

## 第一阶段验收标准

第一阶段只实现串口基础闭环：

- 自动枚举可用串口并支持手动刷新。
- 可配置端口、波特率、数据位、校验位、停止位和流控。
- 能正确打开和关闭串口，并显示明确的失败原因。
- 能发送文本并显示接收文本。
- 关闭窗口或切换端口时能正确释放串口。
- 串口接收不会阻塞或卡住 QML 界面。

在第一阶段通过前，不提前加入绘图、复杂协议、脚本系统或多设备支持。

## 构建与验证

推荐配置命令：

```powershell
& 'D:\qt\Tools\CMake_64\bin\cmake.exe' `
  -S . `
  -B build `
  -G Ninja `
  -DCMAKE_PREFIX_PATH='D:\qt\6.8.3\mingw_64' `
  -DCMAKE_MAKE_PROGRAM='D:\qt\Tools\Ninja\ninja.exe' `
  -DCMAKE_CXX_COMPILER='D:\qt\Tools\mingw1310_64\bin\g++.exe'
```

推荐构建命令：

```powershell
& 'D:\qt\Tools\CMake_64\bin\cmake.exe' --build build
```

修改 C++ 或 QML 后至少完成：

1. CMake 配置检查。
2. 完整构建。
3. 启动程序并验证受影响交互。
4. 有串口硬件时做真实收发；没有硬件时明确记录未验证项。
