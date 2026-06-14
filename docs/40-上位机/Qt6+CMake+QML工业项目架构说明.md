---
title: Qt6+CMake+QML工业项目架构说明
tags:
  - docs/reference
  - qt
  - architecture
status: active
---

# Qt6 + CMake + QML 工业项目架构说明

## 1. 这份文档是做什么的

这份文档用于约定大型 Qt6 + CMake + QML 项目的目录结构、模块边界、依赖关系和开发规范。

目标不是追求"看起来高级"的架构，而是解决下面几个现实问题：

- 项目功能越来越多以后，目录不容易乱
- 多个开发者同时开发时，边界更清楚
- 后续增加新功能时，不容易把旧结构搞坏
- 出现问题时，更容易定位到应该改哪一层
- 方便 IDE、静态检查工具和 AI 工具理解项目结构

这份架构特别适合下面这类项目：

- 上位机软件
- 工业配置工具
- 串口调试和设备管理工具
- 数据显示和数据管理软件
- 多页面、多模块、长期维护型 Qt 项目

## 2. 核心思想

这个架构有两个最重要的原则：

### 2.1 横向按能力拆库

可复用、与页面无关、可以被多个功能共同使用的内容，拆成独立库。

典型例子：

- 串口通信
- 协议编解码
- 数据库存储
- 配置读写
- 日志系统
- 通用算法

这些内容不属于某一个页面，它们属于整个系统的基础能力。

### 2.2 纵向按业务拆功能模块

面向用户的功能页面，按业务功能拆成 feature 模块。

典型例子：

- 串口助手
- 模拟电台
- 数据显示
- 数据管理
- 设置页面

这些内容直接对应用户在菜单中看到的功能入口。

### 2.3 一句话总结

可以把整个项目理解成下面这句话：

**库按复用能力拆，页面按业务功能拆，主程序只负责装配和导航。**

## 3. 为什么不能只按技术类型分目录

很多项目一开始会这样拆：

```text
src/
├─ qml/
├─ viewmodels/
├─ services/
├─ models/
└─ utils/
```

这种结构在小项目里问题不大，但项目一大就会越来越难维护。

原因是同一个功能的代码会被打散到很多目录里。

例如"串口助手"这个功能可能会分散在下面这些地方：

- `qml/SerialAssistantPage.qml`
- `viewmodels/SerialAssistantViewModel.cpp`
- `models/SerialPortListModel.cpp`
- `services/SerialSessionService.cpp`

这样会带来几个问题：

- 改一个功能时，要在很多目录里来回跳
- 新人很难快速看懂一个功能到底由哪些文件组成
- 删除一个功能时，很容易漏删
- AI 工具在理解业务边界时也更容易混淆

所以大型项目更适合先按功能聚合，再考虑功能内部是否继续分层。

## 4. 推荐的总目录结构

```text
MyIndustrialApp/
├─ CMakeLists.txt
├─ cmake/
├─ docs/
├─ assets/
├─ apps/
│  └─ desktop/
├─ libs/
├─ src/
│  ├─ shared/
│  └─ features/
├─ tests/
└─ tools/
```

下面分别说明每一层的职责。

## 5. 顶层目录职责

### 5.1 `apps/`

这里放可执行程序入口。

例如：

- `main.cpp`
- 主窗口
- 主菜单
- 导航壳层
- 启动装配逻辑

这一层负责把功能模块和底层库"拼起来"，但不负责写具体业务细节。

建议它只做下面这些事情：

- 初始化应用对象
- 初始化日志、主题、翻译
- 创建 QML 引擎
- 注册少量全局对象
- 加载主界面
- 管理菜单和导航

不建议把具体业务逻辑写进这一层。

### 5.2 `libs/`

这里放基础能力库、通用业务库和服务库。

它们通常具备下面特征：

- 不直接对应某个页面
- 可被多个 feature 复用
- 逻辑相对稳定
- 适合单独测试
- 尽量少依赖 QML

例如：

- `serial_core`
- `radio_protocol`
- `data_store`
- `settings_core`
- `logging`
- `device_domain`
- `app_services`

### 5.3 `src/features/`

这里放面向用户的功能模块。

一般来说，一个菜单项，或者一组高度相关的页面，可以对应一个 feature。

例如：

- `serial_assistant`
- `radio_simulator`
- `data_view`
- `data_manager`
- `settings`

每个 feature 内部建议至少包含：

