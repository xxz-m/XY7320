/**
 * @file    uart_tx_service.h
 * @brief   USART2 异步发送服务（控制 ACK FIFO + Mode 业务 ping-pong）
 *
 * 责任：
 *  - 把 UART2 正常发送收口为唯一所有者，业务模块不再直接调用 HAL/BSP 发送接口；
 *  - 控制 ACK 保序（FIFO），Mode 业务帧允许新值覆盖（ping-pong）；
 *  - 提交数据立即返回，不等物理发送完成；
 *  - DMA 正在发送的缓冲区在完成前保持不变（发送期间不允许重填）。
 *
 * 数据流：
 *      ProtocolService::SendPacket  -- EnqueueControl -->  +-----+
 *      App/usecase task_state_*    -- PublishModeData -->  | FIFO|\
 *                                                           +-----+ \  Update()
 *                                                                   ----> HAL_UART_Transmit_DMA(&huart2)
 *                                                                   <---- HAL_UART_TxCpltCallback
 *
 * 与 ModeManager 协作：
 *  - ModeManager::RequestSwitch 在 entry 之后回调本服务的 OnModeChanged，
 *    清理旧模式 Pending 槽位；Sending 槽位由 TxCplt 中的 generation 校验兜底丢弃。
 *
 * @note    单例（Instance）。固定内存（无 malloc/new/动态容器）。
 * @note    C++17。UartTxService 由 Core/Src/stm32f4xx_it.c 的 ISR 回调置标志，
 *          由 Task_UpdateConfig::Task_UpdateConfig 周期驱动 Update()。
 */

#ifndef XY7320_UART_TX_SERVICE_H
#define XY7320_UART_TX_SERVICE_H

#include <stdint.h>
#include "configurations.h"

#ifdef __cplusplus
#include "mode_types.h"
extern "C" {
#endif

/**
 * @brief UART2 TX 统计计数器（便于排查 backlog / 丢弃 / busy）
 */
struct UartTxStats {
    uint32_t sentFrames;            ///< 已成功通过 DMA 发出的帧数（控制 + 业务合计）
    uint32_t controlEnqueued;       ///< EnqueueControl 成功入队次数
    uint32_t controlOverflow;       ///< EnqueueControl 因 FIFO 满被丢弃次数
    uint32_t controlQueueHighWater; ///< 控制 FIFO 历史最高水位
    uint32_t modePublished;         ///< PublishModeData 成功入队次数
    uint32_t modePendingOverwrite;  ///< 业务 Pending 槽位被新数据覆盖次数
    uint32_t staleModeDrop;         ///< 模式/会话不匹配的旧 Pending 被丢弃次数
    uint32_t dmaStartBusy;          ///< HAL_UART_Transmit_DMA 返回 BUSY，下轮重试次数
    uint32_t dmaStartError;         ///< HAL_UART_Transmit_DMA 返回 ERROR 次数
    uint32_t txError;               ///< HAL_UART_ErrorCallback 触发次数
    uint32_t buildOverflow;         ///< BuildModeFrame 因 payload 过大丢弃次数
};

#ifdef __cplusplus
}

class UartTxService {
public:
    /// @brief 获取单例
    static UartTxService& Instance();

    /// @brief 初始化：清零 FIFO、ping-pong、统计计数与状态标志
    void Init();

    /// @brief 周期驱动：由 Task_UpdateConfig 每 10ms 调用一次
    void Update();

    /**
     * @brief 控制 ACK 入队（用于协议 ACK）
     * @param data   已按 protocol_xy 编码后的完整帧缓冲（不含额外处理）
     * @param length 帧长度（字节），必须 <= CONTROL_FRAME_MAX
     * @return true 入队成功；false FIFO 满，丢弃
     */
    bool EnqueueControl(const uint8_t* data, uint16_t length);

    /**
     * @brief 模式数据上行（按 mode+generation 路由到 ping-pong 槽位）
     *
     * @param cmd        上行命令码，使用 UPLINK_CMD_*_MEAS
     * @param mode       业务模式枚举（mode::ModeId）
     * @param generation ModeManager 当前会话编号
     * @param seq        业务单调递增序号
     * @param payload    业务负载指针（已序列化的字节流）
     * @param payloadLen 业务负载长度，必须 <= UPLINK_PAYLOAD_MAX
     * @return true 成功入队或覆盖 Pending；false 异常（payload 过大或参数错）
     */
    bool PublishModeData(uint8_t cmd,
                         mode::ModeId mode,
                         uint32_t generation,
                         uint16_t seq,
                         const uint8_t* payload,
                         uint8_t payloadLen);

    /**
     * @brief 模式切换回调：清旧模式 Pending 槽位
     *
     * @param oldMode        切换前的模式
     * @param oldGeneration  切换前的会话编号
     * @param newMode        切换后的模式
     * @param newGeneration  切换后的会话编号
     */
    void OnModeChanged(mode::ModeId oldMode, uint32_t oldGeneration,
                       mode::ModeId newMode, uint32_t newGeneration);

