/**
 * @file    mode_manager.cpp
 * @brief   模式管理器实现
 *          状态机核心：Init/Tick/RequestSwitch，驱动状态切换
 */

#include "mode_manager.h"
#include "app_config.h"
#include "task_state_idle.h"
#include "task_state_dmr.h"
#include "task_state_gsm.h"
#include "task_gnss.h"
#include "uart_tx_service.h"

ModeManager& ModeManager::Instance()
{
    static ModeManager instance;
    return instance;
}

/** 初始化状态机，默认进入 Idle 状态 */
void ModeManager::Init()
{
    /* 第一阶段默认进入 Idle */
    currentState_ = &TaskStateIdle::Instance();
    currentState_->entry();
    currentMode_ = mode::MODE_IDLE;
    previousMode_ = mode::MODE_IDLE;
    currentGeneration_ = 1U;
    previousGeneration_ = 0U;
    LOG_Printf("ModeManager,Init,%s\n", currentState_->name());
}

/** 每 1ms 由 OS 任务驱动，调用当前状态的 tick() */
void ModeManager::Tick()
{
    currentState_->tick();
}

/**
 * 统一切换入口，协议层/业务层都走这里
 *
 * 切换流程：exit 旧状态 → 切换指针 → entry 新状态
 * 同模式重复切换直接忽略，避免无意义的 exit/entry
 */
void ModeManager::RequestSwitch(const fsm::Event &event)
{
    fsm::State *nextState = nullptr;
    mode::ModeId nextMode = mode::MODE_IDLE;

    switch (event.type()) {
    case mode::EVT_SWITCH_TO_IDLE:
        nextState = &TaskStateIdle::Instance();
        nextMode = mode::MODE_IDLE;
        break;
    case mode::EVT_SWITCH_TO_DMR:
        nextState = &TaskStateDmr::Instance();
        nextMode = mode::MODE_DMR;
        break;
    case mode::EVT_SWITCH_TO_GSM:
        nextState = &TaskStateGsm::Instance();
        nextMode = mode::MODE_GSM;
        break;
    case mode ::EVT_SWITCH_TO_GNSS:
        nextState = &TaskstateGnss::Instance();
        nextMode = mode::MODE_GNSS;
        break;
    default:
        /* 未知事件只打印不切换，避免无意义的 exit/entry 抖动 */
        LOG_Printf("ModeManager,UnknownEvent,%d\n", event.type());
        return;
    }

    /* 同模式不切换 */
    if (nextState == currentState_) {
        return;
    }

    /* exit 旧状态 -> 切换 -> entry 新状态 */
    currentState_->exit();

    /* 记录旧会话信息，供 UartTxService 清旧 Pending 使用 */
    previousMode_ = currentMode_;
    previousGeneration_ = currentGeneration_;

    currentState_ = nextState;
    currentMode_ = nextMode;
    currentState_->entry();

    /*
     * entry 之后才 ++currentGeneration_ 并通知 UartTxService：
     *   - 新状态已就绪，再让 UART TX 清理旧 Pending；
     *   - generation 自增让 DMR->Idle->DMR 的旧 DMR Pending 与新 DMR Pending
     *     在 UartTxService::PickModeSlotForSend 中被识别为 stale 并丢弃。
     */
    ++currentGeneration_;
    UartTxService::Instance().OnModeChanged(previousMode_,
                                            previousGeneration_,
                                            currentMode_,
                                            currentGeneration_);

    LOG_Printf("ModeManager,Switch,%s,Gen,%lu\n",
                currentState_->name(),
                static_cast<unsigned long>(currentGeneration_));
}
