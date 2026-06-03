# XY_bootloadr & XY7320 — STM32F407ZGTx 双工程

> 目标芯片：**STM32F407ZGTx**（1MB Flash / 192KB SRAM）
> 开发环境：CLion + STM32CubeCLT 1.21.0 + arm-none-eabi-gcc + OpenOCD
> 参考组件：mOTA v2.0 + STM32F407 bootloader_ymodem 例程

本仓库包含一套 **Bootloader + APP** 双分区 OTA 升级工程，正在按三阶段路线把官方 mOTA 框架移植到自研板子上。

---

## 1. 仓库结构

```
XY7320_ST/
├── README.md                     ← 你正在看的文件（项目总览）
├── mOTA移植步骤.md               ← 详细 3 阶段移植指南（每阶段带验收清单）
├── .gitignore
│
├── XY_bootloadr/                 ← 工程一：Bootloader（64KB 救援程序）
│   ├── .ioc                       STM32CubeMX 工程
│   ├── CMakeLists.txt             CLion 构建入口
│   ├── CMakeLists_template.txt    CubeMX 重新生成时使用的模板
│   ├── STM32F407ZGTX_FLASH.ld     链接脚本：Flash 限制 0x08000000 / 64K
│   ├── STM32F407ZGTX_RAM.ld       链接脚本：SRAM 布局
│   ├── stm32f4discovery.cfg       OpenOCD 调试配置
│   │
│   ├── Core/
│   │   ├── Inc/                   CubeMX 生成的头文件（gpio/dma/usart/it/...）
│   │   ├── Src/                   CubeMX 生成的源文件 + main.c
│   │   ├── Startup/
│   │   │   └── startup_stm32f407zgtx.s
│   │   └── bootloader/            ★ 自定义 bootloader 逻辑
│   │       ├── boot_config.h      Flash 分区 / SRAM 边界宏
│   │       ├── boot_jump.h        Boot_IsValidApp / Boot_JumpToApp 接口
│   │       └── boot_jump.c        合法性判断 + 跳转实现
│   │
│   └── Drivers/                   STM32 HAL + CMSIS（CubeMX 拉取）
│
├── XY7320/                       ← 工程二：APP（960KB 业务程序）
│   ├── .ioc / CMakeLists.txt / *.ld（同上结构）
│   ├── Core/Inc/    Core/Src/    Core/Startup/
│   └── Drivers/
│
└── STM32F407/                    ← 【不入库】官方 bootloader_ymodem 参考例程
    ├── bootloader_ymodem/         mOTA 官方移植样板
    ├── app_v1/    app_v2/         其他 APP 样例
    └── STM32F407_explorer/        板卡原理图
```

> 💡 `cmake-build-debug/` 是 CLion 临时构建目录，已被 `.gitignore` 排除，**不入库**。

---

## 2. 三个核心文件的职责

| 文件 | 角色 | 关键 API |
|---|---|---|
| `Core/bootloader/boot_config.h` | **地址/边界配置中心** | `BOOTLOADER_ADDR` / `APP_ADDRESS` / `SRAM_START_ADDR` 等宏 |
| `Core/bootloader/boot_jump.h` | **对外接口声明** | `Boot_IsValidApp(uint32_t)` / `Boot_JumpToApp(uint32_t)` |
| `Core/bootloader/boot_jump.c` | **合法性判断 + 跳转实现** | 读 APP 首字 → 检查 MSP → 复位外设 → 切 VTOR → 跳转 |

`main.c` 中调用方式（一行决定走哪条路）：

```c
if (Boot_IsValidApp(APP_ADDRESS))
{
    Boot_JumpToApp(APP_ADDRESS);   // APP 有效 → 跳转
}
// 否则留在 bootloader 主循环（后续阶段会改为等待 YModem 升级）
```

---

## 3. Flash 分区图（STM32F407ZGTx 1MB）

```
0x08000000 ┌──────────────────────┐
           │   Bootloader  64KB   │  Sector 0~3（4×16KB）
           │   (XY_bootloadr)     │  链接脚本 LENGTH = 64K
0x08010000 ├──────────────────────┤
           │                      │
           │     APP    960KB     │  Sector 4~11（1×64KB + 7×128KB）
           │    (XY7320)          │  链接脚本 LENGTH = 960K
           │                      │
0x080FFFFF └──────────────────────┘
```

| 区域 | 起始 | 长度 | 链接脚本常量 | 所在 Sector |
|---|---|---|---|---|
| **Bootloader** | `0x08000000` | 64 KB (0x10000) | `BOOTLOADER_ADDR` | S0-S3 |
| **APP** | `0x08010000` | 960 KB (0xF0000) | `APP_ADDRESS` | S4-S11 |

⚠️ 严禁 bootloader 在运行时擦写 Sector 0~3，否则会自杀。

---

## 4. 启动流程

