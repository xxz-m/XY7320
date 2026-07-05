//
// Created by Administrator on 2026/6/26.
//

#include "bsp_adc.h"
#include "bsp_config.h"

/* ====== 模块私有状态 ====== */

static ADC_HandleTypeDef *s_hadc = NULL;   ///< 当前绑定的 ADC 句柄
static TIM_HandleTypeDef *s_htim = NULL;   ///< 当前绑定的 TIM 句柄（TRGO 触发）

/** DMA 直接写入的交织原始数据，8 通道 × 64 点/DMA TC。 */
static uint16_t s_dma_buf[BSP_ADC_DMA_BUF_LEN];

/** 解交织后的前 6 路 active 通道缓存，每通道 512 点。 */
static uint16_t s_samples[BSP_ADC_ACTIVE_CHANNEL_COUNT]
                         [BSP_ADC_TARGET_SAMPLE_COUNT];

/** 后 2 路 idle 通道累加器；累计 512 点后由 BspAdc_GetIdleAverage 求平均。 */
static uint32_t s_idle_acc[BSP_ADC_IDLE_CHANNEL_COUNT];

/** 标记一批 512 点数据已满，可被上层读取。DMA 完成 N 轮后置 1。 */
static volatile uint8_t s_data_ready = 0;

/** 当前采样是否运行（TIM3 + DMA 已启动）。 */
static volatile uint8_t s_running = 0;

/** 当前写入偏移：0、64、128 ... 448；每收到一次 DMA TC 加 BSP_ADC_SCAN_COUNT_PER_DMA。 */
static uint16_t s_store_idx = 0;

/** DMA TC 计数：0..BSP_ADC_DMA_ROUND_COUNT-1，达到上限后停 TIM3。 */
static uint8_t s_round_cnt = 0;

/**
 * @brief 绑定 ADC / TIM 句柄并清空内部状态
 *
 * 只保存句柄，不启动外设；后续需 BspAdc_Start() 才会开始采集。
 */
void BspAdc_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim)
{
    /* 保存句柄 + 清空所有缓存与状态机 */
    s_hadc = hadc;
    s_htim = htim;

    memset(s_dma_buf, 0, sizeof(s_dma_buf));
    memset(s_samples, 0, sizeof(s_samples));
    memset(s_idle_acc, 0, sizeof(s_idle_acc));

    s_store_idx = 0;
    s_round_cnt = 0;
    s_data_ready = 0;
    s_running = 0;
}

/**
 * @brief 启动 ADC DMA 接收并启动 TIM3 触发
 *
 * 启动前清零 s_store_idx / s_round_cnt / s_data_ready / s_idle_acc。
 * DMA 工作在 Circular 模式；HAL_ADC_Start_DMA 仅调用一次，后续靠
 * HAL_ADC_ConvCpltCallback 持续接收。
 *
 * @return BSP_ADC_OK 成功；BSP_ADC_ERR_PARAM 未初始化；BSP_ADC_ERR_HAL HAL 调用失败。
 *         已在运行时不报错，返回 BSP_ADC_OK。
 */
BspAdc_Status BspAdc_Start(void)
{
    if (s_hadc == NULL || s_htim == NULL) {
        return BSP_ADC_ERR_PARAM;
    }

    /* 已在运行直接返回，避免重复启动导致 DMA 状态错乱 */
    if (s_running) {
        return BSP_ADC_OK;
    }

    /* 重置接收进度：512 点满后才置 s_data_ready */
    s_store_idx = 0;
    s_round_cnt = 0;
    s_data_ready = 0;
    memset(s_idle_acc, 0, sizeof(s_idle_acc));

    /* 1. 启动 ADC + DMA（Circular 模式，仅调一次） */
    if (HAL_ADC_Start_DMA(s_hadc,
                          (uint32_t *)s_dma_buf,
                          BSP_ADC_DMA_BUF_LEN) != HAL_OK) {
        return BSP_ADC_ERR_HAL;
                          }

    /* 2. 启动 TIM3，TRGO 周期性触发 ADC 扫描 */
    if (HAL_TIM_Base_Start(s_htim) != HAL_OK) {
        HAL_ADC_Stop_DMA(s_hadc);
        return BSP_ADC_ERR_HAL;
    }

    s_running = 1;
    return BSP_ADC_OK;
}

/**
 * @brief 停止 TIM3 与 ADC DMA，复位内部状态
 */
void BspAdc_Stop(void)
{
    /* 停外设、复位内部状态机。多次调用安全。 */
    if (s_htim != NULL) {
        HAL_TIM_Base_Stop(s_htim);
    }

    if (s_hadc != NULL) {
        HAL_ADC_Stop_DMA(s_hadc);
    }

    s_running = 0;
    s_data_ready = 0;
    s_store_idx = 0;
    s_round_cnt = 0;
}

/**
 * @brief ADC 转换完成回调入口
 *
 * 在 stm32f4xx_it.c 的 HAL_ADC_ConvCpltCallback 中调用本函数。
 * 函数内部会做句柄匹配、运行状态校验，避免重复进入。
 *
 * 解交织排列：[ch0_t0, ch1_t0, ..., ch7_t0, ch0_t1, ...]
 * 每个 scan 内的 8 个点对应同一时刻的 8 个通道；active 通道写入 s_samples，
 * idle 通道仅累加供后续 BspAdc_GetIdleAverage 求平均。
 *
 * 当 s_round_cnt 达到 BSP_ADC_DMA_ROUND_COUNT（=8）即 512 点已满，
 * 停 TIM3、置 s_data_ready = 1、s_running = 0。
 */
