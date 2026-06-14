---
title: STM32 CMake笔记
tags:
  - docs/reference
  - stm32
  - cmake
status: active
---

# CMake 与 STM32 工程知识积累

## 当前工程的 CMake 作用

当前工程使用 CMake 管理 STM32F407 Bootloader 编译。

主要文件位于：

```text
XY_bootloadr/CMakeLists.txt
XY_bootloadr/CMakeLists_template.txt
```

其中：

```text
CMakeLists.txt
```

是当前实际参与编译的文件。

```text
CMakeLists_template.txt
```

是模板文件。如果工具重新生成 CMakeLists，可能会参考它。

所以后续如果你希望配置长期保留，两个文件都要注意。

## 工具链设置

当前 CMake 里指定了交叉编译工具链：

```cmake
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
```

这些工具来自 STM32CubeCLT 或 ARM GCC 工具链。

如果环境变量没配好，就可能找不到：

```text
arm-none-eabi-gcc
arm-none-eabi-g++
arm-none-eabi-objcopy
```

## C 和 C++ 混合

当前项目声明：

```cmake
project(XY_bootloadr C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
```

表示工程支持：

```text
C
C++
ASM 汇编启动文件
```

注意：

```text
底层不一定都要用 C++ 写。
```

STM32 HAL、mOTA core 可以继续用 C。
你自己的业务层、封装层可以逐步用 C++。

C++ 调用 C 接口时，如果头文件被 C++ 包含，通常需要：

```c
#ifdef __cplusplus
extern "C" {
#endif

void Some_C_Function(void);

#ifdef __cplusplus
}
#endif
```

## 编译参数

当前 Cortex-M4 相关参数：

```cmake
add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
```

含义：

```text
-mcpu=cortex-m4     指定 CPU 内核
-mthumb             使用 Thumb 指令集
-mthumb-interwork   支持 ARM/Thumb 互操作
```

Debug 阶段建议用：

```text
-Og -g
```

这样调试信息更友好。

## include_directories

当前工程 include 路径大致是：

```cmake
include_directories(
    Core/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
)

include_directories(
    MOTA/Core
    MOTA/Config
    MOTA/BSP/inc
    MOTA/Port
)
```

作用是告诉编译器：

```text
#include "xxx.h" 去哪些目录找。
```

如果报错：

```text
fatal error: xxx.h: No such file or directory
```

通常检查两件事：

```text
1. 文件是否真实存在。
2. 文件所在目录是否加入 include_directories。
```

## source 文件收集

当前工程使用：

```cmake
file(GLOB_RECURSE SOURCES
    "Core/*.*" "Drivers/*.*"
    "MOTA/Core/*.*"
    "MOTA/BSP/src/*.*"
    "MOTA/Port/*.*"
)
```

作用是递归收集源文件。

注意：

```text
它会把匹配目录下的很多文件都加入编译。
```

所以如果一个目录里放了重复 `.c`，就可能出现：

```text
multiple definition of xxx
```

这次 `bootloader_port.c` 重复定义就是这种问题。

## 添加新模块怎么做

假设后续新增：

```text
MOTA/Bsp/src/bsp_uart.c
MOTA/Bsp/inc/bsp_uart.h
```

如果 `MOTA/BSP/src/*.*` 已经被 GLOB 收集，`.c` 通常会自动加入编译。

如果新增了一个全新目录，比如：

```text
MOTA/Protocol/src
MOTA/Protocol/inc
```

那就要改 CMake：

```cmake
include_directories(
    MOTA/Protocol/inc
)

file(GLOB_RECURSE SOURCES
    "MOTA/Protocol/src/*.*"
)
```

或者把它加入已有 `file(GLOB_RECURSE SOURCES ...)` 的列表中。

## 删除文件要注意什么

如果文件还在 GLOB 范围内，只要它存在，就可能被编译。

所以遇到重复定义时，不只是看 include，还要看：

```text
这个 .c 是否被 CMake 收进 SOURCES。
```

检查思路：

```text
1. 搜索同名函数在哪些 .c 里定义。
2. 看这些 .c 是否在 GLOB 范围内。
3. 删除或排除重复实现。
```

## 链接脚本

当前 Bootloader 使用：

```cmake
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F407ZGTX_FLASH.ld)
add_link_options(-T ${LINKER_SCRIPT})
```

链接脚本决定程序烧到哪里，以及 RAM/FLASH 大小。

当前 Bootloader 的 FLASH：

```ld
FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 64K
```

这表示 Bootloader 最多只能使用 64KB Flash。

APP 工程后续应该改成：

```ld
FLASH (rx) : ORIGIN = 0x08010000, LENGTH = 960K
```

否则 APP 还是从 `0x08000000` 开始，就会和 Bootloader 冲突。

## 生成 hex 和 bin

当前 CMake 中：

```cmake
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
)
```

作用是编译完成后生成：

```text
XY_bootloadr.elf
XY_bootloadr.hex
XY_bootloadr.bin
```

其中：

```text
elf：带符号信息，调试常用
hex：烧录常用
bin：OTA 固件包或裸二进制常用
```

## 常见错误

### fatal error: xxx.h: No such file or directory

原因：

```text
头文件不存在，或 include 路径没加。
```

处理：

```text
1. 确认 xxx.h 在工程里。
2. 确认所在目录加入 include_directories。
```

### undefined reference to xxx

原因：

```text
函数声明存在，但没有实现，或实现文件没有参与编译。
```

处理：

```text
1. 搜索 xxx 函数在哪里实现。
2. 如果没实现，补 .c。
3. 如果实现了，检查 .c 是否被 CMake 收进 SOURCES。
```

### multiple definition of xxx

原因：

```text
同一个函数被多个 .c 文件实现，并且这些 .c 都参与编译。
```

处理：

```text
1. 搜索函数定义。
2. 保留一个实现。
3. 删除或排除重复文件。
```

### region FLASH overflowed

原因：

```text
程序大小超过链接脚本定义的 FLASH 大小。
```

当前 Bootloader 只有 64KB，如果代码太多，就会溢出。

处理：

```text
1. 查看 memory usage。
2. 删除不必要模块。
3. 开启 -Os。
4. 如果设计允许，调整 Bootloader 分区大小。
```

## OpenOCD 和 cfg

当前工程有：

```text
stm32f4discovery.cfg
```

OpenOCD 需要 cfg 告诉它：

```text
使用什么调试器
使用什么芯片目标
使用 SWD 还是 JTAG
```

如果 cfg 路径不对，会报找不到配置文件。

如果 transport 写错，比如旧写法：

```text
hla_swd
```

在某些 OpenOCD 版本里可能不支持，需要改成：

```text
swd
```

## CMakeLists.txt 和 CMakeLists_template.txt

如果只改 `CMakeLists.txt`，当前编译能生效。

但如果 CubeMX 或工具重新生成工程，可能覆盖 `CMakeLists.txt`。

所以重要配置建议同步到：

```text
CMakeLists_template.txt
```

尤其是：

```text
MOTA include 路径
MOTA source 路径
CMake 最低版本
链接脚本设置
```
