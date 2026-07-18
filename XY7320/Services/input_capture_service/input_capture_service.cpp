/**
 * @file    input_capture_service.cpp
 * @brief   A/B 比较器输入捕获测量服务实现
 */

#include "input_capture_service.h"

#include "bsp_tim_os.h"

namespace
{
/*
 * TIM2/TIM4 当前按 1 MHz 计数，因此捕获计数值的差值可直接按 us 使用。
 * 最大周期同时限制异常边沿间隔，避免错误时间戳形成看似有效的测量结果。
 */
constexpr uint32_t INPUT_CAPTURE_MIN_PERIOD_US = 2U;
constexpr uint32_t INPUT_CAPTURE_MAX_PERIOD_US = 10000000U;
constexpr uint32_t INPUT_CAPTURE_MIN_PULSE_WIDTH_US = 1U;

/* 超过该时间没有新边沿，之前的周期和脉宽不再作为有效结果输出。 */
constexpr uint32_t INPUT_CAPTURE_TIMEOUT_MS = 1000U;
constexpr uint32_t INPUT_CAPTURE_TIMER_FREQUENCY_HZ = 1000000U;
constexpr uint32_t INPUT_CAPTURE_PERMILLE = 1000U;
}

/**
 * @brief 获取输入捕获服务单例。
 * @return 输入捕获服务单例引用。
 */
InputCaptureService& InputCaptureService::Instance()
{
    static InputCaptureService instance;
    return instance;
}

/**
 * @brief 构造输入捕获服务。
 */
InputCaptureService::InputCaptureService()
    : m_eventA{},
      m_eventB{},
      m_signalA{},
      m_signalB{},
      m_initialized(false)
{
}

/**
 * @brief 初始化输入捕获服务。
 */
void InputCaptureService::Init()
{
    m_eventA = {};
    m_eventB = {};
    m_signalA = {};
    m_signalB = {};
    m_initialized = true;

    BspTimCapture_Init(&InputCaptureService::OnCapture);
}

/**
 * @brief 启动 A/B 输入捕获。
 * @return HAL_OK 表示两路均启动成功，否则返回 HAL 错误码。
 */
HAL_StatusTypeDef InputCaptureService::Start()
{
    if (!m_initialized) {
        return HAL_ERROR;
    }

    return BspTimCapture_Start();
}

/**
 * @brief 停止 A/B 输入捕获。
 * @return HAL_OK 表示两路均停止成功，否则返回 HAL 错误码。
 */
HAL_StatusTypeDef InputCaptureService::Stop()
{
    return BspTimCapture_Stop();
}

/**
 * @brief 处理捕获事件并更新超时状态。
 */
void InputCaptureService::Update()
{
    const uint32_t nowMs = BspTimOs_GetTick();

    ProcessCapture(m_eventA, m_signalA, nowMs);
    ProcessCapture(m_eventB, m_signalB, nowMs);

    UpdateTimeout(m_signalA, nowMs);
    UpdateTimeout(m_signalB, nowMs);
}

/**
 * @brief 清除指定信号的历史测量结果。
 * @param signal 要清除的输入捕获信号。
 */
void InputCaptureService::ResetSignal(BspTimCaptureSignal_t signal)
{
    if (signal == BSP_TIM_CAPTURE_SIGNAL_A) {
        m_eventA = {};
        m_signalA = {};
    } else {
        m_eventB = {};
        m_signalB = {};
    }
}

/**
 * @brief 获取信号 A 的测量结果。
 * @return 信号 A 测量结果快照。
 */
InputCaptureResult InputCaptureService::GetSignalA() const
{
    return BuildResult(m_signalA);
}

/**
 * @brief 获取信号 B 的测量结果。
 * @return 信号 B 测量结果快照。
 */
InputCaptureResult InputCaptureService::GetSignalB() const
{
    return BuildResult(m_signalB);
}

/**
 * @brief BSP 捕获回调。
 *
 * 该函数运行在定时器中断上下文，只把原始事件写入对应 A/B 队列，
 * 不执行周期、脉宽、频率和有效性计算。
 *
 * @param signal 捕获信号 A 或 B。
 * @param edge 捕获边沿类型。
 * @param tick 定时器捕获计数值。
 */
