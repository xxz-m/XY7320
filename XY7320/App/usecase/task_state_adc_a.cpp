/**
 * @file    task_state_adc_a.cpp
 * @brief   ADC Task A 状态实现
 *
 *          tick() 中调用 AdcService::Update 触发 400/450 四通道采集 + Oscilloscope 滤波。
 */

#include "task_state_adc_a.h"
#include "app_config.h"

TaskStateAdcA& TaskStateAdcA::Instance()
{
    static TaskStateAdcA instance;
    return instance;
}

TaskStateAdcA::TaskStateAdcA() : fsm::State("AdcTaskA") {}

/** @brief 进入状态：启动 ADC A 通道采集 */
void TaskStateAdcA::entry()
{
    AdcService::Instance().StartTaskA();
    LOG_Printf("TaskStateAdcA,Entry\n");
}

/** @brief 退出状态：停止 ADC，并把 Oscilloscope 切到下一状态对应的模式
 *
 * NOTE：当前实现把 Scope 模式切到 GSM（AdcTaskA 退出后通常走 AdcTaskB 走 GSM）；
 *       如未来增加更多状态需重构此处的"预设下一状态模式"逻辑。
 */
void TaskStateAdcA::exit()
{
    AdcService::Instance().Stop();
    AdcService::Instance().SetScopeMode(SCOPE_MODE_GSM);
    LOG_Printf("TaskStateAdcA,Exit\n");
}

/** @brief ADC Task A 不响应事件，切换由 ModeManager 统一驱动 */
void TaskStateAdcA::react(const fsm::Event& event)
{
    (void)event;
}

/** @brief 每 1ms 调用一次：触发 AdcService::Update 处理 512 点数据 */
void TaskStateAdcA::tick()
{
    AdcService::Instance().Update();
}