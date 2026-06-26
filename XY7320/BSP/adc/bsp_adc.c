//
// Created by Administrator on 2026/6/26.
//

#include "bsp_adc.h"
#include "bsp_config.h"

static ADC_HandleTypeDef *s_hadc = NULL;
static TIM_HandleTypeDef *s_htim = NULL;
// DMA 直接写入的交织原始数据
static uint16_t s_dma_buf[BSP_ADC_DMA_BUF_LEN];
//解交织后的前 6 路，每路 512 个样本
static uint16_t s_samples[BSP_ADC_ACTIVE_CHANNEL_COUNT]
                         [BSP_ADC_TARGET_SAMPLE_COUNT];
//后 2 路累加器
static uint32_t s_idle_acc[BSP_ADC_IDLE_CHANNEL_COUNT];
//一批 512 点数据已满
static volatile uint8_t s_data_ready = 0;
//当前采样是否运行
static volatile uint8_t s_running = 0;
//当前写入偏移0、64、128...448
static uint16_t s_store_idx = 0;
// DMA TC：0~7
static uint8_t s_round_cnt = 0;

void BspAdc_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim)
{
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

BspAdc_Status BspAdc_Start(void)
{
    if (s_hadc == NULL || s_htim == NULL) {
        return BSP_ADC_ERR_PARAM;
    }

    if (s_running) {
        return BSP_ADC_OK;
    }

    s_store_idx = 0;
    s_round_cnt = 0;
    s_data_ready = 0;
    memset(s_idle_acc, 0, sizeof(s_idle_acc));

    if (HAL_ADC_Start_DMA(s_hadc,
                          (uint32_t *)s_dma_buf,
                          BSP_ADC_DMA_BUF_LEN) != HAL_OK) {
        return BSP_ADC_ERR_HAL;
                          }

    if (HAL_TIM_Base_Start(s_htim) != HAL_OK) {
        HAL_ADC_Stop_DMA(s_hadc);
        return BSP_ADC_ERR_HAL;
    }

    s_running = 1;
    return BSP_ADC_OK;
}

void BspAdc_Stop(void)
{
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

void BspAdc_OnConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc ==NULL|| s_hadc == NULL)return;
    if (hadc->Instance!=s_hadc->Instance)return;
    if (!s_running||s_data_ready) return;
    for (uint16_t scan=0;scan<BSP_ADC_SCAN_COUNT_PER_DMA;++scan)
    {
        uint16_t base = scan*BSP_ADC_CHANNEL_COUNT;
        for (uint8_t ch=0;ch<BSP_ADC_ACTIVE_CHANNEL_COUNT;++ch)
            s_samples[ch][s_store_idx+scan] = s_dma_buf[base+ch];
        s_idle_acc[0] +=s_dma_buf[base+6];
        s_idle_acc[1] +=s_dma_buf[base+7];
    }
    s_store_idx+=BSP_ADC_SCAN_COUNT_PER_DMA;
    s_round_cnt++;
    if (s_round_cnt >= BSP_ADC_DMA_ROUND_COUNT)
    {
        HAL_TIM_Base_Stop(s_htim);
        s_data_ready = 1;
        s_running = 0;
    }

}

BspAdc_Status BspAdc_Resume(void)
{
    if (s_hadc == NULL || s_htim == NULL) {
        return BSP_ADC_ERR_PARAM;
    }

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
bool BspAdc_IsRunning(void)
{
    return s_running != 0;
}

bool BspAdc_IsDataReady(void)
{
    return s_data_ready != 0;
}

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

void BspAdc_ClearReady(void)
{
    s_data_ready = 0;
}