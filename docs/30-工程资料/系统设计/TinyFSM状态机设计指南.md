---
title: TinyFSM 状态机设计指南
tags:
  - docs/design
  - state-machine
  - tinyfsm
date: 2026-06-23
status: active
---

# TinyFSM 状态机设计指南

> [!abstract] 核心思想
> **状态切换 = exit 旧状态 → 换指针 → entry 新状态**  
> **事件处理 = 交给当前状态的 react()，由它决定切不切**

## 一、TinyFSM 是什么

TinyFSM 是一个 C++ 轻量级状态机库，header-only 实现，零依赖，专为裸机/RTOS 设计。

### 核心模式：entry / exit / react

```cpp
// 每个状态是一个类，继承自 FSM 基类
class Idle : public tinyfsm::MooreMachine<Elevator, Idle> {
    void entry() override { /* 进入时做什么 */ }
    void exit()  override { /* 退出时做什么 */ }
    void react(TimerExpired const &) override { /* 响应事件 */ }
};

class Measuring : public tinyfsm::MooreMachine<Elevator, Measuring> {
    void entry() override { /* 开 ADC、切 LED */ }
    void exit()  override { /* 关 ADC */ }
    void react(ButtonPress const &) override { /* 切换状态 */ }
};
```

### 特点

- **entry / exit / react 三函数模式**：每个状态自己管进入、退出、事件响应
- **模板实现**：编译期确定转换表，零运行时开销
- **事件驱动**：外部命令 = 事件，dispatch 到当前状态的 react()
- **支持层次状态**：子状态继承父状态行为

## 二、TinyFSM 的内部机制

### 本质：一个指针 + 三个虚函数

```cpp
// TinyFSM 的核心就这个类（简化版）

template<typename FSM>
class State {
public:
    virtual void entry() {}       // 进入这个状态时做什么
    virtual void exit()  {}       // 离开这个状态时做什么
    virtual void react(Event e) {} // 收到事件时做什么
};

// FSM 引擎的核心（简化版）

template<typename InitialState>
class Fsm {
    static State* current;   // ← 整个 FSM 就这一个指针

public:
    // 初始化：指向初始状态，调用 entry()
    static void start() {
        current = &InitialState::instance();
        current->entry();
    }

    // 派发事件：交给当前状态的 react()
    static void dispatch(Event e) {
        current->react(e);
    }

    // 状态切换：由 react() 内部调用
    template<typename NextState>
    static void transit() {
        current->exit();              // 1. 退出旧状态
        current = &NextState::instance(); // 2. 切换指针
        current->entry();             // 3. 进入新状态
    }
};
```

**核心就一个 `current` 指针**。`transit()` 做三件事：exit → 换指针 → entry。

### 运行时流程

```text
启动：
  Fsm::start()
    → current = &Idle::instance()
    → current->entry()          ← 打印"进入 IDLE"，LED 闪烁

上位机发"开始测量"：
  EventQueue 收到 StartMeasure 事件
  Fsm::dispatch(StartMeasure)
    → current->react(StartMeasure)
    → 当前是 Idle，命中 Idle::react(StartMeasure)
    → 调用 transit<Measure>()
        → Idle::exit()          ← 没有重写，空
        → current = &Measure::instance()
        → Measure::entry()      ← 开 ADC，LED 常亮

上位机发"停止"：
  Fsm::dispatch(Stop)
    → current->react(Stop)
    → 当前是 Measure，命中 Measure::react(Stop)
    → 调用 transit<Idle>()
        → Measure::exit()       ← 关 ADC，LED 灭
        → current = &Idle::instance()
        → Idle::entry()         ← LED 闪烁
```

## 三、状态机解决什么问题

### 不用状态机的隐患

#### 隐患 1：切换失败怎么办？

```cpp
void HandleSwitchMode(DeviceMode target) {
    AdcService::Instance().Stop();
    BspGpio_On(&gsm_power);
    
    // GSM 模块初始化失败（模块坏了、SIM 卡没插）
    if (!GsmModule_WaitReady(2000)) {
        // 怎么办？ADC 已经关了，GSM 电源已经开了
        // 需要手动回滚
        BspGpio_Off(&gsm_power);
        AdcService::Instance().Start();  // 恢复 ADC
        g_currentMode = DeviceMode::MEASURE;  // 恢复模式
        return;
    }
    
    BspUart_SwitchTo(GSM_UART);
    LedService::Instance().SetMode(BREATHE);
    g_currentMode = DeviceMode::GSM;
}
```

