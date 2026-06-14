/**
 * ============================================================================
 *  XY7320 Bootloader - 地址与分区配置
 * ============================================================================
 *
 *  芯片：STM32F407ZGTx（片内 Flash 1MB, SRAM 128KB, CCMRAM 64KB）
 *
 *  Flash 分区规划（第一阶段 - 单分区方案）：
 *
 *  ┌──────────────────────┐ 0x08000000  ← BOOTLOADER_ADDR
 *  │                      │
 *  │   bootloader (64KB)  │             ← 当前工程，上电最先运行
 *  │   占 Sector 0~3       │
 *  │                      │
 *  ├──────────────────────┤ 0x08010000  ← APP_ADDRESS
 *  │                      │
 *  │   APP (最大 960KB)    │             ← 用户固件，由 bootloader 跳转
 *  │   占 Sector 4~11      │
 *  │                      │
 *  └──────────────────────┘ 0x08100000  ← Flash 末尾
 *
 *  为什么 bootloader 选 64KB？
 *    STM32F407 前 4 个 Sector 都是 16KB
 *    Sector 0: 0x08000000 ~ 0x08003FFF  (16KB)
 *    Sector 1: 0x08004000 ~ 0x08007FFF  (16KB)
 *    Sector 2: 0x08008000 ~ 0x0800BFFF  (16KB)
 *    Sector 3: 0x0800C000 ~ 0x0800FFFF  (16KB)
 *    合计刚好 64KB，地址对齐，擦除安全
 *
 *  SRAM 布局：
 *  ┌──────────────────────┐ 0x20000000  ← SRAM_START_ADDR
 *  │   普通 SRAM (128KB)   │
 *  │   栈和变量都在这里     │
 *  └──────────────────────┘ 0x20020000  ← SRAM_END_ADDR
 *
 *  如何判断 APP 是否有效？
 *    APP bin 文件的前 4 字节是初始 MSP（主栈指针）
 *    合法 APP 的初始 MSP 必须落在 SRAM 区域（0x20000000 ~ 0x20020000）
 *    如果不在此范围，说明 APP 不存在或已损坏
 *
 * @note  第一阶段（当前）：单分区，APP 直接覆盖升级
 *        后续可升级为 APP+download 双分区，或 APP+download+factory 三分区
 * ============================================================================
 */

#ifndef XY_BOOTLOADR_BOOT_CONFIG_H
#define XY_BOOTLOADR_BOOT_CONFIG_H

/* ======================== Bootloader 自身 ======================== */

/** Bootloader 起始地址（芯片上电入口） */
#define BOOTLOADER_ADDR         0x08000000U
/** Bootloader 占用 Flash 大小（64KB） */
#define BOOTLOADER_SIZE         0x00010000U

/* ======================== APP 分区 ======================== */

/** APP 固件起始地址 */
#define APP_ADDRESS             0x08010000U
/** APP 区最大可用空间（1MB - 64KB = 960KB） */
#define APP_PART_SIZE           0x000D0000U
#define APP_CONFIG_ADDRESS      0x080E0000U
#define APP_CONFIG_SIZE         0x00020000U

/* ======================== SRAM 地址范围 ======================== */
/* 用于判断 APP 栈顶是否合法 */

/** STM32F407 普通 SRAM 起始地址 */
#define SRAM_START_ADDR         0x20000000U
/** STM32F407 普通 SRAM 大小（128KB） */
#define SRAM_SIZE               0x00020000U
/** STM32F407 普通 SRAM 结束地址（不含 CCMRAM） */
#define SRAM_END_ADDR           (SRAM_START_ADDR + SRAM_SIZE)

#endif /* XY_BOOTLOADR_BOOT_CONFIG_H */
