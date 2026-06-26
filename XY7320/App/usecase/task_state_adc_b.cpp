/**
 * @file    task_state_adc_b.cpp
 * @brief   ADC Task B 状态实现
 *          当前阶段打印心跳，后续替换为真实 ADC B 采集业务
 */

#include "task_state_adc_b.h"
#include "app_config.h"

TaskStateAdcB& TaskStateAdcB::Instance()
{
    static TaskStateAdcB instance;
    return instance;
}

TaskStateAdcB::TaskStateAdcB() : fsm::State("AdcTaskB") {}

/** 进入 ADC Task B，后续在此启动 ADC 采集 */
void TaskStateAdcB::entry()
{
    AdcService::Instance().StartTaskB();
    LOG_Printf("TaskStateAdcB,Entry\n");
}

/** 退出 ADC Task B，后续在此停止 ADC 采集 */
void TaskStateAdcB::exit()
{
    AdcService::Instance().Stop();
    LOG_Printf("TaskStateAdcB,Exit\n");
}

/** ADC Task B 不响应事件，切换由 ModeManager 统一驱动 */
void TaskStateAdcB::react(const fsm::Event &event)
{
}

/** 每 1ms 调用，当前阶段打印心跳，后续替换为真实 ADC B 采集逻辑 */
void TaskStateAdcB::tick()
{
    AdcService::Instance().Update();
}