**问题**：每个切换点都要手动写回滚逻辑，漏一个就是 bug。

#### 隐患 2：切换过程中又来了一个切换命令

```cpp
// 上位机连续发两个命令
// t=0ms:  发"切到 GSM"
// t=50ms: 发"切到 MEASURE"

void HandleSwitchMode(DeviceMode target) {
    // 正在切 GSM（GSM 电源已开，UART 还没切）
    // 突然收到"切到 MEASURE"
    // g_currentMode 被覆盖，但 GSM 电源还开着
    // ADC 也没恢复
    // 状态混乱
}
```

**问题**：没有"切换中"的锁，并发切换会导致状态不一致。

#### 隐患 3：加新模式时漏掉清理逻辑

```cpp
// 三个月后，加了个 CALIBRATE 模式
void HandleSwitchMode(DeviceMode target) {
    if (target == DeviceMode::CALIBRATE) {
        // 忘记关 GSM 了！
        // BspGpio_Off(&gsm_power);  ← 漏了
        AdcService::Instance().StartFast();
        LedService::Instance().SetMode(FAST_BLINK);
        g_currentMode = DeviceMode::CALIBRATE;
        
        // 结果：GSM 模块还开着，白白耗电
    }
}
```

**问题**：每个切换路径都要记得"关旧的"，漏一个就是 bug。

### 用状态机怎么解决

```cpp
// ===== 每个状态自己管自己的 entry/exit =====

class MeasureState {
public:
    void entry() {
        AdcService::Instance().Start();
        LedService::Instance().SetMode(ON);
    }
    
    void exit() {
        AdcService::Instance().Stop();  // ← 离开 MEASURE 时一定会关 ADC
    }
    
    void react(SwitchToGsmEvent) {
        transit<GsmState>();  // 自动调用 exit() → entry()
    }
};

class GsmState {
public:
    void entry() {
        BspGpio_On(&gsm_power);
        
        // 异步等待 GSM 就绪
        if (!GsmModule_WaitReady(2000)) {
            // 初始化失败 → 切到 ERROR 状态
            transit<ErrorState>();
            return;
        }
        
        BspUart_SwitchTo(GSM_UART);
        LedService::Instance().SetMode(BREATHE);
    }
    
    void exit() {
        BspGpio_Off(&gsm_power);  // ← 离开 GSM 时一定会关电源
    }
    
    void react(SwitchToMeasureEvent) {
        transit<MeasureState>();  // 自动调用 exit() → entry()
    }
};

class ErrorState {
public:
    void entry() {
        // 统一清理：关所有外设
        AdcService::Instance().Stop();
        BspGpio_Off(&gsm_power);
        LedService::Instance().SetMode(FAST_BLINK);
    }
    
    void react(ResetEvent) {
        transit<IdleState>();  // 从 ERROR 恢复
    }
};
```

**三个隐患怎么解决的**：

| 问题 | 解决方案 |
|------|---------|
| **切换失败** | 失败时切到 ERROR 状态，`ErrorState::entry()` 统一清理，不用手动回滚 |
| **并发切换** | FSM 引擎内部有锁，切换中锁住，拒绝新事件 |
| **清理逻辑遗漏** | 每个状态自己管 `exit()`，框架保证切换时一定会调用，不会漏 |

## 四、状态机 vs 任务调度器

### 相似点

```text
任务调度器：
  current_task → Task_LED → 执行一步 → return
  current_task → Task_Update → 执行一步 → return
  current_task → Task_LED → 执行一步 → return
  ...

状态机：
  current_state → Idle → 处理事件 → return
  current_state → Idle → 处理事件 → return
  current_state → Measure → 处理事件 → return
  ...
```

都是"一个指针指向当前执行单元，每周期执行一步就 return"。

### 本质区别：并行 vs 互斥

#### 任务调度器：多个任务**同时存在，轮流执行**

```text
t=0ms   Task_LED 执行一步
t=1ms   Task_Update 执行一步
t=2ms   Task_LED 执行一步
t=3ms   Task_Update 执行一步

LED 和协议接收是同时跑的，互不干扰
```

