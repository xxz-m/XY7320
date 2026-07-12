/**
 * @file    bsp_tim_capture.h
 * @brief   TIM2/TIM5 单通道双边沿输入捕获 BSP
 *
 * 当前硬件映射：
 *   - 信号 A：PA15 / TIM2_CH1
 *   - 信号 B：PA2  / TIM5_CH3
 *
 * 本模块只负责启动/停止输入捕获、读取捕获计数值、识别边沿并
 * 通过回调上报原始捕获事件；周期、脉宽、频率等业务计算放在
 * Services 层，不在中断回调中执行。
 */

#ifndef XY7320_BSP_TIM_CAPTURE_H
#define XY7320_BSP_TIM_CAPTURE_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 输入捕获信号实例。 */
typedef enum
{
    BSP_TIM_CAPTURE_SIGNAL_A = 0U,
    BSP_TIM_CAPTURE_SIGNAL_B = 1U,
} BspTimCaptureSignal_t;

/** 输入信号边沿类型。 */
typedef enum
{
    BSP_TIM_CAPTURE_EDGE_RISING = 0U,
    BSP_TIM_CAPTURE_EDGE_FALLING = 1U,
} BspTimCaptureEdge_t;

/**
 * @brief 输入捕获事件回调类型。
 *
 * 回调运行在 TIM 捕获中断上下文，只允许采集数据、更新最小状态
 * 或投递事件；禁止执行日志、复杂计算、阻塞调用和模式切换。
 *
 * @param signal 信号 A 或信号 B。
 * @param edge   本次捕获的边沿类型。
 * @param tick   定时器捕获计数值，当前配置下 1 tick 约等于 1 us。
 */
typedef void (*BspTimCapture_Callback)(
    BspTimCaptureSignal_t signal,
    BspTimCaptureEdge_t edge,
    uint32_t tick);

/**
 * @brief 初始化输入捕获 BSP。
 *
 * CubeMX 已完成 TIM2/TIM5 的时钟、GPIO、NVIC 和输入捕获参数配置；
 * 本函数只清理 BSP 状态并注册上层回调，不启动捕获。
 *
 * @param callback 捕获事件回调，可传 NULL 表示暂不接收事件。
 */
void BspTimCapture_Init(BspTimCapture_Callback callback);

/**
 * @brief 启动 TIM2_CH1 和 TIM5_CH3 的输入捕获中断。
 * @return HAL_OK 表示两个通道均启动成功；否则返回 HAL 错误码。
 */
HAL_StatusTypeDef BspTimCapture_Start(void);

/**
 * @brief 停止 TIM2_CH1 和 TIM5_CH3 的输入捕获中断。
 * @return HAL_OK 表示两个通道均停止成功；否则返回 HAL 错误码。
 */
HAL_StatusTypeDef BspTimCapture_Stop(void);

/**
 * @brief 查询输入捕获是否已启动。
 * @return 1 表示已启动，0 表示未启动。
 */
uint8_t BspTimCapture_IsRunning(void);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_BSP_TIM_CAPTURE_H */
