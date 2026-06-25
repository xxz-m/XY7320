/**
 * @file    task_state_idle.cpp
 * @brief   空闲状态实现
 *          当前阶段无业务逻辑，tick() 为空
 */

#include "task_state_idle.h"
#include "app_config.h"

TaskStateIdle& TaskStateIdle::Instance()
{
    static TaskStateIdle instance;
    return instance;
}

TaskStateIdle::TaskStateIdle() : fsm::State("Idle") {}

/** 进入空闲状态 */
void TaskStateIdle::entry()
{
    LOG_Printf("IdleState,Entry\n");
}

/** 退出空闲状态 */
void TaskStateIdle::exit()
{
    LOG_Printf("IdleState,Exit\n");
}

/** 空闲状态不响应任何事件 */
void TaskStateIdle::react(const fsm::Event &event)
{
}

/** 空闲状态无周期任务，后续若需要后台轮询可在此实现 */
void TaskStateIdle::tick()
{
}
