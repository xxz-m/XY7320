/**
 * @file    mode_types.h
 * @brief   模式类型定义
 *          模式枚举、事件类型、具体事件类
 */

#ifndef XY7320_MODE_TYPES_H
#define XY7320_MODE_TYPES_H

#include "fsm.h"

namespace mode {

/** 模式 ID 枚举（第一阶段最小集） */
enum ModeId {
    MODE_IDLE,          ///< 空闲模式，系统默认状态
    MODE_ADC_TASK_A,    ///< ADC Task A 模式，负责 A 通道采集
    MODE_ADC_TASK_B,    ///< ADC Task B 模式，负责 B 通道采集
};

/** 事件类型枚举 */
enum EventType {
    EVT_SWITCH_TO_IDLE,         ///< 切换到空闲模式
    EVT_SWITCH_TO_ADC_TASK_A,   ///< 切换到 ADC Task A 模式
    EVT_SWITCH_TO_ADC_TASK_B,   ///< 切换到 ADC Task B 模式
};

/* ====== 具体事件类 ====== */

/** 切换到空闲模式事件 */
class SwitchToIdleEvent : public fsm::Event {
public:
    SwitchToIdleEvent() : fsm::Event(EVT_SWITCH_TO_IDLE) {}
};

/** 切换到 ADC Task A 模式事件 */
class SwitchToAdcTaskAEvent : public fsm::Event {
public:
    SwitchToAdcTaskAEvent() : fsm::Event(EVT_SWITCH_TO_ADC_TASK_A) {}
};

/** 切换到 ADC Task B 模式事件 */
class SwitchToAdcTaskBEvent : public fsm::Event {
public:
    SwitchToAdcTaskBEvent() : fsm::Event(EVT_SWITCH_TO_ADC_TASK_B) {}
};

} // namespace mode

#endif //XY7320_MODE_TYPES_H
