//
// Created by XYKJ on 2026/6/23.
//

#include "protocol_service.h"
#include <string.h>
#include "bsp_uart_rcv.h"
#include "update_service.h"
#include "mode_manager.h"
ProtocolService& ProtocolService::Instance()
{
    static ProtocolService instance;
    return instance;
}

void ProtocolService::Init()
{
    m_streamLen = 0;
    memset(m_rxChunk, 0, sizeof(m_rxChunk));
    memset(m_streamBuf, 0, sizeof(m_streamBuf));
    memset(m_txBuf, 0, sizeof(m_txBuf));
}
//取数据
void ProtocolService::Update()
{
    ///** 检查是否有完整帧到达 */
    if (!BspUartRcv_IsFrameReady()) {
        return;
    }
    /** 获取当前帧长度 */
    uint16_t len = BspUartRcv_GetFrameLength();
    if (len == 0 || len > sizeof(m_rxChunk)) {
        /* 单次突发帧不应超过单 chunk；超长帧属异常（协议帧最长 256B），
         * 直接清标志丢弃，避免后续解析时越界覆盖 m_streamBuf。 */
        BspUartRcv_ClearFlag();
        return;
    }
    //目标缓冲区（调用方确保容量 >= s_frame_len）
    BspUartRcv_CopyFrame(m_rxChunk);
    /** 清除帧就绪标志，必须在 CopyFrame() 取走数据后调用 */
    BspUartRcv_ClearFlag();

    //拼缓存
    AppendInput(m_rxChunk, len);
    //拆包
    ProcessStream();
}
// 这里就是“半包/粘包”问题的核心入口
// 每次新来的原始字节不是立刻当一包处理，而是先追加
// 如果缓存放不下，第一版建议简单处理：
// 直接清流式缓存重新同步
// 这样做的原因是：协议流已经错乱时，保守恢复比继续乱解更稳
void ProtocolService::AppendInput(const uint8_t *data, uint16_t len)
{
    if (data == nullptr || len == 0) {
        return;
    }

    if (m_streamLen + len > sizeof(m_streamBuf)) {
        m_streamLen = 0;
        memset(m_streamBuf, 0, sizeof(m_streamBuf));
        return;
    }

    memcpy(&m_streamBuf[m_streamLen], data, len);
    m_streamLen += len;
}

void ProtocolService::ProcessStream()
{
    Protocol::ProtocolPacket packet;

    while (m_streamLen > 0) {
        memset(&packet, 0, sizeof(packet));

        uint16_t handled_len = Protocol::DecodeBuffer(m_streamBuf, m_streamLen, &packet);
        if (handled_len == 0) {
            break;
        }

        if (handled_len >= m_streamLen) {
            m_streamLen = 0;
        } else {
            uint16_t remaining = m_streamLen - handled_len;
            memmove(m_streamBuf, &m_streamBuf[handled_len], remaining);
            m_streamLen = remaining;
        }

        if (packet.state == Protocol::unNoError) {
            if (ValidatePacket(packet)) {
                DispatchPacket(packet);
            }
        } else if (packet.state == Protocol::unENDErr) {
            break;
        }
    }
}

//这些属于统一协议入口的职责
bool ProtocolService::ValidatePacket(const Protocol::ProtocolPacket &packet)
{
    if (packet.goal_port != Protocol::XY_7320) {
        return false;
    }

    if (packet.origin_port != Protocol::XY_PC) {
        return false;
    }

    if (packet.model != Protocol::unRead && packet.model != Protocol::unWrite) {
        return false;
    }

    return true;
}
//统一分发：升级和普通命令先放一个服务里
void ProtocolService::DispatchPacket(const Protocol::ProtocolPacket &packet)
{
    switch (packet.cmd) {
    case 0xF0:
    case 0xF1:
    case 0xF2:
        HandleUpgradePacket(packet);
        break;

    default:
        HandleCommandPacket(packet);
        break;
    }
}

void ProtocolService::HandleCommandPacket(const Protocol::ProtocolPacket &packet)
{
    switch (packet.cmd) {
    case 0x10:
        ModeManager::Instance().RequestSwitch(mode::SwitchToIdleEvent());
        SendPacket(packet.cmd, nullptr, 0);
        break;
    case 0x11:
        ModeManager::Instance().RequestSwitch(mode::SwitchToAdcTaskAEvent());
        SendPacket(packet.cmd, nullptr, 0);
        break;
    case 0x12:
        ModeManager::Instance().RequestSwitch(mode::SwitchToAdcTaskBEvent());
        SendPacket(packet.cmd, nullptr, 0);
        break;
    default:
        break;
    }
}

void ProtocolService::HandleUpgradePacket(const Protocol::ProtocolPacket &packet)
{
    switch (packet.cmd) {
    case 0xF0:
        {
            bool should_reset = false;
            if (!UpdateService::Instance().HandleProtocolUpgradeRequest(packet.data,
                                                                        packet.data_len,
                                                                        should_reset)) {
                break;
            }

            SendPacket(packet.cmd, nullptr, 0);

            if (should_reset) {
                UpdateService::Instance().ResetToBootloaderAfterAck();
            }
            break;
        }
        break;

    default:
        break;
    }
}

void ProtocolService::SendPacket(uint8_t cmd, const uint8_t *data, uint8_t data_len)
{
    Protocol::ProtocolPacket txPacket;
    memset(&txPacket, 0, sizeof(txPacket));

    Protocol::initProtocol(&txPacket);

    txPacket.origin_port = Protocol::XY_7320;
    txPacket.goal_port = Protocol::XY_PC;
    txPacket.cmd = cmd;
    txPacket.data_len = data_len;

    if (data != nullptr && data_len > 0) {
        memcpy(txPacket.data, data, data_len);
    }

    uint16_t send_len = Protocol::EncodePacket(&txPacket, m_txBuf);
    if (send_len > 0) {
        BspUartRcv_SendAck(m_txBuf, send_len);
    }
}
