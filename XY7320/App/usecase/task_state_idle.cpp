/**
 * @file    task_state_idle.cpp
 * @brief   空闲状态实现
 *
 *          当前阶段无业务逻辑，tick() 为空。
 */

#include "task_state_idle.h"
#include "app_config.h"

TaskStateIdle& TaskStateIdle::Instance()
{
    static TaskStateIdle instance;
    return instance;
}

TaskStateIdle::TaskStateIdle() : fsm::State("Idle") {}

/** @brief 进入空闲状态：仅写日志 */
void TaskStateIdle::entry()
{
    LOG_Printf("IdleState,Entry\n");
}

/** @brief 退出空闲状态：仅写日志 */
void TaskStateIdle::exit()
{
    LOG_Printf("IdleState,Exit\n");
}

/**
 * @brief 空闲状态不响应任何事件
 *
 * 切换由 ModeManager 统一驱动；本状态对事件保持沉默。
 */
void TaskStateIdle::react(const fsm::Event& event)
{
    (void)event;
}

/** @brief 空闲状态无周期任务，后续若需要后台轮询可在此实现 */
void TaskStateIdle::tick()
{
}