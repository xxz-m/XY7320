/**
 * @file    update_service.h
 * @brief   升级流程编排
 *          管理主协议升级握手、A1/A2 写入与系统复位
 */

#ifndef XY7320_UPDATE_SERVICE_H
#define XY7320_UPDATE_SERVICE_H

#include <cstdint>

/**
 * 升级流程编排
 *
 * 职责：
 * 1. 初始化升级相关状态
 * 2. 处理主协议升级握手载荷
 * 3. 写 A2 → 发主协议 ACK → 复位
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class UpdateService
{
public:
    static UpdateService& Instance();

    /**
     * 初始化升级服务
     * - 初始化串口 DMA 接收
     * - 写 A1 为当前运行版本
     */
    void Init();

    /**
     * 处理主协议升级握手载荷
     * data[0..11] : 12位 ASCII 版本号 yyyyMMddHHmm
     * data[12]    : flag
     */
    bool HandleProtocolUpgradeRequest(const uint8_t *data, uint8_t len, bool &should_reset);

    /**
     * @brief 请求在升级 ACK 排空后复位
     *
     * 非阻塞接口：只记录请求并丢弃普通 Mode Pending 数据。
     */
    void RequestResetToBootloaderAfterAck();

    /**
     * @brief 周期驱动 ACK 排空与复位状态机
     *
     * UartTxService 完全空闲后才释放 USART2 并复位。超时只取消复位，
     * 避免在 ACK 未完整发送时进入 Bootloader。
     */
    void Update();

    /** @brief 查询是否正在等待 ACK 排空。 */
    bool IsResetPending() const { return m_resetPending; }

private:
    UpdateService() = default;

    /** 解析 12 位 ASCII 版本号 */
    bool ParseAsciiVersion(const uint8_t *data, uint64_t &version);

    /** 升级串口接收缓冲区 */
    uint8_t m_rxBuf[64]{};

    bool m_resetPending = false;
    uint32_t m_resetRequestTick = 0U;
};

#endif /* XY7320_UPDATE_SERVICE_H */