#### 状态机：多个状态**互斥，同一时刻只有一个在跑**

```text
t=0ms   当前状态 = Idle，处理事件
t=1ms   当前状态 = Idle，处理事件
t=2ms   收到"开始测量" → 切到 Measure
t=3ms   当前状态 = Measure，处理事件

Idle 和 Measure 不会同时跑，切过去之后 Idle 就"死了"
```

### 用图来看

#### 任务调度器：多个任务并行

```text
时间线 ─────────────────────────────────→

Task_LED:      [执行][执行]      [执行][执行]
Task_Update:   [执行][执行][执行][执行][执行]
Task_Adc:           [执行]      [执行][执行]
                    ↑
                 可以动态创建/销毁任务
```

#### 状态机：单个状态切换

```text
时间线 ─────────────────────────────────→

当前状态:  [Idle][Idle][Idle]→[Measure][Measure][Measure]→[Gsm][Gsm]
                                    ↑                    ↑
                                 transit()            transit()
                                 exit(Idle)           exit(Measure)
                                 entry(Measure)       entry(Gsm)
```

**Idle 和 Measure 不会同时存在**，切过去之后 Idle 的 entry/exit/react 都不再被调用。

### 它们的关系：状态机是调度器里的一个"任务"

```text
你的调度器（并行）：
  ├── Task_LED         ← 永远跑
  ├── Task_Update      ← 永远跑（协议接收）
  ├── Task_ModeManager ← 永远跑（状态机引擎）
  │     └── 内部是状态机（互斥）：
  │           [Idle] → [Measure] → [Gsm] → ...
  └── Task_Adc         ← 永远跑，但内部根据模式决定干不干活

状态机（互斥）：
  └── 当前状态 = Measure
        ├── 不调用 Idle 的任何函数
        ├── 不调用 Gsm 的任何函数
        └── 只调用 Measure 的 entry/exit/react
```

**状态机是 Task_ModeManager 内部的逻辑**，它不是调度器，它是调度器管理的众多任务之一。

### 总结对比

| | 任务调度器 | 状态机 |
|---|---|---|
| **多个执行单元** | 并行，轮流执行 | 互斥，同一时刻只有一个 |
| **切换方式** | 时间片轮转（或优先级） | 事件驱动（transit） |
| **生命周期** | 任务长期存在 | 状态切换后旧的"死了" |
| **你的项目里** | OS 调度器（os.c） | ModeManager 内部逻辑 |

**调度器管"时间维度"的并行，状态机管"功能维度"的互斥**。

## 五、在 XY7320 项目中的应用方案

### 方案选择：函数指针简化版

TinyFSM 的 C++ 模板机制精巧，但 XY7320 工程是 C/C++ 混合的。采用函数指针表模拟同样的效果：

- 不引入第三方库
- 纯 C 实现，调试方便
- 核心思想跟 TinyFSM 一样
- 跟现有的 C 风格代码（调度器、BSP）风格统一

### 代码结构

```cpp
// 状态定义
struct StateHandlers {
    void (*onEnter)();
    void (*onExit)();
    void (*onEvent)(uint8_t event);  // 每个状态自己处理事件
};

// 转换表 = TinyFSM 的 "状态类集合"
static const StateHandlers s_states[] = {
    [STATE_IDLE] = {
        .onEnter = Idle_OnEnter,
        .onExit  = Idle_OnExit,
        .onEvent = Idle_OnEvent,     // IDLE 自己处理事件
    },
    [STATE_MEASURE] = {
        .onEnter = Measure_OnEnter,
        .onExit  = Measure_OnExit,
        .onEvent = Measure_OnEvent,  // MEASURE 自己处理事件
    },
};

// FSM 引擎 = TinyFSM 的 dispatch()
void Fsm_Dispatch(uint8_t event) {
    s_states[current_state].onEvent(event);
}

// 状态切换 = TinyFSM 的 transit()
void Fsm_Transit(uint8_t next_state) {
    s_states[current_state].onExit();   // exit 旧状态
    current_state = next_state;          // 换指针
    s_states[current_state].onEnter();   // entry 新状态
}

// 每个状态的 onEvent = TinyFSM 的 react()
void Idle_OnEvent(uint8_t event) {
    switch (event) {
        case EVT_START_MEASURE:
            Fsm_Transit(STATE_MEASURE);   // 等同于 transit<Measure>()
            break;
        case EVT_ERROR:
            Fsm_Transit(STATE_ERROR);
            break;
    }
}

void Measure_OnEvent(uint8_t event) {
    switch (event) {
        case EVT_STOP:
            Fsm_Transit(STATE_IDLE);      // 等同于 transit<Idle>()
            break;
        case EVT_ERROR:
            Fsm_Transit(STATE_ERROR);
            break;
    }
}
```

