# XY7320Host 串口升级上位机说明

## 目标

`tools/XY7320Host/` 是当前项目的 Qt6 + QML + C++ 串口升级上位机，用于配合 `XY_bootloadr` 完成 APP 固件发送、手动调试和升级日志查看。

本工具替代旧的 `tools/QtUpgradeDemo/`。旧 Demo 已下线，后续维护入口统一切到 `XY7320Host`。

## 工程入口

```text
tools/XY7320Host/
├─ CMakeLists.txt
├─ main.cpp
├─ Main.qml
├─ backend/
│  ├─ FirmwareUploader.h
│  └─ FirmwareUploader.cpp
├─ components/
│  └─ EvolveUI 风格 QML 组件
├─ pages/
│  ├─ UpgradePage.qml
│  ├─ LogsPage.qml
│  └─ SettingsPage.qml
└─ assets/
```

依赖：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort)
target_link_libraries(XY7320Host PRIVATE Qt6::Quick Qt6::SerialPort)
```

## 升级协议

上位机先发送 12 字节头包，再发送 APP 原始 bin 数据。

头包格式为小端序：

```text
magic    4 字节：0x41505055，对应 HEX 为 55 50 50 41
app_size 4 字节：APP bin 文件大小
crc32    4 字节：APP bin 文件 CRC32
```

Bootloader 收到头包后擦除 APP 分区，并等待固件数据。固件数据按当前包大小和包间隔连续发送。

## 串口参数

当前默认值和可选项：

```text
波特率默认：115200
波特率可选：9600 / 57600 / 115200 / 230400 / 460800 / 921600
包大小默认：1024 B
包大小可选：128 / 256 / 512 / 1024 B
包间隔默认：10 ms
包间隔可选：1 / 5 / 10 / 30 ms
串口格式：8N1，无流控
```

关键实现：

```cpp
int m_baudRate = 115200;
int m_packetSize = 1024;
int m_packetDelayMs = 10;
QSerialPort m_serial;
QTimer m_sendTimer;
QTimer m_portRefreshTimer;
QTimer m_fileRefreshTimer;
```

说明：

- `m_sendTimer` 控制固件分包发送节奏。
- `m_portRefreshTimer` 定时刷新串口列表，不需要手动刷新按钮。
- `m_fileRefreshTimer` 定时检测已选择固件是否变化，文件变化后重新计算大小、CRC 和首帧。

## 手动升级流程

手动模式用于联调 Bootloader 时分步骤观察设备反应：

```text
1. 选择串口。
2. 设置波特率。
3. 打开串口开关。
4. 选择 APP bin 文件。
5. 查看自动计算出的 12 字节首帧。
6. 点击“发送首帧”。
7. 等 Bootloader 擦除完成并准备接收固件。
8. 点击“发送固件”。
```

手动发送固件时，仍复用当前包大小和包间隔配置。

关键接口：

```cpp
Q_INVOKABLE void openPort();
Q_INVOKABLE void closePort();
Q_INVOKABLE void sendHeaderManual();
Q_INVOKABLE void sendFirmwareManual();
Q_INVOKABLE void copyHeaderToClipboard();
```

## 自动升级流程

自动升级流程用于正常使用：

```text
1. 选择串口、波特率和 APP bin。
2. 点击开始升级。
3. 上位机打开串口。
4. 发送 12 字节头包。
5. 按包大小和间隔连续发送固件。
6. 更新进度和日志。
```

自动和手动模式共用同一套包大小、包间隔、CRC32 和串口写入逻辑，避免两套逻辑长期漂移。

## UI 与布局约束

当前主界面基于 QML 无边框窗口和 EvolveUI 风格组件：

- 主窗口使用 `Qt.FramelessWindowHint`，自定义最小化和关闭按钮。
- 整体窗口使用圆角外壳和中性灰边框，不使用绿色/青色强调边框。
- 串口开关放在串口下拉框同一行，避免额外按钮破坏结构。
- 串口下拉使用“显示名 + 实际端口名”双字段，界面显示完整描述，打开串口只使用 `COMx`。
- 下拉弹层使用浮层方式显示，打开后应覆盖下方控件，而不是把波特率等控件挤到中间。
- 参数区使用 `GridLayout / RowLayout / ColumnLayout`，避免控件绝对定位和重叠。
- 发包设置里的“包大小”和“间隔”下拉框需要适配小窗口场景：打开时提高所在行 `z` 层级，弹层向上展开，`popupMaxHeight` 调整为 220，并启用常显滚动条，避免被外层滚动区域或下方控件裁剪。

首帧行保持和路径行一致：

```text
标签 + 填满输入框 + 右侧复制按钮
```

复制成功后 QML 提示：

```qml
toast.show(qsTr("首帧已复制"))
```

## 构建

Qt Creator 直接打开 `tools/XY7320Host/CMakeLists.txt` 即可。

命令行示例：

```powershell
cmake -S tools/XY7320Host -B tools/XY7320Host/build -G Ninja -DCMAKE_PREFIX_PATH=D:/QT6/6.11.1/mingw_64
cmake --build tools/XY7320Host/build --target XY7320Host
```

本地生成目录不提交：

```text
tools/XY7320Host/build/
tools/XY7320Host/.qtcreator/
```

## 验证记录

已完成的静态检查：

```powershell
git diff --check -- tools/XY7320Host/Main.qml
```

当前 Codex Windows sandbox 环境中，完整构建和 `qmllint` 曾被沙箱刷新错误拦截；该错误不是源码编译错误。最终构建建议以本机 Qt Creator 或本机命令行为准。
