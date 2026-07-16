/**
 * @file    task_state_dmr.h
 * @brief   DMR 任务状态
 *
 * DMR 状态先等待 PA15/TIM2_CH1 比较器周期和脉宽有效，
 * 再启动 DMR 正向/反向功率 ADC 采集。
 */

#ifndef XY7320_TASK_STATE_DMR_H
#define XY7320_TASK_STATE_DMR_H

#include <stdint.h>
#include "fsm.h"

/**
 * @brief DMR 业务状态。
 */
class TaskStateDmr : public fsm::State
{
public:
    /** @brief 获取 DMR 状态单例。 */
    static TaskStateDmr& Instance();

    /** @brief 进入 DMR 状态，清除旧周期并等待新测量。 */
    void entry() override;

    /** @brief 退出 DMR 状态，停止 ADC 采集。 */
    void exit() override;

    /** @brief DMR 状态不直接处理事件。 */
    void react(const fsm::Event& event) override;

    /** @brief 周期处理比较器测量和 DMR 功率采集流程。 */
    void tick() override;

    /** @brief 获取下一个上行序号（单调递增，溢出回卷） */
    uint16_t nextUploadSeq();

private:
    TaskStateDmr();

    bool     m_powerCaptureStarted;
    uint32_t m_lastUploadMs;     ///< 上次 PublishModeData 的 tick 时间
    uint16_t m_uploadSeq;        ///< 上行序号，溢出回卷
};

#endif /* XY7320_TASK_STATE_DMR_H */