void InputCaptureService::OnCapture(BspTimCaptureSignal_t signal,
                                     BspTimCaptureEdge_t edge,
                                     uint32_t tick)
{
    InputCaptureService::Instance().QueueCapture(signal, edge, tick);
}

/**
 * @brief 将原始捕获事件写入对应信号队列。
 *
 * A/B 队列分别由中断生产、Service 任务消费。队列满时丢弃最新事件，
 * 并累计溢出次数，使上层可以通过 invalidEdgeCount 识别数据不完整。
 *
 * @param signal 捕获信号 A 或 B。
 * @param edge 捕获边沿类型。
 * @param tick 定时器捕获计数值。
 */
void InputCaptureService::QueueCapture(BspTimCaptureSignal_t signal,
                                        BspTimCaptureEdge_t edge,
                                        uint32_t tick)
{
    /*
     * A/B 使用独立队列，保证 DMR 和 GSM 的边沿不会相互覆盖。
     * 回调运行在中断上下文，只写入事件和索引，不做测量计算。
     */
    CaptureEvent& event = (signal == BSP_TIM_CAPTURE_SIGNAL_A)
        ? m_eventA
        : m_eventB;

    /* EVENT_QUEUE_SIZE 为 2 的幂，位与等价于取模且无需执行除法。 */
    const uint8_t nextIndex = static_cast<uint8_t>(
        (event.writeIndex + 1U) & (EVENT_QUEUE_SIZE - 1U));

    if (nextIndex == event.readIndex) {
        /* 队列满时保留已有顺序，丢弃最新事件并让上层结果失效。 */
        event.overflowCount++;
        return;
    }

    event.edge[event.writeIndex] = edge;
    event.tick[event.writeIndex] = tick;
    event.writeIndex = nextIndex;
}

/**
 * @brief 消费指定信号队列中的捕获事件。
 *
 * @param event 原始捕获事件队列。
 * @param state 对应信号的测量状态。
 * @param nowMs 当前系统时间。
 */
void InputCaptureService::ProcessCapture(CaptureEvent& event,
                                         CaptureState& state,
                                         uint32_t nowMs)
{
    while (event.readIndex != event.writeIndex) {
        const uint8_t index = event.readIndex;
        const BspTimCaptureEdge_t edge = event.edge[index];
        const uint32_t tick = event.tick[index];

        /* 先移动读指针，再处理事件，保证下一次 Update() 从下一条开始。 */
        event.readIndex = static_cast<uint8_t>(
            (index + 1U) & (EVENT_QUEUE_SIZE - 1U));
        state.lastCaptureMs = nowMs;
        state.timeout = false;

        if (edge == BSP_TIM_CAPTURE_EDGE_RISING) {
            ProcessRisingEdge(state, tick, nowMs);
        } else if (edge == BSP_TIM_CAPTURE_EDGE_FALLING) {
            ProcessFallingEdge(state, tick, nowMs);
        } else {
            state.invalidEdgeCount++;
        }
    }

    /*
     * 队列溢出意味着至少有一个边沿时间戳丢失，当前周期/脉宽序列
     * 可能不再连续；先累计错误次数，后续有效边沿会重新建立测量结果。
     */
    state.invalidEdgeCount += event.overflowCount;
    event.overflowCount = 0U;
}

/**
 * @brief 处理上升沿并计算周期。
 *
 * 通过无符号减法计算差值，依赖 uint32_t 运算规则自然处理定时器回绕。
 *
 * @param state 对应信号测量状态。
 * @param tick 当前上升沿定时器计数值。
 * @param captureMs 当前系统时间。
 */
