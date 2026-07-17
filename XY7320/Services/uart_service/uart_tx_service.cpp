/**
 * @file    uart_tx_service.cpp
 * @brief   USART2 异步发送服务实现
 *
 * 状态机说明：
 *  - 控制 FIFO（容量 CONTROL_QUEUE_SIZE）：严格保序；
 *  - Mode ping-pong（容量 MODE_PINGPONG_NUM）：允许新值覆盖 Pending，不覆盖 Sending；
 *  - 发送优先级：控制 FIFO 优先；空闲时才取 Mode 槽；
 *  - ISR 只写 volatile bool；所有状态机切换在任务上下文（Update）完成。
 *
 * 与 ModeManager 协作：
 *  - OnModeChanged 清理旧模式 Pending；Sending 由 generation 校验兜底丢弃；
 *  - PublishModeData 与 OnModeChanged 都在 Update 调用前的任务上下文执行；
 *    临界区仅保护 FIFO 索引与 slot 状态字段，memcpy 在临界区外。
 *
 * 与 HAL 协作：
 *  - 唯一调用 HAL_UART_Transmit_DMA(&huart2, ...) 的地方；
 *  - HAL_UART_TxCpltCallback/HAL_UART_ErrorCallback 由 stm32f4xx_it.c 转发到本服务。
 */

#include "uart_tx_service.h"
#include "mode_manager.h"
#include "usart.h"
#include "protocol_xy.h"

#include <string.h>

extern "C" {
#include <cmsis_compiler.h>  /* __disable_irq / __enable_irq */
}

UartTxService& UartTxService::Instance()
{
    static UartTxService instance;
    return instance;
}

void UartTxService::Init()
{
    /* 控制 FIFO */
    m_control.readIndex = 0U;
    m_control.writeIndex = 0U;
    m_control.count = 0U;
    for (uint8_t i = 0; i < CONTROL_QUEUE_SIZE; ++i) {
        m_control.frames[i].length = 0U;
        memset(m_control.frames[i].data, 0, sizeof(m_control.frames[i].data));
    }

    /* Mode ping-pong */
    for (uint8_t i = 0; i < MODE_PINGPONG_NUM; ++i) {
        m_modeBuf[i].state = TxBufferState::Free;
        m_modeBuf[i].mode = mode::MODE_IDLE;
        m_modeBuf[i].generation = 0U;
        m_modeBuf[i].seq = 0U;
        m_modeBuf[i].length = 0U;
        memset(m_modeBuf[i].data, 0, sizeof(m_modeBuf[i].data));
    }

    m_activeSource = ActiveSource::None;
    m_activeSlot = ACTIVE_SLOT_NONE;
    m_txCompleteFlag = false;
    m_txErrorFlag = false;
    m_lastErrorCode = 0U;
    m_txBusy = false;
    m_modeDataSuspended = false;
    m_initialized = true;

    m_stats = {};
}

void UartTxService::Update()
{
    if (!m_initialized) {
        return;
    }

    /* 1. 处理 ISR 投递的错误（最高优先级，避免错误状态继续占用总线） */
    if (m_txErrorFlag) {
        HandleTxError(m_lastErrorCode);
    }

    /* 2. 处理 ISR 投递的完成 */
    if (m_txCompleteFlag) {
        HandleTxCompletion();
    }

    /* 3. 已在发则返回 */
    if (m_txBusy) {
        return;
    }

    /* 4. 控制 ACK 优先（FIFO 保序） */
    if (m_control.count > 0U) {
        (void)StartNextFrame();
        return;
    }

    /* 5. 升级排空期间禁止启动或接收新的 Mode 帧。 */
    if (m_modeDataSuspended) {
        return;
    }

    /* 6. 业务 Mode 帧（按当前 mode+generation 选 Pending 槽） */
    mode::ModeId curMode = mode::MODE_IDLE;
    uint32_t curGen = 0U;
    QueryCurrentMode(curMode, curGen);

    uint8_t slot = PickModeSlotForSend(curMode, curGen);
    if (slot != ACTIVE_SLOT_NONE) {
        m_activeSlot = slot;
        m_modeBuf[slot].state = TxBufferState::Sending;
        HAL_StatusTypeDef st = HAL_UART_Transmit_DMA(&huart2,
                                                     m_modeBuf[slot].data,
                                                     m_modeBuf[slot].length);
        if (st == HAL_OK) {
            m_activeSource = ActiveSource::Mode;
            m_txBusy = true;
        } else if (st == HAL_BUSY) {
            /* 上一帧刚刚完成但 m_txBusy 还未清，或 HAL 状态机短暂未释放；
             * 把槽位退回 Pending，下一轮重试 */
            m_modeBuf[slot].state = TxBufferState::Pending;
            m_activeSlot = ACTIVE_SLOT_NONE;
            m_stats.dmaStartBusy++;
        } else {
            m_stats.dmaStartError++;
            m_modeBuf[slot].state = TxBufferState::Free;
            m_activeSlot = ACTIVE_SLOT_NONE;
            m_txErrorFlag = true;
            m_lastErrorCode = static_cast<uint32_t>(st);
        }
    }
}

