/**
 * @file    adc_service.cpp
 * @brief   ADC 采集服务实现
 *
 *          负责管理 BSP ADC 的启停与数据消费，并将通道快照
 *          交给 Oscilloscope 滤波引擎处理。
 */

#include "adc_service.h"
#include "bsp_adc.h"
#include "app_config.h"
#include "bsp_config.h"
#include "adc.h"
#include "tim.h"

/**
 * @brief AdcService 单例
 */
AdcService& AdcService::Instance()
{
    static AdcService instance;
    return instance;
}

/**
 * @brief 初始化 BSP ADC 与 Oscilloscope
 *
 * 绑定 hadc1 / htim3，初始化 Oscilloscope（默认 SCOPE_MODE_400_450）。
 * 不启动 ADC，由上层 StartTaskA / StartTaskB 触发。
 */
void AdcService::Init()
{
    BspAdc_Init(&hadc1, &htim3);
    mode_ = Mode::Idle;

    /* 默认进入 400/450 模式；切到 TaskB 时由 SetScopeMode 切换 */
    auto& scope = Oscilloscope::getInstance();
    scope.initOscilloscope();
    m_scopeMode = SCOPE_MODE_400_450;

}

/**
 * @brief 切换 Oscilloscope 模式
 *
 * 同模式不重复调用；切模式时 Oscilloscope 内部会清空 WaveHoldState 抗闪烁状态机。
 *
 * @param mode SCOPE_MODE_400_450 / SCOPE_MODE_GSM
 */
void AdcService::SetScopeMode(ScopeMode_t mode)
{
    if (m_scopeMode == mode) return;
    m_scopeMode = mode;
    Oscilloscope::getInstance().setMode(mode);
}

/**
 * @brief 进入 TaskA 模式（400/450 四通道采集）
 *
 * 重启 BSP ADC；模式切换由调用方在 ModeManager state enter 中负责
 * （如果当前不是 SCOPE_MODE_400_450，需要先调 SetScopeMode）。
 */
void AdcService::StartTaskA()
{
    mode_ = Mode::TaskA;
    m_dmrPowerMeasurement.Reset();
    m_dmrPowerData = {};

    BspAdc_Stop();
    BspAdc_Start();

    LOG_Printf("AdcService,StartTaskA\n");
}

/**
 * @brief 进入 TaskB 模式（GSM 双通道采集）
 */
void AdcService::StartTaskB()
{
    mode_ = Mode::TaskB;
    m_gsmPowerMeasurement.Reset();
    m_gsmPowerData = {};
    BspAdc_Stop();
    BspAdc_Start();
    LOG_Printf("AdcService,StartTaskB\n");
}

/**
 * @brief 停止采集，回到 Idle
 */
void AdcService::Stop()
{
    BspAdc_Stop();
    mode_ = Mode::Idle;

    LOG_Printf("AdcService,Stop\n");
}

/**
 * @brief 周期轮询入口（由 ModeManager Task_ModeManager 每 1ms 驱动）
 *
 * - Idle 直接返回
 * - 数据未就绪直接返回
 * - 按 mode_ 分发到 ProcessTaskA / ProcessTaskB
 * - 最后调用 BspAdc_Resume 启动下一轮 DMA 接收
 */
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

/**
 * @brief TaskA 内部处理：拷贝 CH0~CH3 → Oscilloscope 400/450 滤波
 *
 * 步骤：
 *  1. memcpy BSP 缓冲到 m_snapshot（Oscilloscope 会原地修改入参）
 *  2. 仅在 m_scopeMode == SCOPE_MODE_400_450 时调用 TickLoop400_450
 *  3. 每 40 帧（约 40ms）通过 LOG_Printf 输出一次滤波结果
 */
