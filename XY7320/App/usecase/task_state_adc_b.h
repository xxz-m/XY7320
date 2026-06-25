/**
 * @file    task_state_adc_b.h
 * @brief   ADC Task B 状态
 *          负责 ADC B 通道的采集业务
 */

#ifndef XY7320_TASK_STATE_ADC_B_H
#define XY7320_TASK_STATE_ADC_B_H

#include "fsm.h"

/**
 * ADC Task B 状态
 *
 * 进入后启动 ADC B 通道采集，退出时停止。
 * tick() 中执行周期性采集逻辑。
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class TaskStateAdcB : public fsm::State {
public:
    static TaskStateAdcB& Instance();

    void entry() override;
    void exit() override;
    void react(const fsm::Event &event) override;
    void tick() override;

private:
    TaskStateAdcB();
};

#endif //XY7320_TASK_STATE_ADC_B_H
