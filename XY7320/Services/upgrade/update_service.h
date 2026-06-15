//
// Created by XYKJ on 2026/6/15.
//

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
 * 调用链：
 *   Task_UpdateConfig → Update_service::Update()
 *                          ├── Bsp_UartRcv_*      (BSP: 串口收发)
 *                          ├── VersionFrame_Parse  (Domain: 协议解析)
 *                          └── Version_store::*    (Services: 版本存储)
 */

class update_service
{
public:
    static update_service& Instance();

    /**
     * 初始化升级服务
     * - 初始化串口 DMA 接收
     * - 写 A1 为当前运行版本
     */
    void Init();

    /**
     * 周期调用（在 Task_UpdateConfig 中每 10ms 调用一次）
     * 检查是否有新帧，有则解析并处理
     */
    void Update();
private:
    update_service() = default;

    /** 接收缓冲区 */
    uint8_t m_rxBuf[64]{};
    /** 处理一帧数据 */
    bool ProcessFrame(const uint8_t *data, uint16_t len);
};


#endif //XY7320_UPDATE_SERVICE_H
