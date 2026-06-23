//
// Created by XYKJ on 2026/6/23.
//

#ifndef XY7320_PROTOCOL_SERVICE_H
#define XY7320_PROTOCOL_SERVICE_H
#include <stdint.h>
#include <stdbool.h>
#include "protocol_xy.h"

class ProtocolService
{
public:
    static ProtocolService& Instance();

    void Init();
    void Update();

private:
    ProtocolService() = default;

    void AppendInput(const uint8_t *data, uint16_t len);
    void ProcessStream();
    bool TryHandleLegacyUpgradeFrame(const uint8_t *data, uint16_t len);
    bool ValidatePacket(const Protocol::ProtocolPacket &packet);
    void DispatchPacket(const Protocol::ProtocolPacket &packet);

    void HandleUpgradePacket(const Protocol::ProtocolPacket &packet);
    void HandleCommandPacket(const Protocol::ProtocolPacket &packet);

    void SendPacket(uint8_t cmd, const uint8_t *data, uint8_t data_len);

private:
    static constexpr uint16_t RX_CHUNK_MAX = 256;     // 每次从 BSP 取出的原始数据段
    static constexpr uint16_t STREAM_BUF_MAX = 512;   // 流式缓存，需要能容纳半包/粘包/多包

    uint8_t m_rxChunk[RX_CHUNK_MAX];                  // 临时接收块：一次从 BSP 取出的原始字节
    uint8_t m_streamBuf[STREAM_BUF_MAX];              // 真正的协议流式缓存
    uint16_t m_streamLen = 0;                         // 当前流式缓存有效长度

    uint8_t m_txBuf[300];                             // 协议编码输出缓冲
};

#endif //XY7320_PROTOCOL_SERVICE_H
