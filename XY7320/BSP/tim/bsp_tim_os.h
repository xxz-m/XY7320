/**
 * @file    bsp_tim_os.h
 * @brief   OS 调度器定时器 + 系统时间接口
 *          负责将 CubeMX 生成的定时器配置为 OS Tick 源，
 *          并提供系统时间获取和系统复位封装
 */

#ifndef XY7320_BSP_TIM_OS_H
#define XY7320_BSP_TIM_OS_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Tick 回调函数类型 */
typedef void (*BspTimOs_TickCallback)(void);

/**
 * @brief  初始化 OS Tick 定时器
 *         根据 os_config.h 中的 OS_TICK_MS 自动计算 PSC/ARR
 *         在 main() 中 MX_TIMx_Init() 之后调用
 * @param  htim  CubeMX 生成的定时器句柄（如 &htim14）
 */
void BspTimOs_Init(TIM_HandleTypeDef *htim);

/**
 * @brief  注册 Tick 回调
 *         在定时器中断中调用此回调，用于驱动调度器
 *         由 main() 在初始化阶段注入，避免 BSP 反向依赖 System
 * @param  cb  回调函数（如 OS_Tick）
 */
void BspTimOs_SetTickCallback(BspTimOs_TickCallback cb);

/**
 * @brief  OS Tick 定时器中断处理
 *         在 stm32f4xx_it.c 的定时器中断 handler 中调用
 *         内部调用已注册的回调函数
 */
void BspTimOs_IRQHandler(void);

/**
 * @brief  获取系统运行时间（毫秒）
 * @return 自 HAL_Init() 以来的毫秒数
 */
uint32_t BspTimOs_GetTick(void);

/**
 * @brief  阻塞延时（毫秒）
 * @param  ms  延时毫秒数
 */
void BspTimOs_DelayMs(uint32_t ms);

/**
 * @brief  系统复位（NVIC_SystemReset 封装）
 */
void BspTimOs_SystemReset(void);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_BSP_TIM_OS_H */
