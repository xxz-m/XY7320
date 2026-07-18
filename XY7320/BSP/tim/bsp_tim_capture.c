/**
 * @file    bsp_tim_capture.c
 * @brief   TIM2/TIM4 单通道双边沿输入捕获 BSP 实现
 *
 * 说明：
 *   - TIM2_CH1（PA15）采集信号 A；
 *   - TIM4_CH3（PD14）采集信号 B；
 *   - 当前 CubeMX 配置为双边沿输入捕获、1 MHz 计数频率；
 *   - 本文件只把捕获事件转换为统一的原始事件回调。
 */

#include "bsp_tim_capture.h"

#include "tim.h"

/** 当前注册的捕获事件回调。 */
static BspTimCapture_Callback s_callback = (BspTimCapture_Callback)0;

/** 输入捕获是否已经启动。 */
static volatile uint8_t s_running = 0U;

/**
 * 下一次等待的边沿；A/B 分别维护，避免两路信号互相影响。
 *
 * 捕获流程固定为“上升沿 -> 下降沿 -> 上升沿”。BSP 不读取 GPIO
 * 当前电平判断边沿，而是根据本变量记录本次中断对应的边沿，
 * 从而避免中断到来时 GPIO 电平已经发生变化造成误判。
 */
static BspTimCaptureEdge_t s_nextEdgeA = BSP_TIM_CAPTURE_EDGE_RISING;
static BspTimCaptureEdge_t s_nextEdgeB = BSP_TIM_CAPTURE_EDGE_RISING;

/**
 * @brief 根据定时器句柄读取对应输入捕获值。
 *
 * @param htim 触发捕获的定时器句柄。
 * @return 当前捕获计数值；未知定时器返回 0。
 */
static uint32_t BspTimCapture_ReadValue(const TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return 0U;
    }

    if (htim->Instance == TIM2) {
        return HAL_TIM_ReadCapturedValue((TIM_HandleTypeDef *)htim,
                                         TIM_CHANNEL_1);
    }

    if (htim->Instance == TIM4) {
        return HAL_TIM_ReadCapturedValue((TIM_HandleTypeDef *)htim,
                                         TIM_CHANNEL_3);
    }

    return 0U;
}

/**
 * @brief 获取指定定时器下一次等待的边沿。
 *
 * 动态切换极性后，BSP 不再通过读取 GPIO 电平猜测边沿，
 * 而是使用 A/B 各自保存的期望边沿确定本次捕获类型。
 *
 * @param htim 触发捕获的定时器句柄。
 * @return 本次捕获对应的边沿类型。
 */
static BspTimCaptureEdge_t BspTimCapture_GetExpectedEdge(
    const TIM_HandleTypeDef *htim)
{
    if (htim != NULL && htim->Instance == TIM4) {
        return s_nextEdgeB;
    }

    return s_nextEdgeA;
}

/**
 * @brief 把指定通道切换到下一种捕获极性。
 *
 * @param htim 触发捕获的定时器句柄。
 * @param edge 当前已经捕获的边沿。
 */
static void BspTimCapture_SelectNextEdge(
    TIM_HandleTypeDef *htim,
    BspTimCaptureEdge_t edge)
{
    /*
     * 只修改当前通道的捕获极性，不停止定时器。这样下一次边沿仍由
     * 定时器硬件捕获，周期和脉宽的时间戳不会受到软件轮询延迟影响。
     */
    const uint32_t nextPolarity =
        (edge == BSP_TIM_CAPTURE_EDGE_RISING)
            ? TIM_INPUTCHANNELPOLARITY_FALLING
            : TIM_INPUTCHANNELPOLARITY_RISING;

    /* TIM2/PA15 对应 DMR，TIM4/PD14 对应 GSM；两路边沿状态必须独立维护。 */
    if (htim->Instance == TIM2) {
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, nextPolarity);
        s_nextEdgeA = (edge == BSP_TIM_CAPTURE_EDGE_RISING)
            ? BSP_TIM_CAPTURE_EDGE_FALLING
            : BSP_TIM_CAPTURE_EDGE_RISING;
    }
    else if (htim->Instance == TIM4) {
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, nextPolarity);
        s_nextEdgeB = (edge == BSP_TIM_CAPTURE_EDGE_RISING)
            ? BSP_TIM_CAPTURE_EDGE_FALLING
            : BSP_TIM_CAPTURE_EDGE_RISING;
    }
}

