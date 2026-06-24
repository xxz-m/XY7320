/**
* @file    mode_manager.h
 * @brief   模式管理器 — 唯一模式真相源，统一切换入口
 *          基于 fsm::State 实现 exit → 切换 → entry
 */
#ifndef XY7320_MODE_MANAGER_H
#define XY7320_MODE_MANAGER_H

#include "fsm.h"
#include "mode_types.h"

class ModeManager
{
public:
    static ModeManager &Instance();
    void Init();
    void Tick();
    /** 统一切换入口，协议层/业务层都走这里 */
    void RequestSwitch(const fsm::Event &event);
    mode::ModeId currentMode()const{return currentMode_;}

private:
    ModeManager()=default;
    fsm::State *currentState_=nullptr;
    mode::ModeId currentMode_= mode::MODE_IDLE;
};


#endif //XY7320_MODE_MANAGER_H
