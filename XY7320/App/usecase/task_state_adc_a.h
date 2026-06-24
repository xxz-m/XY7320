//
// Created by Administrator on 2026/6/24.
//

#ifndef XY7320_TASK_STATE_ADC_A_H
#define XY7320_TASK_STATE_ADC_A_H

#include "fsm.h"
class TaskStateAdcA:public fsm::State
{
public:
    static TaskStateAdcA&Instance();
    void entry() override;
    void exit() override;
    void react(const fsm::Event &e) override;
    void tick() override;
private:
    TaskStateAdcA();
};


#endif //XY7320_TASK_STATE_ADC_A_H
