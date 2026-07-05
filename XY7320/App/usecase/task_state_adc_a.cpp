/**
 * @file    task_state_adc_a.cpp
 * @brief   ADC Task A 状态实现
 *          当前阶段打印心跳，后续替换为真实 ADC A 采集业务
 */

#include "task_state_adc_a.h"
#include "app_config.h"

TaskStateAdcA& TaskStateAdcA::Instance()
{
    static TaskStateAdcA instance;
    return instance;
}

TaskStateAdcA::TaskStateAdcA() : fsm::State("AdcTaskA") {}

/** 进入 ADC Task A，后续在此启动 ADC 采集 */
void TaskStateAdcA::entry()
{
    AdcService::Instance().StartTaskA();
    LOG_Printf("TaskStateAdcA,Entry\n");
}

/** 退出 ADC Task A，后续在此停止 ADC 采集 */
void TaskStateAdcA::exit()
{
    AdcService::Instance().Stop();
    AdcService::Instance().SetScopeMode(SCOPE_MODE_GSM);
    LOG_Printf("TaskStateAdcA,Exit\n");
}

/** ADC Task A 不响应事件，切换由 ModeManager 统一驱动 */
void TaskStateAdcA::react(const fsm::Event &event)
{
}

/** 每 1ms 调用，当前阶段打印心跳，后续替换为真实 ADC A 采集逻辑 */
void TaskStateAdcA::tick()
{
    AdcService::Instance().Update();
}
