/**
 * @file    task_state_idle.h
 * @brief   空闲状态
 *          系统启动后的默认状态，无业务逻辑，等待切换事件
 */

#ifndef XY7320_TASK_STATE_IDLE_H
#define XY7320_TASK_STATE_IDLE_H

#include "fsm.h"

/**
 * 空闲状态
 *
 * 系统启动后默认进入此状态，tick() 为空（无周期任务）。
 * 收到切换事件后由 ModeManager 驱动退出。
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class TaskStateIdle : public fsm::State {
public:
    static TaskStateIdle& Instance();

    void entry() override;
    void exit() override;
    void react(const fsm::Event &event) override;
    void tick() override;

private:
    TaskStateIdle();
};

#endif //XY7320_TASK_STATE_IDLE_H
