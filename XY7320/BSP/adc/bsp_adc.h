//
// Created by Administrator on 2026/6/26.
//

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
//adc状态
typedef enum {
    BSP_ADC_OK = 0,
    BSP_ADC_ERR_PARAM = -1,
    BSP_ADC_ERR_HAL = -2,
    BSP_ADC_ERR_BUSY = -3,
    BSP_ADC_ERR_NOT_READY = -4,
} BspAdc_Status;
//绑定 hadc1 / htim3，清状态
void BspAdc_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim);
//启动 ADC DMA，再启动 TIM3
BspAdc_Status BspAdc_Start(void);
//停 TIM3、停 ADC DMA
void BspAdc_Stop(void);
//上层消费完数据后恢复下一轮采集
BspAdc_Status BspAdc_Resume(void);

bool BspAdc_IsRunning(void);
//查询 512 点数据是否准备好
bool BspAdc_IsDataReady(void);
//获取指定通道 512 点数据
const uint16_t *BspAdc_GetActiveSamples(uint8_t active_ch);
//获取平均值
uint16_t BspAdc_GetIdleAverage(uint8_t idle_ch);
//全部清空
void BspAdc_ClearReady(void);
//给 HAL_ADC_ConvCpltCallback 调用
void BspAdc_OnConvCpltCallback(ADC_HandleTypeDef *hadc);

#ifdef __cplusplus
}
#endif
#endif //XY7320_BSP_ADC_H