void AdcService::ProcessTaskA()
{
    static uint16_t log_div = 0;

    /* 1) 拷贝原始数据到本地快照，避免 Oscilloscope 修改 BSP 内部缓冲 */
    for (uint8_t ch = 0; ch < 4; ++ch) {
        const uint16_t* src = BspAdc_GetActiveSamples(ch);
        if (src == nullptr) return;
        std::memcpy(m_snapshot[ch], src,
                    sizeof(uint16_t) * BSP_ADC_TARGET_SAMPLE_COUNT);
    }

    /* 2) 仅在 400/450 模式下跑 Oscilloscope */
    auto& scope = Oscilloscope::getInstance();
    if (m_scopeMode == SCOPE_MODE_400_450) {
        scope.TickLoop400_450(m_snapshot[0], m_snapshot[1],
                              m_snapshot[2], m_snapshot[3]);
        m_filteredResult = scope.getResult();
        m_dmrPowerMeasurement.Update(m_filteredResult.wavePEP1_avg,
                                     m_filteredResult.wavePEP2_avg,
                                     m_filteredResult.wavePEP3_avg,
                                     m_filteredResult.wavePEP4_avg,
                                     HAL_GetTick(),
                                     &m_dmrPowerData);
    }

    /* 3) 日志降频：每 40 帧打印一次 */
    if (++log_div >= 40u) {
        log_div = 0;
        LOG_Printf("AdcTaskA,DMR,adc,%u,%u,%u,%u,dbmX100,%d,%d,%d,%d,uw,%lu,%lu,%lu,%lu,pepUw,%lu,%lu,%lu,%lu,valid,%u\n",
                   m_dmrPowerData.p1v_413,
                   m_dmrPowerData.p2v_413,
                   m_dmrPowerData.p1v_457,
                   m_dmrPowerData.p2v_457,
                   m_dmrPowerData.dbm1_x100_413,
                   m_dmrPowerData.dbm2_x100_413,
                   m_dmrPowerData.dbm1_x100_457,
                   m_dmrPowerData.dbm2_x100_457,
                   (unsigned long)m_dmrPowerData.w1x_uw_413,
                   (unsigned long)m_dmrPowerData.w2x_uw_413,
                   (unsigned long)m_dmrPowerData.w1x_uw_457,
                   (unsigned long)m_dmrPowerData.w2x_uw_457,
                   (unsigned long)m_dmrPowerData.w1x_uw_pep_413,
                   (unsigned long)m_dmrPowerData.w2x_uw_pep_413,
                   (unsigned long)m_dmrPowerData.w1x_uw_pep_457,
                   (unsigned long)m_dmrPowerData.w2x_uw_pep_457,
                   m_dmrPowerData.valid ? 1u : 0u);
    }
}

/**
 * @brief TaskB 内部处理：拷贝 CH4~CH5 → Oscilloscope GSM 滤波
 *
 * 步骤同 ProcessTaskA，仅通道范围不同。
 */
void AdcService::ProcessTaskB()
{
    static uint16_t log_div = 0;

    /* 1) 拷贝 CH4~CH5 原始数据到本地快照 */
    for (uint8_t ch = 4; ch < 6; ++ch) {
        const uint16_t* src = BspAdc_GetActiveSamples(ch);
        if (src == nullptr) return;
        std::memcpy(m_snapshot[ch], src,
                    sizeof(uint16_t) * BSP_ADC_TARGET_SAMPLE_COUNT);
    }

    /* 2) 仅在 GSM 模式下跑 Oscilloscope 和功率测量 */
    auto& scope = Oscilloscope::getInstance();
    if (m_scopeMode == SCOPE_MODE_GSM) {
        scope.TickLoopGSM(m_snapshot[4], m_snapshot[5]);
        m_filteredResult = scope.getResult();
        m_gsmPowerMeasurement.Update(m_filteredResult.wavePEP5_avg,
                                     m_filteredResult.wavePEP6_avg,
                                     HAL_GetTick(),
                                     &m_gsmPowerData);
    }

    /* 3) 日志降频：每 40 帧打印一次，dBm 使用 dBm*100 定点值 */
    if (++log_div >= 40u) {
        log_div = 0;
        LOG_Printf("AdcTaskB,GSM,adc,%u,%u,dbmX100,%d,%d,uw,%lu,%lu,pepUw,%lu,%lu,valid,%u\n",
                    m_gsmPowerData.p1v,
                    m_gsmPowerData.p2v,
                    m_gsmPowerData.dbm1_x100,
                    m_gsmPowerData.dbm2_x100,
                   (unsigned long)m_gsmPowerData.w1x_uw,
                   (unsigned long)m_gsmPowerData.w2x_uw,
                   (unsigned long)m_gsmPowerData.w1x_uw_pep,
                   (unsigned long)m_gsmPowerData.w2x_uw_pep,
                   m_gsmPowerData.valid ? 1u : 0u);
    }
}
