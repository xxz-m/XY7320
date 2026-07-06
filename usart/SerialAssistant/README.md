# SerialAssistant

基于 Qt 6、QML、Qt SerialPort 和 EvolveUI 风格组件实现的独立串口助手。

当前第一阶段支持：

- 串口自动枚举与手动刷新
- 波特率、数据位、校验位、停止位和流控配置
- 串口打开、关闭和热插拔错误提示
- UTF-8 文本非阻塞接收与发送
- 无、LF、CR、CRLF 行尾选择
- 串口参数记忆和深浅主题

## 构建

```powershell
& 'D:\qt\Tools\CMake_64\bin\cmake.exe' `
  -S . `
  -B build `
  -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_PREFIX_PATH='D:\qt\6.8.3\mingw_64' `
  -DCMAKE_MAKE_PROGRAM='D:\qt\Tools\Ninja\ninja.exe' `
  -DCMAKE_CXX_COMPILER='D:\qt\Tools\mingw1310_64\bin\g++.exe'

& 'D:\qt\Tools\CMake_64\bin\cmake.exe' --build build
```

EvolveUI 组件的许可证见 `LICENSES/EvolveUI-MIT.txt`。

