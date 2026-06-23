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

    /** 主协议 ACK 发出后执行复位，进入 BootLoader 判定流程 */
    void ResetToBootloaderAfterAck();

private:
    UpdateService() = default;

    /** 解析 12 位 ASCII 版本号 */
    bool ParseAsciiVersion(const uint8_t *data, uint64_t &version);

    /** 升级串口接收缓冲区 */
    uint8_t m_rxBuf[64]{};
};

#endif /* XY7320_UPDATE_SERVICE_H */
