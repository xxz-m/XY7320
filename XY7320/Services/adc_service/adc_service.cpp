//
// Created by Administrator on 2026/6/26.
//

#include "adc_service.h"
#include "bsp_adc.h"
#include "app_config.h"
#include "bsp_config.h"
#include "adc.h"
#include "tim.h"

AdcService& AdcService::Instance()
{
    static AdcService instance;
    return instance;
}

void AdcService::Init()
{
    BspAdc_Init(&hadc1, &htim3);
    mode_ = Mode::Idle;
}
void AdcService::StartTaskA()
{
    mode_ = Mode::TaskA;

    BspAdc_Stop();
    BspAdc_Start();

    LOG_Printf("AdcService,StartTaskA\n");
}
void AdcService::StartTaskB()
{
    mode_ = Mode::TaskB;
    BspAdc_Stop();
    BspAdc_Start();
    LOG_Printf("AdcService,StartTaskB\n");
}

void AdcService::Stop()
{
    BspAdc_Stop();
    mode_ = Mode::Idle;

    LOG_Printf("AdcService,Stop\n");
}

void AdcService::Update()
{
    if (mode_ == Mode::Idle) {
        return;
    }

    if (!BspAdc_IsDataReady()) {
        return;
    }

    if (mode_ == Mode::TaskA) {
        ProcessTaskA();
    } else if (mode_ == Mode::TaskB) {
        ProcessTaskB();
    }

    BspAdc_Resume();
}

void AdcService::ProcessTaskA()
{
    static uint16_t log_div = 0;

    const uint16_t *ch0 = BspAdc_GetActiveSamples(0);
    const uint16_t *ch1 = BspAdc_GetActiveSamples(1);
    const uint16_t *ch2 = BspAdc_GetActiveSamples(2);
    const uint16_t *ch3 = BspAdc_GetActiveSamples(3);

    if (ch0 == nullptr || ch1 == nullptr || ch2 == nullptr || ch3 == nullptr) {
        return;
    }

    uint32_t sum0 = 0;
    uint32_t sum1 = 0;
    uint32_t sum2 = 0;
    uint32_t sum3 = 0;

    for (uint16_t i = 0; i < BSP_ADC_TARGET_SAMPLE_COUNT; ++i) {
        sum0 += ch0[i];
        sum1 += ch1[i];
        sum2 += ch2[i];
        sum3 += ch3[i];
    }

    uint16_t avg0 = sum0 / BSP_ADC_TARGET_SAMPLE_COUNT;
    uint16_t avg1 = sum1 / BSP_ADC_TARGET_SAMPLE_COUNT;
    uint16_t avg2 = sum2 / BSP_ADC_TARGET_SAMPLE_COUNT;
    uint16_t avg3 = sum3 / BSP_ADC_TARGET_SAMPLE_COUNT;

    if (++log_div >= 40u) {
        log_div = 0;
        LOG_Printf("AdcTaskA,Avg,%u,%u,%u,%u\n", avg0, avg1, avg2, avg3);
    }
}

void AdcService::ProcessTaskB()
{
    static uint16_t log_div = 0;

    const uint16_t *ch4 = BspAdc_GetActiveSamples(4);
    const uint16_t *ch5 = BspAdc_GetActiveSamples(5);

    if (ch4 == nullptr || ch5 == nullptr) {
        return;
    }

    uint32_t sum4 = 0;
    uint32_t sum5 = 0;

    for (uint16_t i = 0; i < BSP_ADC_TARGET_SAMPLE_COUNT; ++i) {
        sum4 += ch4[i];
        sum5 += ch5[i];
    }

    uint16_t avg4 = sum4 / BSP_ADC_TARGET_SAMPLE_COUNT;
    uint16_t avg5 = sum5 / BSP_ADC_TARGET_SAMPLE_COUNT;

    uint16_t idle0 = BspAdc_GetIdleAverage(0);
    uint16_t idle1 = BspAdc_GetIdleAverage(1);

    if (++log_div >= 40u) {
        log_div = 0;
        LOG_Printf("AdcTaskB,Avg,%u,%u,Idle,%u,%u\n", avg4, avg5, idle0, idle1);
    }
}