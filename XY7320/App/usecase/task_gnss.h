/**
 * @file    task_gnss.h
 * @brief   GNSS 接收解析任务入口
 */

#ifndef XY7320_TASK_GNSS_H
#define XY7320_TASK_GNSS_H
#include "fsm.h"

/**
 * @brief GNSS FSM 状态
 *
 * 进入该状态后，由 ModeManager 的周期 Tick 驱动 GNSS 服务；
 * GNSS 串口的硬件接收由 GnssService 管理，本类只负责状态生命周期编排。
 */
class TaskstateGnss : public fsm::State
{
public:
    /** @brief 获取 GNSS 状态单例。 */
    static TaskstateGnss& Instance();

    /** @brief 进入 GNSS 状态，启动 GNSS DMA+IDLE 接收。 */
    void entry() override;

    /** @brief 退出 GNSS 状态，停止 GNSS DMA+IDLE 接收。 */
    void exit() override;

    /** @brief 响应状态事件；当前由 ModeManager 集中处理切换。 */
    void react(const fsm::Event& event) override;

    /** @brief 周期驱动 GnssService::Update() 处理接收数据。 */
    void tick() override;

private:
    /** @brief 构造 GNSS 状态对象。 */
    TaskstateGnss();
};
#endif /* XY7320_TASK_GNSS_H */
