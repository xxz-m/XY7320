# 项目结构说明

## 总目录

```text
E:\XY7320\XY7320
```

这是当前资料和工程的总目录。

主要内容：

```text
XY_bootloadr/        Bootloader 工程
docs/               项目资料文档
mOTA移植步骤.md      原始移植步骤文档
```

## XY_bootloadr

```text
XY_bootloadr/
```

这是当前 STM32F407 Bootloader 工程。

主要目录：

```text
Core/
Drivers/
MOTA/
```

主要工程文件：

```text
CMakeLists.txt
CMakeLists_template.txt
STM32F407ZGTX_FLASH.ld
STM32F407ZGTX_RAM.ld
XY_bootloadr.ioc
stm32f4discovery.cfg
```

## Core

CubeMX 生成的工程核心代码。

```text
Core/Inc     主工程头文件
Core/Src     主工程源文件
Core/Startup 启动文件
```

当前 `Core/bootloader` 中已有一些 Bootloader 相关文件：

```text
boot_config.h
boot_jump.c
boot_jump.h
```

后续要注意它们和 mOTA 的 `bootloader_port.c` / APP 跳转逻辑是否重复。

## Drivers

STM32 官方驱动和 CMSIS。

```text
CMSIS
STM32F4xx_HAL_Driver
```

这里一般不主动修改。

## MOTA

当前移植的 mOTA 组件。

```text
MOTA/Core
MOTA/Config
MOTA/Bsp
MOTA/Port
```

### MOTA/Core

mOTA 核心逻辑：

```text
bootloader.c
bootloader.h
bootloader_define.h
bootloader_port.c
firmware_manage.c
firmware_manage.h
data_transfer.h
protocol_parser.h
```

当前阶段二重点：

```text
bootloader.c：Bootloader 主状态机
firmware_manage.c：固件包和 Flash 写入管理
bootloader_port.c：项目移植接口
```

当前 `data_transfer.h` 和 `protocol_parser.h` 主要是为了阶段二编译占位，后续接 YModem 时再实现或移植对应 `.c`。

### MOTA/Config

mOTA 配置文件：

```text
bootloader_config.h
```

这里配置：

```text
分区方案
Bootloader 大小
APP 分区大小
是否启用解密
是否启用自动更新
是否启用 APP 校验
Flash 写入粒度
```

### MOTA/Bsp

mOTA 板级抽象层。

当前真实实现：

```text
MOTA/Bsp/src/bsp_flash.c
```

当前主要头文件：

```text
bsp_flash.h
bsp_uart.h
bsp_timer.h
bsp_board.h
common.h
user.h
```

其中 `bsp_flash.c/h` 是阶段二真正使用的。其他 UART、Timer、Board 相关头文件，当前大多是为了阶段二编译，后续再实现。

### MOTA/Port

项目和芯片强相关的底层适配。

当前文件：

```text
flash_port_stm32f4.c
```

作用：

```text
把 mOTA 的 read/write/erase 接到 STM32F4 HAL Flash 操作。
```

## docs

```text
docs/
```

这是当前项目资料文档目录，位于总目录，不在 `XY_bootloadr` 工程内部。

目录规划：

```text
docs/mota
docs/project-docs
docs/cmake-stm32
```

### docs/mota

记录 mOTA 移植步骤、阶段说明、坑点和后续裁剪计划。

### docs/project-docs

记录项目日志、项目结构、知识补充。

### docs/cmake-stm32

记录 CMake、STM32CubeCLT、链接脚本、OpenOCD 等知识。

## 当前工程状态

```text
mOTA 阶段二编译通过
Flash Port 已接入
串口未接入 mOTA
YModem 未接入
```

后续工作重点：

```text
1. 验证 Flash 擦写读回。
2. 接串口。
3. 接 data_transfer。
4. 接 protocol_parser / YModem。
5. 根据实际调用关系裁剪占位文件。
```
