/**
 * @file    task_state_adc_b.cpp
 * @brief   ADC Task B 状态实现
 *
 *          tick() 中调用 AdcService::Update 触发 GSM 双通道采集 + Oscilloscope 滤波。
 */

#include "task_state_adc_b.h"
#include "app_config.h"

TaskStateAdcB& TaskStateAdcB::Instance()
{
    static TaskStateAdcB instance;
    return instance;
}

TaskStateAdcB::TaskStateAdcB() : fsm::State("AdcTaskB") {}

/** @brief 进入状态：启动 ADC B 通道采集 */
void TaskStateAdcB::entry()
{
    AdcService::Instance().StartTaskB();
    LOG_Printf("TaskStateAdcB,Entry\n");
}

/** @brief 退出状态：停止 ADC，并把 Oscilloscope 重置为默认 400/450 模式
 *
 * NOTE：把 Scope 模式重置为 400/450 作为默认；
 *       如未来增加更多状态需重构此处的"重置默认值"逻辑。
 */
void TaskStateAdcB::exit()
{
    AdcService::Instance().Stop();
    AdcService::Instance().SetScopeMode(SCOPE_MODE_400_450);
    LOG_Printf("TaskStateAdcB,Exit\n");
}

/** @brief ADC Task B 不响应事件，切换由 ModeManager 统一驱动 */
void TaskStateAdcB::react(const fsm::Event& event)
{
    (void)event;
}

/** @brief 每 1ms 调用一次：触发 AdcService::Update 处理 512 点数据 */
void TaskStateAdcB::tick()
{
    AdcService::Instance().Update();
}