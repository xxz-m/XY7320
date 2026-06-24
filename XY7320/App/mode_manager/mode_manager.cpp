//
// Created by Administrator on 2026/6/24.
//

#include "mode_manager.h"
#include "app_config.h"
#include "task_state_adc_a.h"
#include "task_state_adc_b.h"
#include "task_state_idle.h"
ModeManager &ModeManager::Instance()
{

    static ModeManager instance;
    return instance;
}
void ModeManager::Init()
{
    /* 第一阶段默认进入 Idle */
    currentState_ = &TaskStateIdle::Instance();
    currentMode_ = mode::MODE_IDLE;
    currentState_->entry();
    LOG_Printf("ModeManager,Init,Idle\r\n");
}
void ModeManager::Tick()
{
    if (currentState_) {
        currentState_->tick();
    }
}

void ModeManager::RequestSwitch(const fsm::Event &event)
{
    fsm::State *nextState = nullptr;
    mode::ModeId nextMode = currentMode_;

    switch (event.type()) {
    case mode::EVT_SWITCH_TO_IDLE:
        nextState = &TaskStateIdle::Instance();
        nextMode = mode::MODE_IDLE;
        break;
    case mode::EVT_SWITCH_TO_ADC_TASK_A:
        nextState = &TaskStateAdcA::Instance();
        nextMode = mode::MODE_ADC_TASK_A;
        break;
    case mode::EVT_SWITCH_TO_ADC_TASK_B:
        nextState = &TaskStateAdcB::Instance();
        nextMode = mode::MODE_ADC_TASK_B;
        break;
    default:
        LOG_Printf("ModeManager,UnknownEvent,%d\r\n", event.type());
        return;
    }

    if (nextState == currentState_) {
        return;  // 同模式不切换
    }

    /* exit 旧状态 → 切换 → entry 新状态 */
    if (currentState_) {
        currentState_->exit();
    }
    currentState_ = nextState;
    currentMode_ = nextMode;
    currentState_->entry();

    LOG_Printf("ModeManager,Switch,%s\r\n", currentState_->name());
}
