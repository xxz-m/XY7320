/**
 * @file    protocol_service.cpp
 * @brief   上位机通信协议服务实现
 *
 *          负责接收流的缓存与拆包、协议包校验、
 *          升级与普通命令分发，以及应答帧编码发送。
 */

#include "protocol_service.h"
#include <string.h>
#include "bsp_uart_rcv.h"
#include "update_service.h"
#include "mode_manager.h"
#include "configurations.h"

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
void ProtocolService::Update()
{
    BspUartRcv_t *upgradeRcv = BspUartRcv_GetUpgrade();
    if (!BspUartRcv_IsFrameReady(upgradeRcv)) {
        return;
    }

    uint16_t len = BspUartRcv_GetFrameLength(upgradeRcv);
    if (len == 0 || len > sizeof(m_rxChunk)) {
        /* 单次突发帧不应超过单 chunk；超长帧属异常（协议帧最长 256B），
         * 直接清标志丢弃，避免后续解析时越界覆盖 m_streamBuf。 */
        BspUartRcv_ClearFlag(upgradeRcv);
        return;
    }
    BspUartRcv_CopyFrame(upgradeRcv, m_rxChunk);

    /* 必须先复制再清标志，否则接收层可能复用当前帧缓冲。 */
    BspUartRcv_ClearFlag(upgradeRcv);

    AppendInput(m_rxChunk, len);
    ProcessStream();
}

void ProtocolService::AppendInput(const uint8_t *data, uint16_t len)
{
    if (data == nullptr || len == 0) {
        return;
    }

    if (m_streamLen + len > sizeof(m_streamBuf)) {
        /* 流已无法完整保留时放弃旧数据，依靠固定帧头重新同步。 */
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
    case WAIT_MODEL:
        ModeManager::Instance().RequestSwitch(mode::SwitchToIdleEvent());
        SendPacket(packet.cmd, nullptr, 0);
        break;
    case POWER_MODEL:
        ModeManager::Instance().RequestSwitch(mode::SwitchToDmrEvent());
        SendPacket(packet.cmd, nullptr, 0);
        break;
    case POWER_MODEL_GSM:
        ModeManager::Instance().RequestSwitch(mode::SwitchToGsmEvent());
        SendPacket(packet.cmd, nullptr, 0);
        break;
    case GPS_MODEL:
        ModeManager::Instance().RequestSwitch(mode::SwitchToGnssEvent());
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
        BspUartRcv_SendAck(BspUartRcv_GetUpgrade(), m_txBuf, send_len);
    }
}