void BspAdc_OnConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    /* 句柄过滤：忽略非本模块管理的 ADC */
    if (hadc == NULL || s_hadc == NULL) return;
    if (hadc->Instance != s_hadc->Instance) return;

    /* 已停机或上一批数据未消费则跳过，避免覆盖 s_samples */
    if (!s_running || s_data_ready) return;

    for (uint16_t scan = 0; scan < BSP_ADC_SCAN_COUNT_PER_DMA; ++scan) {
        uint16_t base = scan * BSP_ADC_CHANNEL_COUNT;

        /* active 通道：直接落库到 s_samples[ch][s_store_idx+scan] */
        for (uint8_t ch = 0; ch < BSP_ADC_ACTIVE_CHANNEL_COUNT; ++ch) {
            s_samples[ch][s_store_idx + scan] = s_dma_buf[base + ch];
        }

        /* idle 通道：仅累加，不保存逐点数据 */
        s_idle_acc[0] += s_dma_buf[base + 6];
        s_idle_acc[1] += s_dma_buf[base + 7];
    }

    s_store_idx += BSP_ADC_SCAN_COUNT_PER_DMA;
    s_round_cnt++;

    /* 已收满 8 轮 = 512 点，停 TIM3 并通知上层 */
    if (s_round_cnt >= BSP_ADC_DMA_ROUND_COUNT) {
        HAL_TIM_Base_Stop(s_htim);
        s_data_ready = 1;
        s_running = 0;
    }
}

/**
 * @brief 数据消费完毕后恢复下一轮采集
 *
 * DMA 处于 Circular 模式，无需重启 HAL_ADC_Start_DMA；只需重启 TIM3。
 * 调用方必须先取走 s_samples / s_idle_acc，再调用本函数。
 *
 * @return BSP_ADC_OK 成功；BSP_ADC_ERR_PARAM 未初始化；BSP_ADC_ERR_NOT_READY 上轮数据未消费。
 */
BspAdc_Status BspAdc_Resume(void)
{
    if (s_hadc == NULL || s_htim == NULL) {
        return BSP_ADC_ERR_PARAM;
    }

    /* 必须先把上一批数据消费掉（BspAdc_IsDataReady 已为 true） */
    if (!s_data_ready) {
        return BSP_ADC_ERR_NOT_READY;
    }

    s_store_idx = 0;
    s_round_cnt = 0;
    s_data_ready = 0;
    memset(s_idle_acc, 0, sizeof(s_idle_acc));

    if (HAL_TIM_Base_Start(s_htim) != HAL_OK) {
        return BSP_ADC_ERR_HAL;
    }

    s_running = 1;
    return BSP_ADC_OK;
}

/**
 * @brief 当前采样是否在运行
 * @retval true 正在采集（TIM3 / DMA 启动）
 * @retval false 已停止
 */
bool BspAdc_IsRunning(void)
{
    return s_running != 0;
}

/**
 * @brief 是否已积累满 512 点（s_data_ready）
 *
 * @retval true  数据已就绪，可通过 BspAdc_GetActiveSamples / BspAdc_GetIdleAverage 读取
 * @retval false 数据未就绪
 */
bool BspAdc_IsDataReady(void)
{
    return s_data_ready != 0;
}

/**
 * @brief 获取指定 active 通道的 512 点采样数组
 *
 * 返回的缓冲区由本模块持有，调用方**只读**；若就地修改会污染下次消费。
 * 需要传给 Oscilloscope 等会修改入参的算法时，必须先 memcpy 到调用方缓冲。
 *
 * @param active_ch 通道号（0..BSP_ADC_ACTIVE_CHANNEL_COUNT-1）
 * @return 指向内部 s_samples[active_ch] 的指针；通道非法或数据未就绪返回 NULL
 */
const uint16_t *BspAdc_GetActiveSamples(uint8_t active_ch)
{
    if (active_ch >= BSP_ADC_ACTIVE_CHANNEL_COUNT) {
        return NULL;
    }

    if (!s_data_ready) {
        return NULL;
    }

    return s_samples[active_ch];
}

/**
 * @brief 获取 idle 通道的累加平均值
 *
 * @param idle_ch 通道号（0..BSP_ADC_IDLE_CHANNEL_COUNT-1）
 * @return 512 点的算术平均；通道非法或数据未就绪返回 0
 */
uint16_t BspAdc_GetIdleAverage(uint8_t idle_ch)
{
    if (idle_ch >= BSP_ADC_IDLE_CHANNEL_COUNT) {
        return 0;
    }

    if (!s_data_ready) {
        return 0;
    }

    return (uint16_t)(s_idle_acc[idle_ch] / BSP_ADC_TARGET_SAMPLE_COUNT);
}

/**
 * @brief 清除数据就绪标志
 *
 * 一般无需调用；BspAdc_Resume 内部已清零。仅在调试时使用。
 */
void BspAdc_ClearReady(void)
{
    s_data_ready = 0;
}