/**
 * @file    mode_manager.h
 * @brief   模式管理器
 *
 *          状态机核心调度，管理 Idle/DMR/GSM 三种模式的切换。
 */

#ifndef XY7320_MODE_MANAGER_H
#define XY7320_MODE_MANAGER_H

#include "fsm.h"
#include "mode_types.h"

/**
 * @brief 模式管理器
 *
 * 职责：
 *  1. 维护当前状态指针和模式枚举
 *  2. 提供统一切换入口 RequestSwitch()
 *  3. 每 1ms 由 OS 任务驱动 Tick()
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class ModeManager {
public:
    /** @brief 获取单例 */
    static ModeManager& Instance();

    /**
     * @brief 初始化状态机，默认进入 Idle 状态
     *
     * 由 App_Main::App_Main_Init 调用。
     */
    void Init();

    /**
     * @brief 每 1ms 调用，驱动当前状态的 tick()
     *
     * 由 App_Main 的 Task_ModeManager 调用。
     */
    void Tick();

    /**
     * @brief 统一切换入口，协议层/业务层都走这里
     *
     * @param event  携带目标模式的事件
     */
    void RequestSwitch(const fsm::Event &event);

    /** @brief 获取当前模式 ID */
    mode::ModeId currentMode() const { return currentMode_; }

private:
    ModeManager() = default;

    fsm::State *currentState_ = nullptr;          ///< 当前状态指针
    mode::ModeId currentMode_ = mode::MODE_IDLE;  ///< 当前模式枚举
};

#endif /* XY7320_MODE_MANAGER_H */