    /**
     * @brief ISR 回调：USART2 TX DMA 完成（HAL_UART_TxCpltCallback 调用）
     *
     * 仅置 m_txCompleteFlag，不做任何协议/编码/排队操作。
     */
    void OnTxCompleteFromIsr();

    /**
     * @brief ISR 回调：USART2 错误（HAL_UART_ErrorCallback 调用）
     *
     * 仅置 m_txErrorFlag 与累加错误码，不做任何协议/编码/排队操作。
     */
    void OnTxErrorFromIsr(uint32_t halErrorCode);

    /**
     * @brief ISR 上下文查询当前是否存在活动 TX DMA
     * @return true 已成功启动一帧且尚未收到完成/错误回调
     */
    bool HasActiveTxFromIsr() const { return m_txBusy; }

    /**
     * @brief 查询发送服务是否已排空
     * @return true 当前 DMA 空闲且控制 FIFO、Mode 缓冲均为空
     *
     * @note 这是非阻塞查询。升级流程应在周期 Update 中轮询，不能在
     *       协作式调度任务内部自旋等待。
     */
    bool IsIdle() const;

    /**
     * @brief 丢弃所有尚未发送的 Mode Pending 数据
     *
     * 升级 ACK 排空前调用，避免升级复位等待普通业务数据；正在发送的
     * 一帧不会被中断，完成后控制 ACK 会优先发送。
     */
    void DiscardPendingModeData();

    /**
     * @brief 暂停或恢复 Mode 数据发布与调度
     *
     * 升级 ACK 排空期间暂停，防止业务任务持续产生新 Pending 帧。
     * 暂停时会同步丢弃尚未发送的 Mode 数据，不中断 Sending。
     */
    void SetModeDataSuspended(bool suspended);

    /// @brief 获取调试统计
    const UartTxStats& GetStats() const { return m_stats; }

private:
    UartTxService() = default;

    enum class TxBufferState : uint8_t { Free, Pending, Sending };
    enum class ActiveSource : uint8_t { None, Control, Mode };

    static constexpr uint16_t CONTROL_FRAME_MAX = 300U;  ///< 控制帧单帧上限（与 protocol_service m_txBuf 对齐）
    static constexpr uint16_t MODE_FRAME_MAX = 300U;     ///< Mode 完整协议帧上限（含帧头/CRC/转义）
    static constexpr uint8_t  CONTROL_QUEUE_SIZE = 4U;   ///< 控制 FIFO 槽位数
    static constexpr uint8_t  MODE_PINGPONG_NUM  = 2U;   ///< 模式业务 ping-pong 槽位数
    static constexpr uint8_t  ACTIVE_SLOT_NONE   = 0xFFU;///< 当前 DMA 在发控制帧或空闲的标识
    static constexpr uint8_t  FRAME_TX_TIMEOUT_MS = 50U; ///< 同帧重试周期上限参考值

    struct ControlFrame {
        uint16_t length;
        uint8_t  data[CONTROL_FRAME_MAX];
    };

    struct ControlQueue {
        ControlFrame frames[CONTROL_QUEUE_SIZE];
        uint8_t readIndex;
        uint8_t writeIndex;
        uint8_t count;
    };

    struct ModeTxBuffer {
        TxBufferState state;
        mode::ModeId  mode;
        uint32_t      generation;
        uint16_t      seq;
        uint16_t      length;
        uint8_t       data[MODE_FRAME_MAX];
    };

    bool StartNextFrame();
    void HandleTxCompletion();
    void HandleTxError(uint32_t halErrorCode);
    void ResetSendingBuffer();
    uint16_t BuildModeFrame(uint8_t* dst, uint16_t dstSize,
                            uint8_t cmd,
                            mode::ModeId mode, uint32_t generation,
                            uint16_t seq,
                            const uint8_t* payload, uint8_t payloadLen);

    /// @brief 选一个用于 Publish 的 slot：Free > Pending；不覆盖 Sending
    /// @return slot 索引；0xFF 表示无可用槽位
    uint8_t AcquireModeSlotForPublish();

    /// @brief 选一个用于发送的 slot：Pending 且 generation 与当前 ModeManager 一致
    /// @param currentMode        当前模式
    /// @param currentGeneration  当前会话编号
    /// @return slot 索引；0xFF 表示无可发送槽位
    uint8_t PickModeSlotForSend(mode::ModeId currentMode, uint32_t currentGeneration);

    /// @brief 当前 ModeManager 的 mode+generation（轻量读，避免循环依赖）
    void QueryCurrentMode(mode::ModeId& outMode, uint32_t& outGeneration) const;

    ControlQueue  m_control;
    ModeTxBuffer  m_modeBuf[MODE_PINGPONG_NUM];
    ActiveSource  m_activeSource;
    uint8_t       m_activeSlot;
    volatile bool m_txCompleteFlag;
    volatile bool m_txErrorFlag;
    uint32_t      m_lastErrorCode;

    volatile bool m_txBusy;
    bool m_modeDataSuspended;
    bool m_initialized;

    UartTxStats m_stats;
};

#endif /* __cplusplus */

#endif /* XY7320_UART_TX_SERVICE_H */