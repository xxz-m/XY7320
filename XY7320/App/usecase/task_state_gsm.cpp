/**
 * @file    task_state_gsm.cpp
 * @brief   GSM 任务状态实现
 */

#include "task_state_gsm.h"

#include "adc_service.h"
#include "app_config.h"
#include "bsp_tim_capture.h"
#include "input_capture_service.h"

/**
 * GSM 状态对应 PA2/TIM5_CH3 比较器输入。
 * 进入状态后先等待周期和脉宽形成有效结果，再启动 GSM 功率采集。
 */
TaskStateGsm& TaskStateGsm::Instance()
{
    static TaskStateGsm instance;
    return instance;
}

TaskStateGsm::TaskStateGsm()
    : fsm::State("TaskState_GSM"),
      m_powerCaptureStarted(false)
{
}

void TaskStateGsm::entry()
{
    /*
     * 清除上一次 GSM 状态留下的周期/脉宽和排队边沿，避免把旧测量
     * 直接当成当前状态的有效前置条件。
     */
    InputCaptureService::Instance().ResetSignal(
        BSP_TIM_CAPTURE_SIGNAL_B);

    /* 进入新状态先停止 ADC，保证流程从“周期脉宽”阶段重新开始。 */
    AdcService::Instance().Stop();
    m_powerCaptureStarted = false;

    LOG_Printf("TaskState_GSM,Entry,WaitPeriodPulse\n");
}

void TaskStateGsm::exit()
{
    /* 状态切换后不允许 GSM ADC 继续占用采样资源或更新功率结果。 */
    AdcService::Instance().Stop();
    m_powerCaptureStarted = false;

    LOG_Printf("TaskState_GSM,Exit\n");
}

void TaskStateGsm::react(const fsm::Event& event)
{
    (void)event;
}

void TaskStateGsm::tick()
{
    const InputCaptureResult captureResult =
        InputCaptureService::Instance().GetSignalB();

    /*
     * 周期/脉宽是 GSM 功率采集的前置条件。无效或超时期间保持等待，
     * 不启动 ADC，也不使用 InputCaptureService 中的过期结果。
     */
    if (!m_powerCaptureStarted) {
        if (!captureResult.valid) {
            return;
        }

        /*
         * PA2 的比较器结果有效后，切换 ADC 到 GSM 模式，启动正向和
         * 反向功率通道；后续采样由 AdcService::Update() 消费。
         */
        AdcService::Instance().SetScopeMode(SCOPE_MODE_GSM);
        AdcService::Instance().StartTaskB();
        m_powerCaptureStarted = true;
        LOG_Printf("TaskState_GSM,PeriodPulseReady,StartPower\n");
        return;
    }

    /* ADC 尚未产生完整数据时，AdcService::Update() 会自行保持等待。 */
    AdcService::Instance().Update();
}
