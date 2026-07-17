/**
 * @file    task_state_dmr.cpp
 * @brief   DMR 任务状态实现
 */

#include "task_state_dmr.h"

#include "adc_service.h"
#include "app_config.h"
#include "bsp_tim_capture.h"
#include "input_capture_service.h"
#include "uart_tx_service.h"
#include "mode_manager.h"
#include "configurations.h"
#include "bsp_tim_os.h"

namespace {
constexpr uint32_t kDmrUploadPeriodMs = 200U;
constexpr uint8_t kDmrPayloadSize = 26U;

void WriteU16Be(uint8_t *dst, uint16_t value)
{
    dst[0] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    dst[1] = static_cast<uint8_t>(value & 0xFFU);
}

void WriteU32Be(uint8_t *dst, uint32_t value)
{
    dst[0] = static_cast<uint8_t>((value >> 24) & 0xFFU);
    dst[1] = static_cast<uint8_t>((value >> 16) & 0xFFU);
    dst[2] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    dst[3] = static_cast<uint8_t>(value & 0xFFU);
}
}

/**
 * DMR 状态对应 PA15/TIM2_CH1 比较器输入。
 * 进入状态后先等待周期和脉宽形成有效结果，再启动 400/450 功率采集。
 */
TaskStateDmr& TaskStateDmr::Instance()
{
    static TaskStateDmr instance;
    return instance;
}

TaskStateDmr::TaskStateDmr()
    : fsm::State("TaskState_DMR"),
      m_powerCaptureStarted(false),
      m_lastUploadMs(0U),
      m_uploadSeq(0U)
{
}

void TaskStateDmr::entry()
{
    /*
     * 清除上一次 DMR 状态留下的周期/脉宽和排队边沿，避免把旧测量
     * 直接当成当前状态的有效前置条件。
     */
    InputCaptureService::Instance().ResetSignal(
        BSP_TIM_CAPTURE_SIGNAL_A);

    /* 进入新状态先停止 ADC，保证流程从“周期脉宽”阶段重新开始。 */
    AdcService::Instance().Stop();
    m_powerCaptureStarted = false;
    m_lastUploadMs = 0U;
    m_uploadSeq = 0U;

    LOG_Printf("TaskState_DMR,Entry,WaitPeriodPulse\n");
}

void TaskStateDmr::exit()
{
    /* 状态切换后不允许 DMR ADC 继续占用采样资源或更新功率结果。 */
    AdcService::Instance().Stop();
    m_powerCaptureStarted = false;

    LOG_Printf("TaskState_DMR,Exit\n");
}

void TaskStateDmr::react(const fsm::Event& event)
{
    (void)event;
}

uint16_t TaskStateDmr::nextUploadSeq()
{
    ++m_uploadSeq;
    return m_uploadSeq;
}

void TaskStateDmr::tick()
{
    const InputCaptureResult captureResult =
        InputCaptureService::Instance().GetSignalA();

    /*
     * 周期/脉宽是 DMR 功率采集的前置条件。无效或超时期间保持等待，
     * 不启动 ADC，也不使用 InputCaptureService 中的过期结果。
     */
    if (!m_powerCaptureStarted) {
        if (!captureResult.valid) {
            return;
        }

        /*
         * PA15 的比较器结果有效后，切换 ADC 到 DMR 的 400/450 模式，
         * 启动正向和反向功率通道；后续采样由 AdcService::Update() 消费。
         */
        AdcService::Instance().SetScopeMode(SCOPE_MODE_400_450);
        AdcService::Instance().StartTaskA();
        m_powerCaptureStarted = true;
        LOG_Printf("TaskState_DMR,PeriodPulseReady,StartPower\n");
        return;
    }

    /* ADC 尚未产生完整数据时，AdcService::Update() 会自行保持等待。 */
    AdcService::Instance().Update();

    /*
     * 占位上行：把当前 tick 周期内的"最新一次有效信号"打包成 DMR 上行帧，
     * 通过 UartTxService 异步发出。当前 payload 字段为占位，真值确定后再
     * 替换为 DmrPowerMeasurement 给出的序列化字段集合。
     */
    const uint32_t now = BspTimOs_GetTick();
    if ((now - m_lastUploadMs) < kDmrUploadPeriodMs) {
        return;
    }
    m_lastUploadMs = now;

    const DMRPowerData_t powerData = AdcService::Instance().GetDmrPowerData();
    uint8_t payload[kDmrPayloadSize]{};
    payload[0] = ModeManager::Instance().freqModel();
    payload[1] = ModeManager::Instance().catchModel();
    WriteU16Be(&payload[2], static_cast<uint16_t>(powerData.dbm1_x100_413));
    WriteU16Be(&payload[4], static_cast<uint16_t>(powerData.dbm2_x100_413));
    WriteU16Be(&payload[6], powerData.p1v_413);
    WriteU16Be(&payload[8], powerData.p2v_413);
    WriteU16Be(&payload[10], static_cast<uint16_t>(powerData.dbm1_x100_457));
    WriteU16Be(&payload[12], static_cast<uint16_t>(powerData.dbm2_x100_457));
    WriteU16Be(&payload[14], powerData.p1v_457);
    WriteU16Be(&payload[16], powerData.p2v_457);
    WriteU32Be(&payload[18], powerData.valid ? captureResult.pulseWidthUs : 0U);
    WriteU32Be(&payload[22], powerData.valid ? captureResult.periodUs : 0U);

    (void)UartTxService::Instance().PublishModeData(
        UPLINK_CMD_DMR_MEAS,
        mode::MODE_DMR,
        ModeManager::Instance().currentGeneration(),
        nextUploadSeq(),
        payload,
        sizeof(payload));
}