bool UartTxService::EnqueueControl(const uint8_t* data, uint16_t length)
{
    if (!m_initialized) {
        return false;
    }
    if ((data == nullptr) || (length == 0U) || (length > CONTROL_FRAME_MAX)) {
        return false;
    }

    bool ok = false;
    /*
     * 数据复制完成后才能增加 count。否则 Update 可能看到半填充槽位并立即
     * 启动 DMA。控制帧最大 300 字节，短临界区换取明确的发布原子性。
     */
    __disable_irq();
    if (m_control.count < CONTROL_QUEUE_SIZE) {
        ControlFrame& frame = m_control.frames[m_control.writeIndex];
        memcpy(frame.data, data, length);
        frame.length = length;
        m_control.writeIndex = (m_control.writeIndex + 1U) % CONTROL_QUEUE_SIZE;
        m_control.count++;
        if (m_control.count > m_stats.controlQueueHighWater) {
            m_stats.controlQueueHighWater = m_control.count;
        }
        m_stats.controlEnqueued++;
        ok = true;
    } else {
        m_stats.controlOverflow++;
    }
    __enable_irq();
    return ok;
}

bool UartTxService::PublishModeData(uint8_t cmd,
                                    mode::ModeId mode,
                                    uint32_t generation,
                                    uint16_t seq,
                                    const uint8_t* payload,
                                    uint8_t payloadLen)
{
    if (!m_initialized || m_modeDataSuspended) {
        return false;
    }
    if ((payload == nullptr) || (payloadLen > UPLINK_PAYLOAD_MAX)) {
        m_stats.buildOverflow++;
        return false;
    }

    uint8_t encoded[MODE_FRAME_MAX];
    const uint16_t built = BuildModeFrame(encoded,
                                          sizeof(encoded),
                                          cmd,
                                          mode,
                                          generation,
                                          seq,
                                          payload,
                                          payloadLen);
    if (built == 0U) {
        m_stats.buildOverflow++;
        return false;
    }

    uint8_t slot = ACTIVE_SLOT_NONE;
    bool needOverwrite = false;

    /*
     * 先在局部缓冲完成编码，再一次性发布到槽位。Pending 只表示完整帧，
     * Update 不可能在 BuildModeFrame 尚未结束时启动 DMA。
     */
    __disable_irq();
    slot = AcquireModeSlotForPublish();
    if (slot != ACTIVE_SLOT_NONE) {
        needOverwrite = (m_modeBuf[slot].state == TxBufferState::Pending);
        memcpy(m_modeBuf[slot].data, encoded, built);
        m_modeBuf[slot].length = built;
        m_modeBuf[slot].mode = mode;
        m_modeBuf[slot].generation = generation;
        m_modeBuf[slot].seq = seq;
        m_modeBuf[slot].state = TxBufferState::Pending;
        if (needOverwrite) {
            m_stats.modePendingOverwrite++;
        }
        m_stats.modePublished++;
    }
    __enable_irq();

    if (slot == ACTIVE_SLOT_NONE) {
        /* 两个槽位均处于 Sending/保留状态时，本次最新值无法发布。 */
        m_stats.staleModeDrop++;
        return false;
    }
    return true;
}

void UartTxService::OnModeChanged(mode::ModeId oldMode, uint32_t oldGeneration,
                                  mode::ModeId newMode, uint32_t newGeneration)
{
    if (!m_initialized) {
        return;
    }
    (void)newMode;
    (void)newGeneration;

    /* 清旧模式 Pending。Sending 槽位由 TxCplt 中的 generation 校验兜底丢弃。 */
    __disable_irq();
    for (uint8_t i = 0; i < MODE_PINGPONG_NUM; ++i) {
        if (m_modeBuf[i].state == TxBufferState::Pending &&
            m_modeBuf[i].mode == oldMode &&
            m_modeBuf[i].generation == oldGeneration) {
            m_modeBuf[i].state = TxBufferState::Free;
            m_stats.staleModeDrop++;
        }
    }
    __enable_irq();
}

void UartTxService::OnTxCompleteFromIsr()
{
    /* ISR 仅置标志 */
    m_txCompleteFlag = true;
}

void UartTxService::OnTxErrorFromIsr(uint32_t halErrorCode)
{
    m_txErrorFlag = true;
    m_lastErrorCode = halErrorCode;
    m_stats.txError++;
}

