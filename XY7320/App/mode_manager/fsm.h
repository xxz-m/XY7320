/**
* @file    fsm.h
 * @brief   轻量级 C++ 状态机框架（TinyFSM 风格）
 *          提供 State 基类和 Event 基类，业务状态继承 State 实现 entry/exit/react/tick
 */
#ifndef XY7320_FSM_H
#define XY7320_FSM_H
#include <cstdint>
namespace fsm {
    /* ====== 事件基类 ====== */
    class Event {
    public:
        explicit Event(uint8_t type) : type_(type) {}
        virtual ~Event() = default;
        uint8_t type() const { return type_; }
    private:
        uint8_t type_;
    };
    /* ====== 状态基类 ====== */
    class State {
    public:
        explicit State(const char *name) : name_(name) {}
        virtual ~State() = default;

        const char *name() const { return name_; }

        virtual void entry()  {}                          // 进入时一次性初始化
        virtual void exit()   {}                          // 退出时一次性清理
        virtual void react(const Event &e) { (void)e; }   // 响应事件
        virtual void tick()   {}                          // 周期业务逻辑（调度器驱动）

    private:
        const char *name_;
    };
} // namespace fsm
#endif //XY7320_FSM_H
