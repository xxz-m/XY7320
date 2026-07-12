/**
 * @file    task_state_gsm.h
 * @brief   GSM 任务状态
 *
 * GSM 状态先等待 PA2/TIM5_CH3 比较器周期和脉宽有效，
 * 再启动 GSM 正向/反向功率 ADC 采集。
 */

#ifndef XY7320_TASK_STATE_GSM_H
#define XY7320_TASK_STATE_GSM_H

#include "fsm.h"

/**
 * @brief GSM 业务状态。
 */
class TaskStateGsm : public fsm::State
{
public:
    /** @brief 获取 GSM 状态单例。 */
    static TaskStateGsm& Instance();

    /** @brief 进入 GSM 状态，清除旧周期并等待新测量。 */
    void entry() override;

    /** @brief 退出 GSM 状态，停止 ADC 采集。 */
    void exit() override;

    /** @brief GSM 状态不直接处理事件。 */
    void react(const fsm::Event& event) override;

    /** @brief 周期处理比较器测量和 GSM 功率采集流程。 */
    void tick() override;

private:
    TaskStateGsm();

    bool m_powerCaptureStarted;
};

#endif /* XY7320_TASK_STATE_GSM_H */
