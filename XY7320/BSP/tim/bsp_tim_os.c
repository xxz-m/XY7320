/**
 * @file    bsp_tim_os.c
 * @brief   OS 调度器定时器 + 系统时间接口实现
 *          通过回调注入驱动调度器，不直接依赖 System 层头文件
 *
 * 职责边界：
 *   CubeMX (tim.c)    → 负责时钟使能、GPIO、中断优先级等基础初始化
 *   BSP 层 (本文件)    → 只修正 PSC/ARR 为 OS 所需值，启动中断
 *
 * 依赖方向：BSP → Common/config（读取配置）
 *           BSP ← System（通过回调注入，无反向 include）
 */
#include "bsp_tim_os.h"
#include "os_config.h"

/* ============================================================
 *  硬件参数（根据 OS_TICK_MS 自动计算）
 * ============================================================ */

/**
 * 定时器时钟频率（Hz）
 * STM32F407:
 *   APB1 定时器 (TIM2/3/4/5/6/7/12/13/14) = 84MHz
 *   APB2 定时器 (TIM1/8/9/10/11)           = 168MHz
 *
 * 切换定时器时只需改这个值
 */
#define BSP_TIM_OS_CLK_HZ     84000000u

/**
 * 预分频值：将定时器时钟分频到 1MHz（1us/计数）
 * PSC = CLK_HZ / 1000000 - 1
 * 84MHz → PSC = 83
 */
#define BSP_TIM_OS_PSC        (BSP_TIM_OS_CLK_HZ / 1000000u - 1u)

/**
 * 自动重装载值：产生 OS_TICK_MS 毫秒的中断
 * ARR = 1000 * OS_TICK_MS - 1
 * OS_TICK_MS=1 → ARR = 999（1ms 中断）
 * OS_TICK_MS=5 → ARR = 4999（5ms 中断）
 */
#define BSP_TIM_OS_ARR        (1000u * OS_TICK_MS - 1u)

/* ============================================================
 *  内部状态
 * ============================================================ */

/** Tick 回调（由 main() 注入，指向 OS_Tick） */
static BspTimOs_TickCallback s_tickCb = (BspTimOs_TickCallback)0;

/* ============================================================
 *  接口实现
 * ============================================================ */

void BspTimOs_Init(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return;
    }

    /*
     * CubeMX 的 MX_TIMx_Init() 已经调用过 HAL_TIM_Base_Init()，
     * 完成了时钟使能、GPIO、中断优先级等基础配置。
     *
     * 这里只修正 PSC/ARR 为 OS 调度器需要的值，
     * 然后启动定时器中断。
     *
     * 不再重复调用 HAL_TIM_Base_Init()，避免冲突。
     */
    __HAL_TIM_SET_PRESCALER(htim, BSP_TIM_OS_PSC);
    __HAL_TIM_SET_AUTORELOAD(htim, BSP_TIM_OS_ARR);

    /* 启动定时器中断（CubeMX 只做了初始化，没有启动） */
    if (HAL_TIM_Base_Start_IT(htim) != HAL_OK) {
        Error_Handler();
    }
}

void BspTimOs_SetTickCallback(BspTimOs_TickCallback cb)
{
    s_tickCb = cb;
}

void BspTimOs_IRQHandler(void)
{
    if (s_tickCb) {
        s_tickCb();
    }
}

uint32_t BspTimOs_GetTick(void)
{
    return HAL_GetTick();
}

void BspTimOs_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

void BspTimOs_SystemReset(void)
{
    NVIC_SystemReset();
}
