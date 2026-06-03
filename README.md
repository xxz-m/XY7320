# XY_bootloadr & XY7320 - STM32F407ZGTx 双工程

> 目标芯片：**STM32F407ZGTx**（1MB Flash / 192KB SRAM）  
> 开发环境：CLion + STM32CubeCLT + arm-none-eabi-gcc + OpenOCD  
> 参考组件：mOTA v2.0 + STM32F407 bootloader_ymodem 例程

本仓库包含一套 **Bootloader + APP** 双分区 OTA 升级工程，正在按三阶段路线把官方 mOTA 框架移植到自研 STM32F407ZGTx 板子上。

当前重点：`XY_bootloadr` 已完成 mOTA 阶段二的编译接入，后续需要继续验证 APP 区 Flash 擦写读回，然后再接串口和 YModem。

---

## 1. 仓库结构

```text
XY7320/
├── README.md                         项目总览
├── .gitignore
│
├── docs/                             项目资料文档
│   ├── mota/
│   │   └── mOTA移植步骤.md           三阶段移植指南，阶段二已按当前工程补充
│   ├── project-docs/
│   │   ├── dev-log.md                编写日志
│   │   ├── project-structure.md      项目结构说明
│   │   └── project-notes.md          项目知识补充与裁剪建议
│   └── cmake-stm32/
│       ├── cmake-stm32-notes.md      CMake / STM32 工程知识积累
│       └── linker-and-boot-notes.md  链接脚本与 Bootloader 笔记
│
├── XY_bootloadr/                     工程一：Bootloader（64KB 救援程序）
│   ├── XY_bootloadr.ioc              STM32CubeMX 工程
│   ├── CMakeLists.txt                CLion 构建入口
│   ├── CMakeLists_template.txt       CubeMX 重新生成时使用的模板
│   ├── STM32F407ZGTX_FLASH.ld        链接脚本：Flash 限制 0x08000000 / 64K
│   ├── STM32F407ZGTX_RAM.ld          链接脚本：SRAM 布局
│   ├── stm32f4discovery.cfg          OpenOCD 调试配置
│   │
│   ├── Core/
│   │   ├── Inc/                      CubeMX 生成的头文件
│   │   ├── Src/                      CubeMX 生成的源文件 + main.c
│   │   ├── Startup/
│   │   └── bootloader/               自定义 bootloader 跳转逻辑
│   │
│   ├── MOTA/                         当前移植的 mOTA 组件
│   │   ├── Core/                     mOTA core / firmware_manage / port 接口
│   │   ├── Config/                   bootloader_config.h
│   │   ├── Bsp/                      BSP Flash 抽象与占位 BSP 头文件
│   │   └── Port/                     STM32F4 Flash read/write/erase 适配
│   │
│   └── Drivers/                      STM32 HAL + CMSIS
│
└── XY7320/                           工程二：APP（960KB 业务程序）
    ├── XY7320.ioc
    ├── CMakeLists.txt
    ├── STM32F407ZGTX_FLASH.ld
    ├── Core/
    └── Drivers/
```

> `cmake-build-*` 是 CLion 临时构建目录，应该被 `.gitignore` 排除，不入库。

---

## 2. Flash 分区图（STM32F407ZGTx 1MB）

```text
0x08000000 ┌──────────────────────┐
           │   Bootloader  64KB   │  Sector 0~3（4 x 16KB）
           │   XY_bootloadr       │  链接脚本 LENGTH = 64K
0x08010000 ├──────────────────────┤
           │                      │
           │     APP    960KB     │  Sector 4~11（1 x 64KB + 7 x 128KB）
           │     XY7320           │  链接脚本 LENGTH = 960K
           │                      │
0x080FFFFF └──────────────────────┘
```

| 区域 | 起始 | 长度 | 所在 Sector |
|---|---:|---:|---|
| Bootloader | `0x08000000` | 64KB | Sector 0~3 |
| APP | `0x08010000` | 960KB | Sector 4~11 |

约束：Bootloader 运行时严禁擦写 Sector 0~3，否则会破坏自身。

---