- 本功能的 QML 页面
- 本功能对应的 ViewModel
- 本功能说明文档
- 本功能自己的 `CMakeLists.txt`

### 5.4 `src/shared/`

这里放多个 feature 都会用到的共享内容。

例如：

- 通用 UI 组件
- 通用表格模型
- 时间和字节格式化工具
- 公共弹窗和状态控件

它的目的不是替代 `libs/`，而是承载偏上层、偏界面复用的公共内容。

### 5.5 `tests/`

这里放测试代码。

建议至少分成下面几类：

- `unit/`：单元测试
- `integration/`：集成测试
- `qml/`：QML 页面和交互测试

### 5.6 `docs/`

这里放项目文档、架构说明、模块说明和开发约束。

大型项目里，文档不是附属品，而是架构的一部分。

## 6. 结合当前场景的推荐目录树

假设这个项目包含下面这些业务：

- 串口助手
- 模拟电台
- 数据显示
- 数据管理
- 设置

并且底层包含：

- 串口能力
- 电台协议
- 数据存储
- 配置系统
- 日志系统

那么推荐目录如下：

```text
MyIndustrialApp/
├─ apps/
│  └─ desktop/
│     ├─ main.cpp
│     ├─ AppBootstrap.h
│     ├─ AppBootstrap.cpp
│     └─ qml/
│        ├─ Main.qml
│        ├─ AppShell.qml
│        ├─ MainMenu.qml
│        └─ NavigationPanel.qml
├─ libs/
│  ├─ serial_core/
│  ├─ radio_protocol/
│  ├─ data_store/
│  ├─ settings_core/
│  ├─ logging/
│  ├─ device_domain/
│  └─ app_services/
├─ src/
│  ├─ shared/
│  │  ├─ ui/
│  │  ├─ models/
│  │  └─ utils/
│  └─ features/
│     ├─ serial_assistant/
│     ├─ radio_simulator/
│     ├─ data_view/
│     ├─ data_manager/
│     └─ settings/
└─ tests/
```

## 7. `libs/` 应该怎么拆

### 7.1 `serial_core`

负责串口底层通信能力，例如：

- 端口枚举
- 打开和关闭端口
- 发送和接收原始字节流
- 超时处理
- 错误状态
- 重连和会话基础管理

注意，这里放的是"串口能力"，不是"串口助手页面"。

### 7.2 `radio_protocol`

负责电台相关协议逻辑，例如：

- 帧格式
- 编码和解码
- 校验算法
- 命令集定义
- 协议字段解释

这一层不负责页面，也不负责菜单逻辑。

### 7.3 `data_store`

负责数据存取能力，例如：

- 数据记录持久化
- 查询条件封装
- 数据导出
- SQLite 或文件读写

### 7.4 `settings_core`

负责配置系统，例如：

- 配置读取
- 配置保存
- 默认值管理
- 配置版本迁移

### 7.5 `logging`

负责日志能力，例如：

- 日志级别
- 输出格式
- 文件日志
- 控制台日志
- 日志转发

### 7.6 `device_domain`

负责核心业务对象定义，例如：

- 设备状态
- 通道数据
- 告警记录
- 设备配置结构

这层更偏"领域对象"，尽量保持纯净，不直接依赖页面。

### 7.7 `app_services`

这一层用于放跨多个页面的应用服务。

它通常介于"纯底层库"和"页面专属逻辑"之间。

例如：

- 串口会话服务
- 实时数据聚合服务
- 数据查询编排服务
- 设置门面服务

如果某段逻辑被多个 feature 共同使用，但又不完全属于底层能力，可以放到这里。

## 8. `features/` 应该怎么拆

### 8.1 一个 feature 的本质

一个 feature 可以理解成一个"业务功能包"。

这个包里放的是完成该功能所需的界面、状态组织和少量专属逻辑。

例如 `serial_assistant` 模块可以这样组织：

```text
src/features/serial_assistant/
├─ CMakeLists.txt
├─ README.md
├─ backend/
│  ├─ SerialAssistantViewModel.h
│  ├─ SerialAssistantViewModel.cpp
│  ├─ SerialPortListModel.h
│  ├─ SerialPortListModel.cpp
│  ├─ SendHistoryModel.h
│  └─ SendHistoryModel.cpp
└─ qml/
   ├─ SerialAssistantPage.qml
   ├─ SerialConfigPanel.qml
   ├─ SerialSendPanel.qml
   ├─ SerialReceivePanel.qml
   └─ PortSelectorDialog.qml
```