bool UartTxService::IsIdle() const
{
    if (!m_initialized || m_txBusy || m_control.count != 0U) {
        return false;
    }

    for (uint8_t i = 0U; i < MODE_PINGPONG_NUM; ++i) {
        if (m_modeBuf[i].state != TxBufferState::Free) {
            return false;
        }
    }
    return true;
}

void UartTxService::DiscardPendingModeData()
{
    if (!m_initialized) {
        return;
    }

    __disable_irq();
    for (uint8_t i = 0U; i < MODE_PINGPONG_NUM; ++i) {
        if (m_modeBuf[i].state == TxBufferState::Pending) {
            m_modeBuf[i].state = TxBufferState::Free;
            m_modeBuf[i].length = 0U;
            m_stats.staleModeDrop++;
        }
    }
    __enable_irq();
}

void UartTxService::SetModeDataSuspended(bool suspended)
{
    if (!m_initialized) {
        return;
    }

    m_modeDataSuspended = suspended;
    if (suspended) {
        DiscardPendingModeData();
    }
}

bool UartTxService::StartNextFrame()
{
    /*
     * 控制 FIFO 的 readIndex 在 DMA 完成前不推进。DMA 始终读取服务对象
     * 内的固定槽位，直到 HAL_UART_TxCpltCallback 后才释放该槽。
     */
    if (m_control.count == 0U) {
        return false;
    }

    ControlFrame& frame = m_control.frames[m_control.readIndex];
    if (frame.length == 0U || frame.length > CONTROL_FRAME_MAX) {
        m_stats.dmaStartError++;
        return false;
    }

    HAL_StatusTypeDef st = HAL_UART_Transmit_DMA(&huart2, frame.data, frame.length);
    if (st == HAL_OK) {
        m_activeSource = ActiveSource::Control;
        m_activeSlot = ACTIVE_SLOT_NONE;
        m_txBusy = true;
        return true;
    }
    if (st == HAL_BUSY) {
        m_stats.dmaStartBusy++;
        return false;
    }

    m_stats.dmaStartError++;
    m_txErrorFlag = true;
    m_lastErrorCode = static_cast<uint32_t>(st);
    return false;
}

void UartTxService::HandleTxCompletion()
{
    if (!m_txCompleteFlag) {
        return;
    }
    m_txCompleteFlag = false;

    if (m_activeSource == ActiveSource::Control) {
        /* 控制帧直到 DMA 完成后才真正出队，保证槽位在发送期间不变。 */
        __disable_irq();
        if (m_control.count > 0U) {
            m_control.frames[m_control.readIndex].length = 0U;
            m_control.readIndex = (m_control.readIndex + 1U) % CONTROL_QUEUE_SIZE;
            m_control.count--;
        }
        __enable_irq();
    } else if (m_activeSource == ActiveSource::Mode &&
               m_activeSlot != ACTIVE_SLOT_NONE &&
               m_activeSlot < MODE_PINGPONG_NUM) {
        ModeTxBuffer& buf = m_modeBuf[m_activeSlot];
        mode::ModeId curMode = mode::MODE_IDLE;
        uint32_t curGen = 0U;
        QueryCurrentMode(curMode, curGen);
        if (buf.mode != curMode || buf.generation != curGen) {
            m_stats.staleModeDrop++;
        }
        buf.state = TxBufferState::Free;
        buf.mode = mode::MODE_IDLE;
        buf.generation = 0U;
        buf.seq = 0U;
        buf.length = 0U;
    }

    m_activeSource = ActiveSource::None;
    m_activeSlot = ACTIVE_SLOT_NONE;
    m_txBusy = false;
    m_stats.sentFrames++;
}

void UartTxService::HandleTxError(uint32_t halErrorCode)
{
    (void)halErrorCode;
    if (!m_txErrorFlag) {
        return;
    }
    m_txErrorFlag = false;
    ResetSendingBuffer();
    m_txBusy = false;
}

void UartTxService::ResetSendingBuffer()
{
    if (m_activeSource == ActiveSource::Mode &&
        m_activeSlot != ACTIVE_SLOT_NONE &&
        m_activeSlot < MODE_PINGPONG_NUM) {
        ModeTxBuffer& buf = m_modeBuf[m_activeSlot];
        buf.state = TxBufferState::Free;
        buf.mode = mode::MODE_IDLE;
        buf.generation = 0U;
        buf.seq = 0U;
        buf.length = 0U;
    }

    /* 控制帧在错误时不推进 FIFO，下一轮 Update 会重试当前 readIndex。 */
    m_activeSource = ActiveSource::None;
    m_activeSlot = ACTIVE_SLOT_NONE;
}

