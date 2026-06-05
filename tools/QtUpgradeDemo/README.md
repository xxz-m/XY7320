# QtUpgradeDemo

这是一个 Qt 6 + CMake + QML + C++ 写的串口升级上位机 demo，用来适配当前
`XY_bootloadr` 里的 `simple_update` 简易升级协议。

## 协议

上位机先发送一个 12 字节的小端头：

```text
uint32 magic    = 0x41505055
uint32 app_size = APP .bin byte size
uint32 app_crc  = CRC32/IEEE of APP .bin
```

然后等待设置的擦除延时，再按包发送 APP `.bin` 原始数据。

当前 bootloader 使用 `USART2` 接收升级数据，DMA 缓冲区是 1024 字节，所以这个
demo 把包大小限制在 `1..1024`。包间隔单位是 ms，最小限制为 1 ms，方便固件端
通过 UART IDLE 把每包数据分开。

当前 bootloader 收到头包后会擦除整个 APP 分区。`擦除等待` 要留够 sector 4..11
擦除完成的时间，默认 3000 ms。如果串口日志显示固件数据在擦除完成前就到了，
就把这个值调大。

注意：当前 bootloader 会打印并接收 `app_crc`，但还没有真正做 CRC 校验。上位机
这里仍然发送真实 CRC32，后续固件端补校验时不用再改上位机协议。

## 编译

安装 Qt 6、Qt SerialPort 模块和 C++ 编译器后执行：

```powershell
cmake -S tools\QtUpgradeDemo -B tools\QtUpgradeDemo\build -DCMAKE_PREFIX_PATH=C:\Qt\6.6.3\msvc2019_64
cmake --build tools\QtUpgradeDemo\build --config Release
```

把 `CMAKE_PREFIX_PATH` 改成你本机 Qt 的安装路径。MinGW 套件也可以，只要 Qt
版本和编译器匹配即可。

## 使用

1. 先把 `XY_bootloadr` 烧到 MCU。
2. 复位板子。如果没有合法 APP，bootloader 会等待升级数据。
3. 上位机连接 bootloader 升级串口：`USART2`，默认 `115200 8N1`。
4. 打开本工具，选择串口和 APP `.bin`，设置包大小、擦除等待、包间隔，然后点击
   `开始升级`。
5. 如需观察 `OK`、接收进度和跳转信息，用 `USART1` 看 bootloader 日志。

APP `.bin` 必须链接到 `0x08010000`，也就是和 `XY7320/STM32F407ZGTX_FLASH.ld`
保持一致。
