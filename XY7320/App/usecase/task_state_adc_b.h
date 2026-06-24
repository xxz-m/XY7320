//
// Created by Administrator on 2026/6/24.
//

#ifndef XY7320_TASK_STATE_ADC_B_H
#define XY7320_TASK_STATE_ADC_B_H

#include "fsm.h"
class TaskStateAdcB:public fsm::State
{
public:
    static TaskStateAdcB&Instance();
    void entry() override;
    void exit() override;
    void react(const fsm::Event &e) override;
    void tick() override;
private:
    TaskStateAdcB();
};


#endif //XY7320_TASK_STATE_ADC_B_H