### 8.2 为什么 feature 内部建议聚合

因为这样做有几个直接收益：

- 看一个功能时，不用跨很多目录来回跳
- 改一个功能时，影响范围更集中
- 新增功能更容易复制已有模块的组织方式
- 删除功能时不容易漏文件
- AI 和新人更容易理解"这整个目录就是一个功能"

### 8.3 feature 内部一定要分 `backend/` 和 `qml/` 吗

不是强制要求。

如果项目较小、单个 feature 文件不多，也可以混放：

```text
src/features/serial_assistant/
├─ SerialAssistantPage.qml
├─ SerialConfigPanel.qml
├─ SerialAssistantViewModel.h
├─ SerialAssistantViewModel.cpp
├─ SendHistoryModel.h
├─ SendHistoryModel.cpp
└─ README.md
```

但大型项目通常建议继续细分，因为这样更利于维护。

所以更准确的原则是：

- 第一步，先按 feature 聚合
- 第二步，再根据项目规模决定 feature 内是否分 `qml/` 和 `backend/`

## 9. 菜单功能如何映射到架构

对于当前这个项目，可以按下面方式理解：

### 9.1 串口助手

这是一个 feature。

它的职责是：

- 提供串口参数设置页面
- 提供收发数据显示页面
- 提供发送历史和辅助工具

它依赖：

- `serial_core`
- 可能还依赖 `logging`

### 9.2 模拟电台

这也是一个 feature。

它的职责是：

- 编辑电台协议数据
- 预览帧内容
- 发起模拟发送
- 管理模拟过程

它依赖：

- `radio_protocol`
- `serial_core`
- 可能依赖 `app_services`

### 9.3 数据显示

这是一个 feature。

它的职责是：

- 实时数据显示
- 列表显示
- 图表显示
- 状态摘要

它依赖：

- `device_domain`
- `app_services`
- 可能依赖 `data_store`

### 9.4 数据管理

这是一个 feature。

它的职责是：

- 历史数据查询
- 过滤
- 删除
- 导出

它依赖：

- `data_store`
- `app_services`

### 9.5 设置

这是一个 feature。

它的职责是：

- 应用设置
- 串口默认参数
- 界面设置
- 关于页面

它依赖：

- `settings_core`

## 10. QML 和 C++ 应该怎么分工

这是整个 Qt 项目最容易混乱的地方，必须明确。

### 10.1 QML 负责什么

QML 负责：

- 页面结构
- 控件布局
- 状态绑定
- 用户交互转发
- 少量简单展示逻辑

例如：

- 按钮点击
- 输入框绑定
- 列表展示
- 页面切换
- 简单颜色和文本状态切换

### 10.2 C++ 负责什么

C++ 负责：

- 业务状态组织
- 数据转换
- 与底层库交互
- 调用服务层
- 复杂逻辑处理
- 生命周期和资源管理

例如：

- 打开串口
- 启动模拟会话
- 查询数据库
- 处理协议帧
- 管理导出任务

### 10.3 不建议做的事

不建议让 QML 直接做下面这些事情：

- 直接操作串口
- 直接访问数据库
- 直接写复杂协议解析
- 用大量 JS 写业务流程

因为一旦这样做，后面维护会迅速变难。

## 11. 推荐的依赖规则

这一部分是大型项目最关键的约束。

建议固定成下面这条依赖路径：

```text
apps -> features -> shared / libs
```

也就是说：

- `apps` 可以依赖 `features`
- `features` 可以依赖 `shared` 和 `libs`
- `shared` 可以依赖少量底层库
- `libs` 不允许反向依赖 `features`

### 11.1 强约束

建议在团队里明确下面这些规则：

- `feature A` 不直接依赖 `feature B` 的内部实现
- 页面不直接访问数据库和串口
- `main.cpp` 不写业务逻辑
- `libs` 不依赖 `features`
- 同一个通用能力不要在多个 feature 里重复造一份

### 11.2 为什么要这样限制

因为如果没有依赖规则，项目很快就会形成网状耦合：

- 页面互相调用
- 模块互相引用内部对象
- 一个小改动影响一大片

最后就会出现"谁都不敢动"的情况。

## 12. CMake 组织建议

### 12.1 顶层 CMake

顶层 `CMakeLists.txt` 建议只负责组织子模块：