## 3. Bootloader 启动流程

```text
系统上电 / 复位
   |
   v
Bootloader: 0x08000000
   |
   |-- 判断 APP 首 word 是否像合法 SRAM 栈顶
   |
   |-- APP 有效：关闭中断、清 SysTick/NVIC、设置 VTOR/MSP、跳转 APP
   |
   `-- APP 无效：停留 Bootloader，后续阶段等待串口升级
```

APP 跳转后，APP 侧必须配合：

```c
SCB->VTOR = 0x08010000;
__enable_irq();
```

否则中断向量表仍可能指向 Bootloader，或者中断没有重新打开。

---

## 4. mOTA 阶段二当前状态

阶段二目标：

```text
mOTA core 加入工程
bootloader_config.h 适配 STM32F407ZGTx 1MB Flash
BSP Flash 抽象接到底层 STM32F4 Flash read/write/erase
工程先编译通过
```

当前已完成：

| 项目 | 状态 |
|---|---|
| mOTA core 加入 `XY_bootloadr/MOTA/Core` | 已完成 |
| `bootloader_config.h` 适配 1MB Flash / 64KB Bootloader | 已完成 |
| 使用 `ONE_PART_PROJECT` 单分区方案 | 已完成 |
| `BSP_Flash_Read/Write/Erase` 接入 Port 层 | 已完成 |
| `flash_port_stm32f4.c` 实现 STM32F4 read/write/erase | 已完成 |
| `bootloader_port.c` 阶段二简化 | 已完成 |
| 阶段二工程编译 | 已通过 |
| APP 区 Flash 擦写读回实机验证 | 待验证 |
| 串口 / YModem | 未接入 |

当前阶段二可以认为：**编译目标已通过，Flash 功能还建议补一次擦写读回验证。**

---

## 5. 当前 mOTA 调用链

```text
Bootloader_Init()
   |
   v
FM_Init()
   |
   v
BSP_Flash_Init(APP 分区)

固件写入相关流程
   |
   v
BSP_Flash_Read / BSP_Flash_Write / BSP_Flash_Erase
   |
   v
read / write / erase
   |
   v
HAL_FLASH_Program / HAL_FLASHEx_Erase
```

阶段二暂时不接串口和 YModem，所以 `bootloader_port.c` 中：

```text
Bootloader_Port_Init             空实现
Bootloader_Port_HostDataProcess  空实现
Bootloader_Port_Reset            空实现
Bootloader_Port_JumpToAPP        保留真实跳转逻辑
Bootloader_Port_SystemReset      保留系统复位
```

---

## 6. 资料文档入口

| 文档 | 内容 |
|---|---|
| [`docs/mota/mOTA移植步骤.md`](docs/mota/mOTA移植步骤.md) | 三阶段 mOTA 移植主文档，阶段二已补详细 |
| [`docs/project-docs/dev-log.md`](docs/project-docs/dev-log.md) | 阶段二编写日志 |
| [`docs/project-docs/project-structure.md`](docs/project-docs/project-structure.md) | 项目结构说明 |
| [`docs/project-docs/project-notes.md`](docs/project-docs/project-notes.md) | 项目知识补充、文件作用、裁剪建议 |
| [`docs/cmake-stm32/cmake-stm32-notes.md`](docs/cmake-stm32/cmake-stm32-notes.md) | CMake 与 STM32 工程知识 |
| [`docs/cmake-stm32/linker-and-boot-notes.md`](docs/cmake-stm32/linker-and-boot-notes.md) | 链接脚本、Flash 分区、APP 跳转笔记 |

---

## 7. 下一步计划

建议顺序：

```text
1. 做 APP 区 Flash 擦写读回测试。
2. 确认 Bootloader 不会擦写 Sector 0~3。
3. 先接串口收发，不急着接 YModem。
4. USART2 DMA + IDLE 跑通。
5. 接 data_transfer。
6. 接 protocol_parser / YModem。
7. 使用 APP.fpk 做升级测试。
```

记住：串口是底层通道，YModem 是上层协议。先把串口通道打通，再接协议。