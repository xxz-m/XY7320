/**
 * @file    bsp_tim_os.h
 * @brief   OS 调度器定时器 - BSP 层接口
 *          负责将 CubeMX 生成的定时器配置为 OS Tick 源
 *          读取 Common/config/os_config.h 中的 OS_TICK_MS，自动计算硬件参数
 */
#ifndef BSP_TIM_OS_H
#define BSP_TIM_OS_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  初始化 OS Tick 定时器
 *         根据 os_config.h 中的 OS_TICK_MS 自动计算 PSC/ARR
 *         在 main() 中 MX_TIMx_Init() 之后调用
 * @param  htim  CubeMX 生成的定时器句柄（如 &htim14）
 */
void BSP_TIM_OS_Init(TIM_HandleTypeDef *htim);

/**
 * @brief  OS Tick 定时器中断处理
 *         在 stm32f4xx_it.c 的定时器中断 handler 中调用
 *         内部会调用 OS_Tick() 驱动调度器
 */
void BSP_TIM_OS_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TIM_OS_H */
