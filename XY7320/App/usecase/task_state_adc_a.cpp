//
// Created by Administrator on 2026/6/24.
//

#include "task_state_adc_a.h"
#include "log_service.h"
#include "app_config.h"
TaskStateAdcA &TaskStateAdcA::Instance()
{
    static TaskStateAdcA instance;
    return instance;
}

TaskStateAdcA::TaskStateAdcA() : fsm::State("AdcTaskA") {}

void TaskStateAdcA::entry()
{
    LOG_Printf("TaskStateAdcA,Entry\r\n");
}

void TaskStateAdcA::exit()
{
    LOG_Printf("TaskStateAdcA,Exit\r\n");
}

void TaskStateAdcA::react(const fsm::Event &e)
{
    (void)e;
}

void TaskStateAdcA::tick()
{
    /* 第一阶段：打印心跳，后续替换为真实 ADC A 业务 */
    LOG_Printf("TaskStateAdcA,Tick\r\n");
}