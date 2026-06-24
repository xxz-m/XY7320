//
// Created by Administrator on 2026/6/24.
//

#include "task_state_idle.h"
#include "log_service.h"
#include "app_config.h"
TaskStateIdle &TaskStateIdle::Instance()
{
    static TaskStateIdle instance;
    return instance;
}

TaskStateIdle :: TaskStateIdle():fsm::State("Idle"){}

void TaskStateIdle :: entry()
{
    LOG_Printf("IdleState,Entry\r\n");
}

void TaskStateIdle :: exit()
{
    LOG_Printf("IdleState,Exit\r\n");
}
void TaskStateIdle :: react(const fsm::Event &e)
{
    (void)e;
}
void TaskStateIdle::tick()
{
    /* 第一阶段：心跳验证调度器在跑 */
    static uint32_t cnt = 0;
    if (++cnt % 1000 == 0) {
        LOG_Printf("IdleState,Tick,%lu\r\n", cnt);
    }
}