uint16_t UartTxService::BuildModeFrame(uint8_t* dst, uint16_t dstSize,
                                       uint8_t cmd,
                                       mode::ModeId mode, uint32_t generation,
                                       uint16_t seq,
                                       const uint8_t* payload, uint8_t payloadLen)
{
    const uint16_t businessLen = static_cast<uint16_t>(UPLINK_HEADER_LEN) +
                                 static_cast<uint16_t>(payloadLen) +
                                 static_cast<uint16_t>(UPLINK_FIELD_RESERVED_LEN);
    if (dst == nullptr || dstSize < MODE_FRAME_MAX ||
        businessLen > MAX_PROTOCOL_LEN) {
        return 0U;
    }

    Protocol::ProtocolPacket packet{};
    Protocol::initProtocol(&packet);
    packet.origin_port = Protocol::XY_7000XMAIN;
    packet.goal_port = Protocol::XY_PC;
    packet.model = Protocol::unWrite;
    packet.cmd = cmd;

    uint16_t idx = 0U;
    if (cmd == UPLINK_CMD_DMR_MEAS || cmd == UPLINK_CMD_GSM_MEAS ||
        cmd == UPLINK_CMD_GNSS_MEAS) {
        if (payloadLen > MAX_PROTOCOL_LEN) {
            return 0U;
        }
        memcpy(packet.data, payload, payloadLen);
        packet.data_len = payloadLen;
    } else {
        packet.data[idx++] = static_cast<uint8_t>(mode);
        packet.data[idx++] = static_cast<uint8_t>((generation >> 8) & 0xFFU);
        packet.data[idx++] = static_cast<uint8_t>(generation & 0xFFU);
        packet.data[idx++] = static_cast<uint8_t>((seq >> 8) & 0xFFU);
        packet.data[idx++] = static_cast<uint8_t>(seq & 0xFFU);
        packet.data[idx++] = payloadLen;
        if (payloadLen > 0U) {
            memcpy(&packet.data[idx], payload, payloadLen);
            idx = static_cast<uint16_t>(idx + payloadLen);
        }
        packet.data[idx++] = 0U;
        packet.data[idx++] = 0U;
        packet.data_len = static_cast<uint8_t>(idx);
    }

    return Protocol::EncodePacket(&packet, dst);
}

uint8_t UartTxService::AcquireModeSlotForPublish()
{
    /* 优先 Free，其次 Pending；不覆盖 Sending。
     * 注意：本函数只在临界区内调用（由 PublishModeData 包裹）。 */
    uint8_t firstFree = ACTIVE_SLOT_NONE;
    uint8_t firstPending = ACTIVE_SLOT_NONE;
    for (uint8_t i = 0; i < MODE_PINGPONG_NUM; ++i) {
        if (m_modeBuf[i].state == TxBufferState::Free) {
            firstFree = i;
            break;
        }
        if ((firstPending == ACTIVE_SLOT_NONE) &&
            (m_modeBuf[i].state == TxBufferState::Pending)) {
            firstPending = i;
        }
    }
    return (firstFree != ACTIVE_SLOT_NONE) ? firstFree : firstPending;
}

uint8_t UartTxService::PickModeSlotForSend(mode::ModeId currentMode, uint32_t currentGeneration)
{
    /* 选择 Pending 且 mode/generation 与当前一致的 slot；
     * 不一致则置 Free 并计数，避免旧会话数据漏到新模式。
     * 注意：本函数只在 Update 任务上下文调用。 */
    uint8_t firstPending = ACTIVE_SLOT_NONE;
    for (uint8_t i = 0; i < MODE_PINGPONG_NUM; ++i) {
        if (m_modeBuf[i].state == TxBufferState::Pending) {
            if (m_modeBuf[i].mode != currentMode ||
                m_modeBuf[i].generation != currentGeneration) {
                m_modeBuf[i].state = TxBufferState::Free;
                m_modeBuf[i].mode = mode::MODE_IDLE;
                m_modeBuf[i].generation = 0U;
                m_modeBuf[i].seq = 0U;
                m_modeBuf[i].length = 0U;
                m_stats.staleModeDrop++;
                continue;
            }
            if (firstPending == ACTIVE_SLOT_NONE) {
                firstPending = i;
            }
        }
    }
    return firstPending;
}

void UartTxService::QueryCurrentMode(mode::ModeId& outMode, uint32_t& outGeneration) const
{
    /* 通过 ModeManager 单例读取当前 mode 与 generation，
     * 与 ModeManager 的回调 OnModeChanged 配合保证发送会话一致。 */
    outMode = ModeManager::Instance().currentMode();
    outGeneration = ModeManager::Instance().currentGeneration();
}