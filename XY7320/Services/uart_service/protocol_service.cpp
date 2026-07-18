/**
 * @file    protocol_service.cpp
 * @brief   上位机通信协议服务实现
 *
 *          负责接收流的缓存与拆包、协议包校验、
 *          升级与普通命令分发，以及应答帧编码发送。
 */

#include "protocol_service.h"
#include <string.h>
#include "app_config.h"
#include "bsp_uart_rcv.h"
#include "uart_tx_service.h"
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

    LOG_Printf("Protocol,RxChunk,%u\r\n", len);
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
            LOG_Printf("Protocol,Packet,origin=%02X,goal=%02X,model=%02X,cmd=%02X,data=%u\r\n",
                       packet.origin_port, packet.goal_port, packet.model, packet.cmd,
                       packet.data_len);
            if (ValidatePacket(packet)) {
                DispatchPacket(packet);
            } else {
                LOG_Printf("Protocol,Reject,PortOrModel\r\n");
            }
        } else if (packet.state == Protocol::unENDErr) {
            break;
        } else {
            LOG_Printf("Protocol,DecodeError,%d\r\n", packet.state);
        }
    }
}

bool ProtocolService::ValidatePacket(const Protocol::ProtocolPacket &packet)
{
    /* 正式下发方向为 PC -> 设备；旧端口分支仅用于兼容 XY7000XMAIN 协议名称。 */
    const bool currentDirection = packet.goal_port == Protocol::XY_7320 &&
                                   packet.origin_port == Protocol::XY_PC;
    const bool legacyDirection = packet.goal_port == Protocol::XY_7000XMAIN &&
                                  packet.origin_port == Protocol::XY_PC;
    if (!currentDirection && !legacyDirection) {
        return false;
    }

    if (packet.model != Protocol::unRead && packet.model != Protocol::unWrite) {
        return false;
    }

    return true;
}
void ProtocolService::DispatchPacket(const Protocol::ProtocolPacket &packet)
{
    /* F0-F2 保留为升级命令族，当前 APP 只实际处理 F0 握手。 */
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
    case WAIT_MODEL: {
        ModeManager::Instance().RequestSwitch(mode::SwitchToIdleEvent());
        LOG_Printf("Protocol,Mode,IDLE\r\n");
        const bool ackQueued = SendPacketFromPorts(Protocol::XY_7000XMAIN, Protocol::XY_PC, packet.cmd, nullptr, 0);
        LOG_Printf("Protocol,Ack,%u\r\n", ackQueued ? 1U : 0U);
        break;
    }
    case POWER_MODEL: {
        if (packet.data_len >= 2U) {
            ModeManager::Instance().SetModeParameters(packet.data[0], packet.data[1],
                                                      ModeManager::Instance().gpsModel());
        }
        ModeManager::Instance().RequestSwitch(mode::SwitchToDmrEvent());
        LOG_Printf("Protocol,Mode,DMR\r\n");
        const bool dmrAckQueued = SendPacketFromPorts(Protocol::XY_7000XMAIN, Protocol::XY_PC, packet.cmd, nullptr, 0);
        LOG_Printf("Protocol,Ack,%u\r\n", dmrAckQueued ? 1U : 0U);
        break;
    }
    case POWER_MODEL_GSM: {
        if (packet.data_len >= 2U) {
            ModeManager::Instance().SetModeParameters(packet.data[0], packet.data[1],
                                                      ModeManager::Instance().gpsModel());
        }
        ModeManager::Instance().RequestSwitch(mode::SwitchToGsmEvent());
        LOG_Printf("Protocol,Mode,GSM\r\n");
        const bool gsmAckQueued = SendPacketFromPorts(Protocol::XY_7000XMAIN, Protocol::XY_PC, packet.cmd, nullptr, 0);
        LOG_Printf("Protocol,Ack,%u\r\n", gsmAckQueued ? 1U : 0U);
        break;
    }
    case GPS_MODEL:
        if (packet.data_len >= 1U) {
            ModeManager::Instance().SetModeParameters(ModeManager::Instance().freqModel(),
                                                      ModeManager::Instance().catchModel(),
                                                      packet.data[0]);
        }
        ModeManager::Instance().RequestSwitch(mode::SwitchToGnssEvent());
        SendPacketFromPorts(Protocol::XY_7000XMAIN, Protocol::XY_PC, packet.cmd, nullptr, 0);
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

            /* 只有 ACK 已进入 USART2 控制队列，才能在队列排空后安全复位。 */
            const bool ackQueued = SendPacket(packet.cmd, nullptr, 0);

            if (should_reset && ackQueued) {
                UpdateService::Instance().RequestResetToBootloaderAfterAck();
            }
            break;
        }
        break;

    default:
        break;
    }
}

bool ProtocolService::SendPacket(uint8_t cmd, const uint8_t *data, uint8_t data_len)
{
    return SendPacketFromPorts(Protocol::XY_7320, Protocol::XY_PC, cmd, data, data_len);
}

bool ProtocolService::SendPacketFromPorts(uint8_t originPort, uint8_t goalPort,
                                          uint8_t cmd, const uint8_t *data, uint8_t data_len)
{
    Protocol::ProtocolPacket txPacket;
    memset(&txPacket, 0, sizeof(txPacket));

    Protocol::initProtocol(&txPacket);
    txPacket.origin_port = originPort;
    txPacket.goal_port = goalPort;
    txPacket.cmd = cmd;
    txPacket.data_len = data_len;

    if (data != nullptr && data_len > 0) {
        memcpy(txPacket.data, data, data_len);
    }

    const uint16_t send_len = Protocol::EncodePacket(&txPacket, m_txBuf);
    if (send_len == 0U) {
        return false;
    }

    return UartTxService::Instance().EnqueueControl(m_txBuf, send_len);
}