/**
 * @brief 将 HAL 定时器句柄转换为 BSP 信号实例。
 *
 * @param htim 触发捕获的定时器句柄。
 * @return BSP 信号实例；未知定时器返回信号 A 作为占位值，
 *         调用方应先判断是否为 TIM2/TIM4。
 */
static BspTimCaptureSignal_t BspTimCapture_GetSignal(
    const TIM_HandleTypeDef *htim)
{
    if (htim != NULL && htim->Instance == TIM4) {
        return BSP_TIM_CAPTURE_SIGNAL_B;
    }

    return BSP_TIM_CAPTURE_SIGNAL_A;
}

void BspTimCapture_Init(BspTimCapture_Callback callback)
{
    s_callback = callback;
    s_running = 0U;
    s_nextEdgeA = BSP_TIM_CAPTURE_EDGE_RISING;
    s_nextEdgeB = BSP_TIM_CAPTURE_EDGE_RISING;
}

HAL_StatusTypeDef BspTimCapture_Start(void)
{
    HAL_StatusTypeDef statusA;
    HAL_StatusTypeDef statusB;

    /* 两路都从上升沿开始，之后由捕获回调动态切换极性。 */
    __HAL_TIM_SET_CAPTUREPOLARITY(
        &htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
    __HAL_TIM_SET_CAPTUREPOLARITY(
        &htim4, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
    s_nextEdgeA = BSP_TIM_CAPTURE_EDGE_RISING;
    s_nextEdgeB = BSP_TIM_CAPTURE_EDGE_RISING;

    /*
     * 先启动 A，再启动 B。若 B 启动失败，必须回滚 A，避免出现只有
     * 一路仍在产生中断、但上层认为两路都已停止的半启动状态。
     */
    statusA = HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    if (statusA != HAL_OK) {
        return statusA;
    }

    statusB = HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_3);
    if (statusB != HAL_OK) {
        (void)HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
        return statusB;
    }

    /* 只有两路都启动成功后，ISR 才允许向上层发送捕获事件。 */
    s_running = 1U;
    return HAL_OK;
}

HAL_StatusTypeDef BspTimCapture_Stop(void)
{
    HAL_StatusTypeDef statusA;
    HAL_StatusTypeDef statusB;

    /* 先停止硬件中断，再清除运行标志，防止停止过程中继续上报事件。 */
    statusA = HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
    statusB = HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_3);
    s_running = 0U;

    if (statusA != HAL_OK) {
        return statusA;
    }

    return statusB;
}

uint8_t BspTimCapture_IsRunning(void)
{
    return s_running;
}

/**
 * @brief HAL 输入捕获完成回调。
 *
 * 该函数运行在中断上下文，只读取捕获值、识别信号和边沿，
 * 然后把原始事件交给上层回调。
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    BspTimCaptureSignal_t signal;
    BspTimCaptureEdge_t edge;
    uint32_t tick;

    if (htim == NULL || s_running == 0U) {
        return;
    }

    if (htim->Instance != TIM2 && htim->Instance != TIM4) {
        return;
    }

    signal = BspTimCapture_GetSignal(htim);
    edge = BspTimCapture_GetExpectedEdge(htim);
    tick = BspTimCapture_ReadValue(htim);

    /*
     * 先切换下一次极性，再通知上层。即使上层回调触发后立即处理事件，
     * 当前定时器通道也已经处于等待下一边沿的状态。
     */
    BspTimCapture_SelectNextEdge(htim, edge);

    if (s_callback != (BspTimCapture_Callback)0) {
        s_callback(signal, edge, tick);
    }
}
