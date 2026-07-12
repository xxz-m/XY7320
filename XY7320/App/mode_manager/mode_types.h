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
    MODE_DMR,            ///< DMR 模式，负责 PA15 周期、脉宽和功率采集
    MODE_GSM,            ///< GSM 模式，负责 PA2 周期、脉宽和功率采集
    MODE_GNSS,
};

/** 事件类型枚举 */
enum EventType {
    EVT_SWITCH_TO_IDLE,         ///< 切换到空闲模式
    EVT_SWITCH_TO_DMR,          ///< 切换到 DMR 模式
    EVT_SWITCH_TO_GSM,          ///< 切换到 GSM 模式
    EVT_SWITCH_TO_GNSS,
};

/* ====== 具体事件类 ====== */

/** 切换到空闲模式事件 */
class SwitchToIdleEvent : public fsm::Event {
public:
    SwitchToIdleEvent() : fsm::Event(EVT_SWITCH_TO_IDLE) {}
};

/** 切换到 DMR 模式事件 */
class SwitchToDmrEvent : public fsm::Event {
public:
    SwitchToDmrEvent() : fsm::Event(EVT_SWITCH_TO_DMR) {}
};

/** 切换到 GSM 模式事件 */
class SwitchToGsmEvent : public fsm::Event {
public:
    SwitchToGsmEvent() : fsm::Event(EVT_SWITCH_TO_GSM) {}
};
class  SwitchToGnssEvent : public fsm::Event
{
public:
     SwitchToGnssEvent() : fsm::Event(EVT_SWITCH_TO_GNSS) {}
};
} // namespace mode

#endif //XY7320_MODE_TYPES_H
