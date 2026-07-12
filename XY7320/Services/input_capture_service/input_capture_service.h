/**
 * @file    input_capture_service.h
 * @brief   A/B 比较器输入捕获测量服务
 *
 * Service 层接收 BSP 上报的原始边沿事件，计算周期、脉宽、频率和占空比，
 * 并负责测量结果有效性及信号超时判断。
 */

#ifndef XY7320_INPUT_CAPTURE_SERVICE_H
#define XY7320_INPUT_CAPTURE_SERVICE_H

#include <stdint.h>

#include "bsp_tim_capture.h"

/**
 * @brief 输入捕获测量结果。
 */
struct InputCaptureResult
{
    uint32_t periodUs; /**< 最近一次有效周期，单位 us。 */
    uint32_t pulseWidthUs; /**< 最近一次脉宽，单位 us。 */
    uint32_t frequencyHz; /**< 根据周期计算的频率，单位 Hz。 */
    uint16_t dutyPermille; /**< 占空比，单位 1/1000。 */
    uint32_t lastCaptureMs; /**< 最近一次捕获对应的系统时间。 */
    uint32_t invalidEdgeCount; /**< 无效边沿或无效测量累计次数。 */
    bool periodValid; /**< 周期是否通过范围检查。 */
    bool pulseWidthValid; /**< 脉宽是否通过范围检查。 */
    bool valid; /**< 当前周期和脉宽结果是否整体有效。 */
    bool timeout; /**< 是否超过信号超时时间。 */
};

/**
 * @brief A/B 比较器输入捕获测量服务。
 *
 * 捕获中断只保存 A/B 各自最新的原始事件；完整测量和结果判断由
 * Update() 在 OS 任务上下文中执行，避免在中断中进行除法和业务计算。
 */
class InputCaptureService
{
public:
    /**
     * @brief 获取服务单例。
     * @return 输入捕获服务单例引用。
     */
    static InputCaptureService& Instance();

    /**
     * @brief 初始化服务并注册 BSP 捕获回调。
     */
    void Init();

    /**
     * @brief 启动 A/B 输入捕获。
     * @return HAL_OK 表示两路均启动成功，否则返回 HAL 错误码。
     */
    HAL_StatusTypeDef Start();

    /**
     * @brief 停止 A/B 输入捕获。
     * @return HAL_OK 表示两路均停止成功，否则返回 HAL 错误码。
     */
    HAL_StatusTypeDef Stop();

    /**
     * @brief 处理待处理捕获事件并更新超时状态。
     *
     * 应由周期性 App/OS 任务调用，建议周期不大于 10 ms。
     */
    void Update();

    /**
     * @brief 清除指定信号的历史测量结果。
     * @param signal 要清除的输入捕获信号。
     */
    void ResetSignal(BspTimCaptureSignal_t signal);

    /**
     * @brief 获取信号 A 的最新测量结果。
     * @return 信号 A 测量结果快照。
     */
    InputCaptureResult GetSignalA() const;

    /**
     * @brief 获取信号 B 的最新测量结果。
     * @return 信号 B 测量结果快照。
     */
    InputCaptureResult GetSignalB() const;

private:
    static constexpr uint8_t EVENT_QUEUE_SIZE = 8U;

    struct CaptureEvent
    {
        volatile BspTimCaptureEdge_t edge[EVENT_QUEUE_SIZE];
        volatile uint32_t tick[EVENT_QUEUE_SIZE];
        volatile uint8_t readIndex;
        volatile uint8_t writeIndex;
        volatile uint32_t overflowCount;
    };

    struct CaptureState
    {
        uint32_t lastRisingTick;
        uint32_t lastCaptureMs;
        uint32_t periodUs;
        uint32_t pulseWidthUs;
        uint32_t invalidEdgeCount;
        bool hasRising;
        bool periodValid;
        bool pulseWidthValid;
        bool timeout;
    };

    InputCaptureService();
    InputCaptureService(const InputCaptureService&) = delete;
    InputCaptureService& operator=(const InputCaptureService&) = delete;

    static void OnCapture(BspTimCaptureSignal_t signal,
                          BspTimCaptureEdge_t edge,
                          uint32_t tick);

    void QueueCapture(BspTimCaptureSignal_t signal,
                      BspTimCaptureEdge_t edge,
                      uint32_t tick);
    void ProcessCapture(CaptureEvent& event,
                        CaptureState& state,
                        uint32_t nowMs);
    void ProcessRisingEdge(CaptureState& state,
                           uint32_t tick,
                           uint32_t captureMs);
    void ProcessFallingEdge(CaptureState& state,
                            uint32_t tick,
                            uint32_t captureMs);
    void UpdateTimeout(CaptureState& state, uint32_t nowMs);
    InputCaptureResult BuildResult(const CaptureState& state) const;

    CaptureEvent m_eventA;
    CaptureEvent m_eventB;
    CaptureState m_signalA;
    CaptureState m_signalB;
    bool m_initialized;
};

#endif /* XY7320_INPUT_CAPTURE_SERVICE_H */
