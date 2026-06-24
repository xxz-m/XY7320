//
// Created by Administrator on 2026/6/24.
//

#include "task_state_adc_b.h"
#include "log_service.h"
#include "app_config.h"
TaskStateAdcB &TaskStateAdcB::Instance()
{
    static TaskStateAdcB instance;
    return instance;
}

TaskStateAdcB::TaskStateAdcB() : fsm::State("AdcTaskB") {}

void TaskStateAdcB::entry()
{
    LOG_Printf("TaskStateAdcB,Entry\r\n");
}

void TaskStateAdcB::exit()
{
    LOG_Printf("TaskStateAdcB,Exit\r\n");
}

void TaskStateAdcB::react(const fsm::Event &e)
{
    (void)e;
}

void TaskStateAdcB::tick()
{
    /* 第一阶段：打印心跳，后续替换为真实 ADC A 业务 */
    LOG_Printf("TaskStateAdcB,Tick\r\n");
}