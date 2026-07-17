/**
 * @file    protocol_service.h
 * @brief   上位机 ↔ 下位机协议服务
 *
 *          责任：
 *          - 周期性从 BSP UART 接收层取出原始字节，组装成完整协议帧
 *          - 调用 Protocol::DecodeBuffer 解析为 ProtocolPacket
 *          - 校验端口 / 读写模式
 *          - 分发到 UpdateService（升级帧）或 ModeManager（命令帧）
 *          - 提供 SendPacket 主动出帧（出帧方向端口固定为 XY_PC）
 *
 *          数据流：
 *              Task_UpdateConfig (10ms) → Update()
 *                ├── BspUartRcv_IsFrameReady / CopyFrame
 *                ├── AppendInput → 拼流 / 流满重同步
 *                ├── ProcessStream → DecodeBuffer → ValidatePacket → DispatchPacket
 *                └── HandleUpgradePacket / HandleCommandPacket
 *
 * @note    单例，通过 Instance() 获取。
 * @note    协议帧格式由 protocol_xy.h 定义。
 */

#ifndef XY7320_PROTOCOL_SERVICE_H
#define XY7320_PROTOCOL_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol_xy.h"

class ProtocolService
{
public:
    /**
     * @brief 获取单例
     */
    static ProtocolService& Instance();

    /**
     * @brief 初始化：清空所有缓冲
     *
     * 在 OS 启动前由 App_Main::App_Main_Init 调用。
     */
    void Init();

    /**
     * @brief 周期轮询入口
     *
     * 由 App/usecase/task_update.cpp 的 Task_UpdateConfig 每 10ms 驱动一次。
     * 流程：
     *  1. 检查 BspUartRcv 是否有完整帧
     *  2. AppendInput 拼到流式缓冲
     *  3. ProcessStream 循环拆帧
     *  4. 分发到 HandleUpgradePacket / HandleCommandPacket
     */
    void Update();

private:
    ProtocolService() = default;

    /**
     * @brief 拼流：把一段原始字节追加到流式缓冲
     *
     * @param data  原始字节
     * @param len   长度
     *
     * @note  流式缓冲放不下时**清空重同步**（放弃当前不完整数据，重新对齐），
     *        因为协议帧头是 0x10 0x02，重同步策略简单可靠。
     */
    void AppendInput(const uint8_t *data, uint16_t len);

    /**
     * @brief 拆包循环：从流式缓冲中识别完整帧并分发
     */
    void ProcessStream();

    /**
     * @brief 校验包合法性
     *
     * 过滤条件：
     *  - goal_port == XY_7320（本设备）
     *  - origin_port == XY_PC（上位机）
     *  - model ∈ {unRead, unWrite}
     *
     * @return true 合法；false 非法（丢弃）
     */
    bool ValidatePacket(const Protocol::ProtocolPacket &packet);

    /**
     * @brief 分发到具体处理器
     *
     *  - cmd == 0xF0 → HandleUpgradePacket
     *  - 其他        → HandleCommandPacket
     */
    void DispatchPacket(const Protocol::ProtocolPacket &packet);

    /**
     * @brief 升级帧处理
     *
     * 当前仅处理 0xF0（写 A2 槽位 + 应答 + 复位进入 Bootloader）；
     * 0xF1 / 0xF2 留待后续扩展。
     */
    void HandleUpgradePacket(const Protocol::ProtocolPacket &packet);

    /**
     * @brief 命令帧处理
     *
     * 已实现命令：
     *  - 0x10：ModeManager 切到 Idle
     *  - 0x11：ModeManager 切到 AdcTaskA
     *  - 0x12：ModeManager 切到 AdcTaskB
     */
    void HandleCommandPacket(const Protocol::ProtocolPacket &packet);

    /**
     * @brief 出帧：编码 + 发送
     *
     * @param cmd       命令码
     * @param data      数据负载（可为 NULL）
     * @param data_len  数据长度
     *
     * @return true 完整帧已进入控制 FIFO；false 编码失败或 FIFO 已满
     * @note 出帧方向端口固定为 XY_PC（origin=本机、goal=PC）。
     */
    bool SendPacket(uint8_t cmd, const uint8_t *data, uint8_t data_len);
    bool SendPacketFromPorts(uint8_t originPort, uint8_t goalPort,
                             uint8_t cmd, const uint8_t *data, uint8_t data_len);

private:
    static constexpr uint16_t RX_CHUNK_MAX = 256;     ///< 每次从 BSP 取出的原始数据段
    static constexpr uint16_t STREAM_BUF_MAX = 512;   ///< 流式缓存，需要能容纳半包/粘包/多包

    uint8_t m_rxChunk[RX_CHUNK_MAX];                  ///< 临时接收块：一次从 BSP 取出的原始字节
    uint8_t m_streamBuf[STREAM_BUF_MAX];              ///< 真正的协议流式缓冲
    uint16_t m_streamLen = 0;                         ///< 当前流式缓冲有效长度

    uint8_t m_txBuf[300];                             ///< 协议编码输出缓冲
};

#endif /* XY7320_PROTOCOL_SERVICE_H */