```cmake
cmake_minimum_required(VERSION 3.24)
project(MyIndustrialApp VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Qml QuickControls2 Test)
qt_standard_project_setup(REQUIRES 6.8)

add_subdirectory(libs/serial_core)
add_subdirectory(libs/radio_protocol)
add_subdirectory(libs/data_store)
add_subdirectory(libs/settings_core)
add_subdirectory(libs/logging)
add_subdirectory(libs/device_domain)
add_subdirectory(libs/app_services)

add_subdirectory(src/shared/ui)
add_subdirectory(src/shared/models)
add_subdirectory(src/shared/utils)

add_subdirectory(src/features/serial_assistant)
add_subdirectory(src/features/radio_simulator)
add_subdirectory(src/features/data_view)
add_subdirectory(src/features/data_manager)
add_subdirectory(src/features/settings)

add_subdirectory(apps/desktop)
add_subdirectory(tests)
```

### 12.2 每个 feature 自己维护自己的构建脚本

这样做的好处是：

- feature 边界更明确
- 新增和删除功能时更简单
- 构建关系不容易堆到顶层

## 13. `README.md` 在 feature 里的作用

建议每个 feature 都有一个短小但明确的 `README.md`。

它建议至少包含下面内容：

- 模块目标
- 模块入口页面
- 主要 ViewModel
- 依赖哪些库
- 哪些目录允许放什么
- 哪些边界不能跨

例如：

```text
模块：串口助手
入口页面：SerialAssistantPage.qml
主要状态类：SerialAssistantViewModel
依赖库：serial_core、logging
不允许：页面直接访问串口库内部对象；其他 feature 直接引用本模块内部控件
```

这对团队协作和 AI 工具都非常有帮助。

## 14. 建议的命名规范

建议统一使用下面这种命名方式：

### 14.1 QML 页面

- `XxxPage.qml`
- `XxxDialog.qml`
- `XxxPanel.qml`
- `XxxCard.qml`
- `XxxToolbar.qml`

### 14.2 C++ 状态和模型

- `XxxViewModel.h/.cpp`
- `XxxListModel.h/.cpp`
- `XxxTableModel.h/.cpp`
- `XxxService.h/.cpp`
- `XxxRepository.h/.cpp`

### 14.3 为什么命名统一很重要

因为统一命名以后：

- 搜索更快
- 新人更容易理解文件角色
- AI 更容易推断同类文件应该放在哪里

## 15. 推荐的测试和质量门禁

大型 Qt 项目不建议只靠"能跑起来"判断质量。

至少建议接入下面这些内容：

- C++ 单元测试
- QML 页面测试
- `qmllint`
- `qmlformat`
- C++ 格式化
- C++ 静态检查

建议在 CI 中至少执行：

- 配置工程
- 编译
- 单元测试
- QML 测试
- QML lint

## 16. 最常见的错误组织方式

下面这些写法非常容易把项目带乱：

### 16.1 所有页面都堆在一个总 `qml/` 目录

问题是功能边界会越来越模糊。

### 16.2 所有 ViewModel 都堆在一个总 `viewmodels/` 目录

问题是一个功能的相关文件被彻底打散。

### 16.3 页面直接访问串口、数据库或协议解析

问题是界面层和业务层耦合过重。

### 16.4 `main.cpp` 变成总控中心

问题是后期几乎无法维护。

### 16.5 feature 之间相互依赖内部细节

问题是最终形成网状依赖。

## 17. 当前项目建议采用的最小规则

如果要尽快开始落地，建议优先落实下面这些规则：

1. 菜单功能优先拆成 `features/<name>`
2. 串口、协议、数据、配置、日志这些优先拆成 `libs/<name>`
3. 页面只和 ViewModel 交互
4. ViewModel 通过 service 或 library 完成业务动作
5. 每个 feature 自带一个简短 `README.md`
6. 不允许 feature 直接互相依赖内部实现
7. 新增需求时，先判断它属于 feature 还是 library

## 18. 结论

对于大型 Qt6 + CMake + QML 项目，最稳妥的做法不是把所有代码按"技术类型"平铺，也不是盲目引入很重的第三方框架，而是：

- 横向按能力拆库
- 纵向按业务拆 feature
- 页面和业务分层
- 用 CMake 显式管理模块
- 用文档和规则约束依赖边界

如果后续项目继续扩大，这套结构仍然可以平滑扩展。

如果项目还处于早期阶段，也可以先保留这套大方向，只在 feature 内部适度简化目录层级。
