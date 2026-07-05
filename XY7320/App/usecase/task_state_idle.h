/**
 * @file    task_state_idle.h
 * @brief   空闲状态
 *
 *          系统启动后的默认状态，无业务逻辑，等待切换事件。
 */

#ifndef XY7320_TASK_STATE_IDLE_H
#define XY7320_TASK_STATE_IDLE_H

#include "fsm.h"

/**
 * @brief 空闲状态
 *
 * 系统启动后默认进入此状态，tick() 为空（无周期任务）。
 * 收到切换事件后由 ModeManager 驱动退出。
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class TaskStateIdle : public fsm::State
{
public:
    /** @brief 获取单例 */
    static TaskStateIdle& Instance();

    /** @brief 进入状态：仅日志 */
    void entry() override;

    /** @brief 退出状态：仅日志 */
    void exit() override;

    /**
     * @brief 响应事件
     * @param event  fsm 事件
     *
     * 空闲状态不响应任何事件；切换由 ModeManager 统一驱动。
     */
    void react(const fsm::Event& event) override;

    /** @brief 周期回调：当前为空，后续若需要后台轮询可在此实现 */
    void tick() override;

private:
    TaskStateIdle();
};

#endif /* XY7320_TASK_STATE_IDLE_H */