```
            ┌─────────────────────┐
            │   系统上电 / 复位   │
            └──────────┬──────────┘
                       │ PC = 0x08000000
                       ▼
        ┌──────────────────────────────┐
        │   Bootloader (XY_bootloadr)  │
        │   1. HAL_Init                │
        │   2. 时钟 + GPIO + USART     │
        │   3. Boot_IsValidApp()?      │
        └──────────┬──────────┬────────┘
              有效 │          │ 无效
                  ▼          ▼
   ┌─────────────────────┐  ┌──────────────────────────┐
   │ Boot_JumpToApp()    │  │ 停在 bootloader 主循环   │
   │ ├ 关中断            │  │ 阶段一：LED 闪烁          │
   │ ├ 复位 RCC/SysTick  │  │ 阶段三：周期发 'C' 等升级  │
   │ ├ 清 NVIC           │  └──────────────────────────┘
   │ ├ HAL_DeInit        │
   │ ├ SCB->VTOR = APP   │
   │ └ __set_MSP / 跳转  │
   └──────────┬──────────┘
              │ PC = 0x08010000 + 4
              ▼
   ┌──────────────────────────────┐
   │   APP (XY7320)               │
   │   1. SCB->VTOR = 0x08010000  │  ★ 必须重定位向量表
   │   2. __enable_irq()          │  ★ 必须重新开中断
   │   3. HAL_Init / 外设 / 业务  │
   └──────────────────────────────┘
```

**为什么 APP 一开始要做这两件事？** bootloader's `Boot_JumpToApp()` 跳转前会 `__disable_irq()` 并把 `SCB->VTOR` 留在 bootloader 自己的 0x08000000。如果 APP 不重设 VTOR，中断来时会跑去 bootloader 的向量表执行垃圾指令 → HardFault；如果不开中断，`HAL_Delay()` 这种依赖 SysTick 的函数会永远卡死。

---

## 5. 移植进度

| 阶段 | 内容 | 状态 |
|---|---|---|
| **阶段一** | bootloader + APP 跳转 | ✅ **代码完成**（待硬件验证） |
| 1.1 | 链接脚本 Flash 限制 64KB | ✅ |
| 1.2 | `boot_config.h` 地址宏 | ✅ |
| 1.3 | `Boot_IsValidApp()` 合法性判断 | ✅ |
| 1.4 | `Boot_JumpToApp()` 跳转实现 | ✅ |
| 1.5 | `main.c` 接入跳转 | ✅ |
| 1.6 | APP 改地址 + VTOR + `__enable_irq()` | ✅ |
| **阶段二** | mOTA core + Flash 分区管理 | ⏳ 待开始 |
| **阶段三** | YModem 接收 + fpk 升级 | ⏳ 待开始 |

每个阶段都有详细验收清单，见 [`mOTA移植步骤.md`](./mOTA移植步骤.md)。

---

## 6. 烧录顺序（OpenOCD / ST-Link）

```bash
# 1. 先烧 bootloader（覆盖 0x08000000 ~ 0x0800FFFF）
openocd -f stm32f4discovery.cfg \
        -c "program XY_bootloadr/cmake-build-debug/XY_bootloadr.elf 0x08000000 verify reset exit"

# 2. 再烧 APP（覆盖 0x08010000 ~ 0x080FFFFF）
openocd -f stm32f4discovery.cfg \
        -c "program XY7320/cmake-build-debug/XY7320.elf 0x08010000 verify reset exit"
```

> CLion 里直接用 `OCD_XY7320.xml` / `OCD_XY_bootloadr.xml` 跑配置即可（已自动生成）。

---

## 7. 关键约束（容易踩的坑）

1. **升级的是 `.fpk` 不是 `.bin`** —— mOTA 框架只认 Firmware_Packager 打包出来的格式
2. **fpk 表头必须 1024 byte** —— 因为 YModem-1K 每帧数据区就是 1024 byte
3. **第一版用 `USING_HOST_CMD_UPDATE`** —— 不用 `USING_APP_SET_FLAG_UPDATE`，因为后者在 GCC 下需要专门处理 `.noinit` 段
4. **串口分配** —— `USART1` = 日志，`USART2` = YModem（已配 DMA1_Stream5 + IDLE 中断）
5. **不跳阶段** —— 阶段一没跑通别急阶段二，阶段二没跑通别急阶段三

---

## 8. 文件大小速查

| 文件 | 作用 |
|---|---|
| `boot_config.h` | 改 Flash/APP 地址就改这里 |
| `boot_jump.h/.c` | 不动，改完注释就不管 |
| `XY_bootloadr/STM32F407ZGTX_FLASH.ld` | bootloader 链接脚本（**已改 64K**） |
| `XY7320/STM32F407ZGTX_FLASH.ld` | APP 链接脚本（**已改 0x08010000 / 960K**） |
| `XY7320/Core/Src/main.c` | 入口加了 VTOR + `__enable_irq()` |

---

## 9. 下一步

1. **硬件验证阶段一**：两个工程都重编译 + 烧录，看 APP 的 LED 能不能正常闪
2. **通过后**开阶段二：把 mOTA core 拉进来
3. **最后**做阶段三：YModem 串口升级

详情 → [`mOTA移植步骤.md`](./mOTA移植步骤.md)
