/**
* @file    mode_types.h
 * @brief   XY7320 模式枚举与切换事件定义
 */
#ifndef XY7320_MODE_TYPES_H
#define XY7320_MODE_TYPES_H
#include "fsm.h"
namespace mode
{
    /* ====== 模式枚举（第一阶段最小集） ====== */
    enum ModeId {
        MODE_IDLE      = 0,
        MODE_ADC_TASK_A = 1,
        MODE_ADC_TASK_B = 2,
    };
    /* ====== 事件类型 ====== */
    enum EventType {
        EVT_SWITCH_TO_IDLE      = 0,
        EVT_SWITCH_TO_ADC_TASK_A = 1,
        EVT_SWITCH_TO_ADC_TASK_B = 2,
    };

    /* ====== 具体事件类 ====== */
    class SwitchToIdleEvent : public fsm::Event {
    public:
        SwitchToIdleEvent() : fsm::Event(EVT_SWITCH_TO_IDLE) {}
    };
    class SwitchToAdcTaskAEvent : public fsm::Event {
    public:
        SwitchToAdcTaskAEvent() : fsm::Event(EVT_SWITCH_TO_ADC_TASK_A) {}
    };

    class SwitchToAdcTaskBEvent : public fsm::Event {
    public:
        SwitchToAdcTaskBEvent() : fsm::Event(EVT_SWITCH_TO_ADC_TASK_B) {}
    };
}//namespace mode
#endif //XY7320_MODE_TYPES_H
