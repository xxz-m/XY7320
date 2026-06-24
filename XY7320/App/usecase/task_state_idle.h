//
// Created by Administrator on 2026/6/24.
//

#ifndef XY7320_TASK_STATE_IDLE_H
#define XY7320_TASK_STATE_IDLE_H
#include "fsm.h"


class TaskStateIdle : public fsm::State
{
public:
    static TaskStateIdle &Instance();

    void entry() override;
    void exit() override;
    void react(const fsm::Event &e) override;
    void tick() override;
private:
    TaskStateIdle();
};


#endif //XY7320_TASK_STATE_IDLE_H
