/**
 * @file    bsp_adc.h
 * @brief   ADC 采集 BSP 模块
 *
 *          基于 STM32 HAL ADC + DMA + TIM3 触发，实现多通道交织扫描。
 *          提供 6 路 active 通道的 512 点缓存 + 2 路 idle 通道累加器。
 *
 *          数据通路：
 *              TIM3 TRGO → ADC 扫描 (8ch) → DMA → 解交织 → 512 点/通道
 *
 *          数据流：
 *              HAL_ADC_ConvCpltCallback → BspAdc_OnConvCpltCallback
 *              → s_samples[ch][idx] / s_idle_acc[ch]
 *              → s_data_ready = 1（512 点满）
 *              → AdcService 消费 → BspAdc_Resume → 下一轮
 *
 * @note    仅供 Services 层调用；App / Domain 不得直接 include。
 * @note    通道编号 active 0..5，idle 0..1（与硬件 rank 一致）。
 */

#ifndef XY7320_BSP_ADC_H
#define XY7320_BSP_ADC_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "adc.h"
#include "tim.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ADC BSP 操作返回码
 */
typedef enum {
    BSP_ADC_OK         =  0,   ///< 操作成功
    BSP_ADC_ERR_PARAM  = -1,   ///< 参数非法（hadc / htim 未初始化、通道号越界）
    BSP_ADC_ERR_HAL    = -2,   ///< HAL 调用失败（ADC Start_DMA / TIM Base Start 失败）
    BSP_ADC_ERR_BUSY   = -3,   ///< 已在运行中
    BSP_ADC_ERR_NOT_READY = -4,///< 数据尚未就绪
} BspAdc_Status;

/**
 * @brief  绑定 ADC / TIM 句柄并清空内部状态
 *
 * @param  hadc  CubeMX 生成的 ADC 句柄（通常为 &hadc1）
 * @param  htim  触发 ADC 采样的 TIM 句柄（通常为 &htim3，使用 TRGO）
 *
 * @note   只保存句柄，不启动外设；后续需 BspAdc_Start() 才会开始采集。
 */
void BspAdc_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim);

/**
 * @brief  启动 ADC DMA 接收并启动 TIM3 触发
 *
 * @return BSP_ADC_OK 成功；BSP_ADC_ERR_PARAM 未初始化；BSP_ADC_ERR_HAL HAL 调用失败；
 *         BSP_ADC_OK（已在运行时不报错）。
 *
 * @note   启动前会清零 s_store_idx / s_round_cnt / s_data_ready / s_idle_acc。
 * @note   DMA 工作在 Circular 模式；HAL_ADC_Start_DMA 仅调用一次，后续靠
 *         HAL_ADC_ConvCpltCallback 持续接收。
 */
BspAdc_Status BspAdc_Start(void);

/**
 * @brief  停止 TIM3 与 ADC DMA，复位内部状态
 */
void BspAdc_Stop(void);

/**
 * @brief  数据消费完毕后恢复下一轮采集
 *
 * @return BSP_ADC_OK 成功；BSP_ADC_ERR_PARAM 未初始化；BSP_ADC_ERR_NOT_READY 上轮数据未消费。
 *
 * @note   仅重启 TIM3，依靠 DMA Circular 模式继续接收。
 * @note   调用方必须先取走 s_samples / s_idle_acc，再调用本函数。
 */
BspAdc_Status BspAdc_Resume(void);

/**
 * @brief  当前采样是否在运行
 *
 * @retval true  正在采集（TIM3 / DMA 启动）
 * @retval false 已停止
 */
bool BspAdc_IsRunning(void);

/**
 * @brief  是否已积累满 512 点（s_data_ready）
 *
 * @retval true  数据已就绪，可通过 BspAdc_GetActiveSamples / BspAdc_GetIdleAverage 读取
 * @retval false 数据未就绪
 */
bool BspAdc_IsDataReady(void);

/**
 * @brief  获取指定 active 通道的 512 点采样数组
 *
 * @param  active_ch  通道号（0..BSP_ADC_ACTIVE_CHANNEL_COUNT-1）
 * @return 指向内部 s_samples[active_ch] 的指针；通道非法或数据未就绪返回 NULL
 *
 * @note   返回的缓冲区由本模块持有，调用方**只读**；若就地修改会污染下次消费。
 *         需要传给 Oscilloscope 等会修改入参的算法时，必须先 memcpy 到调用方缓冲。
 */
const uint16_t *BspAdc_GetActiveSamples(uint8_t active_ch);

/**
 * @brief  获取 idle 通道的累加平均值
 *
 * @param  idle_ch  通道号（0..BSP_ADC_IDLE_CHANNEL_COUNT-1）
 * @return 512 点的算术平均；通道非法或数据未就绪返回 0
 */
uint16_t BspAdc_GetIdleAverage(uint8_t idle_ch);

/**
 * @brief  清除数据就绪标志
 *
 * @note   一般无需调用；BspAdc_Resume 内部已清零。仅在调试时使用。
 */
void BspAdc_ClearReady(void);

/**
 * @brief  ADC 转换完成回调入口
 *
 * @param  hadc  触发回调的 ADC 句柄
 *
 * @note   必须在 stm32f4xx_it.c 的 HAL_ADC_ConvCpltCallback 中调用本函数。
 * @note   函数内部会做句柄匹配、运行状态校验，避免重复进入。
 */
void BspAdc_OnConvCpltCallback(ADC_HandleTypeDef *hadc);

#ifdef __cplusplus
}
#endif
#endif /* XY7320_BSP_ADC_H */
