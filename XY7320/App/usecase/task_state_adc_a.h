/**
 * @file    task_state_adc_a.h
 * @brief   ADC Task A 状态
 *          负责 ADC A 通道的采集业务
 */

#ifndef XY7320_TASK_STATE_ADC_A_H
#define XY7320_TASK_STATE_ADC_A_H

#include "fsm.h"
#include "adc_service.h"
/**
 * ADC Task A 状态
 *
 * 进入后启动 ADC A 通道采集，退出时停止。
 * tick() 中执行周期性采集逻辑。
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class TaskStateAdcA : public fsm::State {
public:
    static TaskStateAdcA& Instance();

    void entry() override;
    void exit() override;
    void react(const fsm::Event &event) override;
    void tick() override;

private:
    TaskStateAdcA();
};

#endif //XY7320_TASK_STATE_ADC_A_H
