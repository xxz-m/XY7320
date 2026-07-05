/**
 * @file    task_state_adc_b.h
 * @brief   ADC Task B 状态
 *
 *          负责 ADC B 通道（CH4~CH5）的采集业务 + Oscilloscope GSM 滤波。
 */

#ifndef XY7320_TASK_STATE_ADC_B_H
#define XY7320_TASK_STATE_ADC_B_H

#include "fsm.h"
#include "adc_service.h"

/**
 * @brief ADC Task B 状态
 *
 * 进入后启动 ADC B 通道采集，退出时停止。
 * tick() 中执行周期性采集逻辑。
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class TaskStateAdcB : public fsm::State
{
public:
    /** @brief 获取单例 */
    static TaskStateAdcB& Instance();

    /** @brief 进入：调用 AdcService::StartTaskB */
    void entry() override;

    /** @brief 退出：调用 AdcService::Stop + SetScopeMode(400/450) */
    void exit() override;

    /**
     * @brief 响应事件
     * @param event  fsm 事件
     *
     * ADC Task B 不响应事件，切换由 ModeManager 统一驱动。
     */
    void react(const fsm::Event& event) override;

    /** @brief 周期回调：调用 AdcService::Update（每 1ms 一次） */
    void tick() override;

private:
    TaskStateAdcB();
};

#endif /* XY7320_TASK_STATE_ADC_B_H */