### 概念映射

| TinyFSM 概念 | 简化版对应 |
|---|---|
| `class Idle : public State` | `Idle_OnEnter()` + `Idle_OnExit()` + `Idle_OnEvent()` |
| `entry()` | `Idle_OnEnter()` |
| `exit()` | `Idle_OnExit()` |
| `react(Event)` | `Idle_OnEvent(event)` 里的 switch-case |
| `transit<Measure>()` | `Fsm_Transit(STATE_MEASURE)` |
| FSM 引擎 `dispatch()` | `Fsm_Dispatch(event)` |

### 文件落点

```text
App/mode_manager/
  ├── device_mode.h          # 状态枚举 + 事件枚举（纯定义）
  ├── mode_manager.h         # ModeManager 类接口
  └── mode_manager.cpp       # 转换表 + 驱动逻辑 + 超时保护
```

### 与协议层的集成

```cpp
// protocol_service.cpp — HandleCommandPacket 里加一个 case

void ProtocolService::HandleCommandPacket(const Protocol::ProtocolPacket &packet)
{
    switch (packet.cmd) {
    case 0x02:  // 模式切换命令
        {
            uint8_t target_mode = packet.data[0];

            // 根据目标模式，生成对应事件
            DeviceEvent evt = DeviceEvent::EVT_NONE;
            switch (static_cast<DeviceMode>(target_mode)) {
                case DeviceMode::GSM:     evt = DeviceEvent::EVT_SWITCH_GSM;     break;
                case DeviceMode::MEASURE: evt = DeviceEvent::EVT_SWITCH_MEASURE; break;
                case DeviceMode::IDLE:    evt = DeviceEvent::EVT_SWITCH_IDLE;    break;
                default: break;
            }

            if (evt != DeviceEvent::EVT_NONE) {
                // 扔进 ModeManager 的事件队列
                bool accepted = ModeManager::Instance().PostEvent(evt);

                // 立刻回 ACK 或 NACK
                uint8_t result = accepted ? 0x00 : 0x01;  // 0=受理, 1=拒绝
                SendPacket(packet.cmd, &result, 1);
            }
        }
        break;
    }
}
```

**关键点**：协议层只做两件事——**转事件 + 回 ACK**，不做任何切换动作。整个过程微秒级完成，不会阻塞协议接收。

## 六、总结

### 状态机框架解决的核心问题

| 问题 | 不用状态机 | 用状态机 |
|------|-----------|---------|
| **切换失败** | 手动回滚，容易漏 | 切到 ERROR 状态，统一清理 |
| **并发切换** | 状态混乱 | 切换中锁住，拒绝新事件 |
| **清理逻辑遗漏** | 每个路径都要记得关旧的 | 框架保证 `exit()` 一定被调用 |
| **加新模式** | 要改 N 个切换路径 | 只写新状态的 entry/exit |
| **超时保护** | 手动写计时器 | 框架提供超时机制 |
| **审计转换规则** | 翻遍整个代码库 | 所有转换在一个地方 |

### 适用场景

**需要状态机**，如果：
- 切换是异步的（GSM 初始化要等几百毫秒）
- 切换可能失败（模块坏了、SIM 卡没插）
- 模式数量 > 4 个
- 需要超时保护

**不需要状态机**，如果：
- 切换都是同步的（GPIO 开关，1ms 内完成）
- 不会失败
- 模式数量 < 3 个
- 不需要超时

### XY7320 的判断

- 有 GSM 模式（异步初始化，可能失败）
- 有 9 种工作模式（从老项目继承）
- 需要超时保护（GSM 初始化超时）

**结论**：需要状态机框架。

## 参考

- [[30-工程资料/系统设计/升级模块拆分方案|升级模块拆分方案]]
- [[10-项目规范/架构规范|架构规范]]