void InputCaptureService::ProcessRisingEdge(CaptureState& state,
                                            uint32_t tick,
                                            uint32_t captureMs)
{
    /*
     * 周期必须由相邻两个上升沿计算；下降沿只用于测量高电平脉宽。
     * uint32_t 无符号减法可自然处理定时器从 0xFFFFFFFF 回到 0 的情况，
     * 前提是相邻边沿间隔小于一个完整的 32 位计数周期。
     */
    if (state.hasRising) {
        const uint32_t periodUs = tick - state.lastRisingTick;

        state.periodUs = periodUs;
        state.periodValid = (periodUs >= INPUT_CAPTURE_MIN_PERIOD_US) &&
                            (periodUs <= INPUT_CAPTURE_MAX_PERIOD_US);
        if (!state.periodValid) {
            state.invalidEdgeCount++;
        }
    }

    state.lastRisingTick = tick;
    state.lastCaptureMs = captureMs;
    state.hasRising = true;
}

/**
 * @brief 处理下降沿并计算脉宽。
 *
 * @param state 对应信号测量状态。
 * @param tick 当前下降沿定时器计数值。
 * @param captureMs 当前系统时间。
 */
void InputCaptureService::ProcessFallingEdge(CaptureState& state,
                                             uint32_t tick,
                                             uint32_t captureMs)
{
    /* 没有对应上升沿时，下降沿无法组成一组完整的高电平脉宽。 */
    if (!state.hasRising) {
        state.invalidEdgeCount++;
        state.pulseWidthValid = false;
        return;
    }

    /* 与周期一样，使用无符号差值处理定时器计数器回绕。 */
    const uint32_t pulseWidthUs = tick - state.lastRisingTick;

    state.pulseWidthUs = pulseWidthUs;
    state.pulseWidthValid =
        (pulseWidthUs >= INPUT_CAPTURE_MIN_PULSE_WIDTH_US) &&
        (pulseWidthUs <= INPUT_CAPTURE_MAX_PERIOD_US);

    /* 一个周期内的高电平时间不可能大于完整周期，否则边沿配对已异常。 */
    if (state.periodValid && state.pulseWidthValid &&
        pulseWidthUs > state.periodUs) {
        state.pulseWidthValid = false;
    }

    state.lastCaptureMs = captureMs;
    if (!state.pulseWidthValid) {
        state.invalidEdgeCount++;
    }
}

/**
 * @brief 更新指定信号的超时状态。
 *
 * @param state 对应信号测量状态。
 * @param nowMs 当前系统时间。
 */
void InputCaptureService::UpdateTimeout(CaptureState& state,
                                        uint32_t nowMs)
{
    /*
     * 使用系统 Tick 判断“多久没有新边沿”，而不是使用捕获定时器计数值。
     * 超时后清除有效标志，防止上层继续使用已经过期的周期和脉宽。
     */
    if (!state.hasRising ||
        (nowMs - state.lastCaptureMs) > INPUT_CAPTURE_TIMEOUT_MS) {
        state.timeout = true;
        state.periodValid = false;
        state.pulseWidthValid = false;
    }
}

/**
 * @brief 将内部状态转换为对外测量结果。
 * @param state 对应信号测量状态。
 * @return 测量结果快照。
 */
InputCaptureResult InputCaptureService::BuildResult(
    const CaptureState& state) const
{
    InputCaptureResult result{};

    result.periodUs = state.periodUs;
    result.pulseWidthUs = state.pulseWidthUs;
    result.lastCaptureMs = state.lastCaptureMs;
    result.invalidEdgeCount = state.invalidEdgeCount;
    result.periodValid = state.periodValid;
    result.pulseWidthValid = state.pulseWidthValid;
    result.timeout = state.timeout;
    result.valid = state.periodValid && state.pulseWidthValid &&
                   !state.timeout;

    /* 只有周期和脉宽都有效时才换算频率、占空比，避免除零或输出旧数据。 */
    if (result.valid) {
        result.frequencyHz = INPUT_CAPTURE_TIMER_FREQUENCY_HZ /
                             result.periodUs;
        result.dutyPermille = static_cast<uint16_t>(
            (static_cast<uint64_t>(result.pulseWidthUs) *
             INPUT_CAPTURE_PERMILLE) /
            result.periodUs);
    }

    return result;
}
