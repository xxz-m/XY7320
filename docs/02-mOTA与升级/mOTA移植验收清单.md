---
title: mOTA 移植验收清单
module: mota
type: checklist
tags: [mota, type/guide, status/current]
created: 2026-06-11
related: "[[mOTA移植指南]], [[Bootloader开发指南]]"
---

# mOTA 移植验收清单

> [!info] 使用说明
> 本清单对应 [[mOTA移植指南]] 中的三阶段移植流程。每完成一个阶段后逐项检查，全部通过后再进入下一阶段。不要跳过阶段一和阶段二直接做阶段三。

---

## 阶段一：Bootloader + APP 跳转

> [!todo] 阶段目标
> 先不移植完整 mOTA，只实现 Bootloader 启动、判断 APP 是否存在、跳转 APP、APP 从 0x08010000 正常运行。这一阶段是所有后续工作的基础。

- [ ] `XY_bootloadr` 可以正常编译
- [ ] Bootloader 链接脚本 Flash 限制为 64KB（`FLASH : ORIGIN = 0x08000000, LENGTH = 64K`）
- [ ] Bootloader 可以烧录运行
- [ ] APP 链接地址为 `0x08010000`（`FLASH : ORIGIN = 0x08010000, LENGTH = 960K`）
- [ ] APP 已设置 `SCB->VTOR = 0x08010000`
- [ ] APP 启动后执行了 `__enable_irq()`
- [ ] Bootloader 可以判断 APP 是否有效（首地址栈顶值落在 SRAM 区域）
- [ ] Bootloader 可以成功跳转 APP
- [ ] 跳转后 APP 不 HardFault

> [!check] 阶段一完成后检查
> ```text
> 链接脚本是否正确
> APP 地址是否为 0x08010000
> 跳转函数是否完整（关中断、停 SysTick、清 NVIC、设 VTOR、设 MSP）
> APP 是否设置了 SCB->VTOR 和 __enable_irq
> 是否有可能 HardFault 的路径
> ```

**阶段一不通过，不进入阶段二。**

---

## 阶段二：mOTA 核心 + Flash 管理

> [!todo] 阶段目标
> 引入 mOTA 核心，但还不跑完整 YModem。让 mOTA 核心源码加入工程、`bootloader_config.h` 适配 Flash、片内 Flash 分区管理接入、APP 区可以读/擦/写、工程能编译通过。

- [ ] mOTA core 已加入工程
- [ ] `bootloader_config.h` 已适配 STM32F407ZGTx 1MB Flash
- [ ] 当前配置为 `ONE_PART_PROJECT`
- [ ] 第一版未启用 SPI Flash / FAL / SFUD
- [ ] 第一版未启用 AES
- [ ] `BSP_Flash_Read/Write/Erase` 已接到底层 `read/write/erase`
- [ ] `flash_port_stm32f4.c` 已限制只能操作 APP 区（Sector 4~11）
- [ ] 工程完整编译通过
- [ ] `BSP_Flash_Read()` 实机读取验证
- [ ] `BSP_Flash_Erase()` 实机擦除 APP 区验证
- [ ] `BSP_Flash_Write()` 实机写入 APP 区验证
- [ ] 写入后读回校验一致
- [ ] Bootloader 区不会被擦除或写坏

> [!check] 阶段二完成后检查
> ```text
> bootloader_config.h 分区配置是否正确
> Flash 分区是否与链接脚本一致
> read/write/erase 三个函数是否正确实现
> 是否会误擦 Bootloader 区（Sector 0~3）
> CMake include/source 路径是否完整
> mOTA core 编译依赖是否满足
> 占位 .h 是否只是声明，没有被误认为已经实现
> ```

**Flash 没验证通过前，不建议直接进入完整 YModem。**

---

## 阶段三：串口 + YModem

> [!todo] 阶段目标
> 接入完整升级流程：Bootloader 上电等待升级，周期发送字符 C，上位机通过 YModem-1K 发送 APP.fpk，Bootloader 接收 fpk，解析表头，写入 APP 区，校验固件，跳转 APP。

- [ ] USART2 作为 YModem 接收口可用
- [ ] USART1 作为日志口可用
- [ ] USART2 DMA RX 正常工作
- [ ] USART2 IDLE 中断可以触发一帧结束
- [ ] BSP Timer 已接入 SysTick
- [ ] Bootloader 可以周期发送字符 `'C'`
- [ ] YModem_Sender 可以发送 `.fpk` 文件
- [ ] `.fpk` 表头尺寸为 1024 byte
- [ ] Bootloader 可以收到 SOH/STX/EOT
- [ ] 固件包头校验通过
- [ ] APP 区 Sector4 ~ Sector11 可以正确擦除
- [ ] 固件数据写入 `0x08010000` 后读回正常
- [ ] 固件完整性校验通过
- [ ] 升级完成后可以跳转 APP
- [ ] 重启后可以识别 APP 有效并跳转 APP
- [ ] 升级失败不会破坏 Bootloader 自身

> [!check] 阶段三完成后检查
> ```text
> USART2 DMA 是否正常
> USART2 IDLE 中断是否正常触发
> BSP UART 初始化是否正确
> BSP Timer 是否正常计时
> YModem 流程是否完整跑通
> fpk 表头尺寸是否为 1024 byte
> 固件写入和校验流程是否正确
> ```

---

## 最终完成标准

全部移植完成后，应达到以下标准：

- [ ] Bootloader 位于 `0x08000000`，大小控制在 64KB 内
- [ ] APP 位于 `0x08010000`
- [ ] APP 设置 `SCB->VTOR = 0x08010000`，并重新开启中断
- [ ] Bootloader 上电后等待 YModem 升级
- [ ] 未收到升级数据时，可以跳转已有 APP
- [ ] 收到 APP.fpk 后，可以写入 APP 区
- [ ] 写入完成后校验 APP
- [ ] 校验成功后跳转 APP
- [ ] APP 不存在或损坏时，停留 Bootloader 等待升级
- [ ] Bootloader 自身不会被擦除或写坏

---

## 相关文档

- [[mOTA移植指南]] -- 三阶段移植详细步骤
- [[Bootloader开发指南]] -- Bootloader 启动跳转流程
