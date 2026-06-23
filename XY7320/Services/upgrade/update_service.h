/**
 * @file    update_service.h
 * @brief   升级流程编排
 *          管理串口 DMA 接收、版本帧解析、A2 写入与系统复位
 *
 * 调用链：
 *   Task_UpdateConfig → UpdateService::Update()
 *                          ├── BspUartRcv_*       (BSP: 串口收发)
 *                          ├── VersionFrame_Parse  (Domain: 协议解析)
 *                          └── VersionStore::*     (Services: 版本存储)
 */

#ifndef XY7320_UPDATE_SERVICE_H
#define XY7320_UPDATE_SERVICE_H

#include <cstdint>

/**
 * 升级流程编排
 *
 * 职责：
 * 1. 初始化串口 DMA 接收
 * 2. 周期轮询：检查是否有新版本帧到达
 * 3. 解析版本帧 → 写 A2 → 发 ACK → 复位
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

    /** 处理旧升级协议原始帧 */
    bool HandleUpgradeFrame(const uint8_t *data, uint16_t len);

private:
    UpdateService() = default;

    /** 接收缓冲区 */
    uint8_t m_rxBuf[64]{};

    /** 处理一帧数据 */
    bool ProcessFrame(const uint8_t *data, uint16_t len);
};

#endif /* XY7320_UPDATE_SERVICE_H */
