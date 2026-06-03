/**
 * ============================================================================
 *  XY7320 Bootloader - APP 跳转模块
 * ============================================================================
 *
 *  提供两个核心功能：
 *    1. Boot_IsValidApp() → 判断指定地址处是否存在合法的 APP 固件
 *    2. Boot_JumpToApp()  → 清理当前环境，跳转到 APP 运行
 *
 *  调用流程示例（在 main.c 中）：
 *  @code
 *  if (Boot_IsValidApp(APP_ADDRESS))
 *  {
 *      Boot_JumpToApp(APP_ADDRESS);   // 有 APP → 跳转
 *  }
 *  // 无 APP → 停在 bootloader 等待升级
 *  @endcode
 *
 *  依赖：
 *    boot_config.h → 提供 APP_ADDRESS、SRAM_START_ADDR、SRAM_END_ADDR 等宏
 *    main.h        → STM32 HAL 库类型定义
 * ============================================================================
 */

#ifndef XY_BOOTLOADR_BOOT_JUMP_H
#define XY_BOOTLOADR_BOOT_JUMP_H

#include "main.h"
#include "boot_config.h"

/**
 * @brief  检查指定地址处是否存在合法的 APP 固件
 * @param  app_addr  APP 起始地址（例如 0x08010000）
 * @retval 1   APP 存在且栈顶有效
 * @retval 0   APP 不存在或已损坏
 * @note   判断依据：APP 首地址前 4 字节是初始 MSP
 *         合法 MSP 应在 SRAM 范围内（0x20000000 ~ 0x20020000）
 */
int  Boot_IsValidApp(uint32_t app_addr);

/**
 * @brief  从 bootloader 跳转到 APP 运行
 * @param  app_addr  APP 起始地址（例如 0x08010000）
 * @note   跳转前会执行以下清理工作：
 *           1. 关闭全局中断
 *           2. 复位 RCC（恢复时钟外设到默认状态）
 *           3. 关闭 SysTick
 *           4. 清除所有 NVIC 中断挂起
 *           5. HAL_DeInit（反初始化所有外设）
 *           6. 设置 SCB->VTOR = APP 地址（重定位中断向量表）
 *           7. 设置 MSP = APP 初始栈顶
 *           8. 跳转到 APP 的 Reset_Handler
 * @warning 该函数不会返回，如果 APP 无效会死循环
 */
void Boot_JumpToApp(uint32_t app_addr);

#endif /* XY_BOOTLOADR_BOOT_JUMP_H */
