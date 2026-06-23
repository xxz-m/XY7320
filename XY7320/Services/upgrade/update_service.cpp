/**
 * @file    update_service.cpp
 * @brief   升级流程编排实现
 *
 * 本模块是 Services 层的"胶水层"，负责协调：
 * - BSP 层：串口收发（BspUartRcv_*）、系统复位（BspTimOs_*）
 * - Domain 层：协议解析（VersionFrame_Parse）
 * - Services 层：版本存储（VersionStore::*）
 *
 * 典型调用流程：
 * 1. APP 启动 → Init()：初始化串口，写 A1 为当前版本
 * 2. 上位机发旧升级版本帧 → UART IDLE 中断 → BSP 层收数据、置标志
 * 3. 统一协议服务识别旧升级协议 → 调用 HandleUpgradeFrame()
 * 4. 解析 → 写 A2 → 发 ACK → 复位 → Bootloader 检测 A2 进入升级模式
 */

#include "update_service.h"
#include "bsp_uart_rcv.h"      /* BSP: 串口 DMA 接收 */
#include "bsp_tim_os.h"        /* BSP: 延时、系统复位 */
#include "bsp_config.h"        /* Common: BSP 硬件配置 */
#include "version_frame.h"     /* Domain: 版本帧协议解析 */
#include "version_store.h"     /* Services: 版本配置存储 */
#include "app_config.h"        /* Common: APP 功能配置（版本号） */

UpdateService& UpdateService::Instance()
{
    static UpdateService instance;
    return instance;
}

/**
 * 初始化升级服务
 *
 * 在 App_Main_Init() 中调用，完成：
 * 1. 初始化串口 DMA 接收（绑定 UPGRADE_HUART，分配接收缓冲）
 * 2. 启动 DMA + IDLE 中断（开始接收数据）
 * 3. 写 A1 为当前运行版本（标记为 DOWNLOADED）
 *
 * 为什么写 A1：
 * - Bootloader 需要知道"当前 APP 是什么版本"
 * - APP 每次启动都更新 A1，确保 Bootloader 拿到最新版本号
 */
void UpdateService::Init()
{
    /* 1. 初始化 BSP 串口 DMA 接收 */
    BspUartRcv_Init(&UPGRADE_HUART, m_rxBuf, sizeof(m_rxBuf));

    /* 2. 启动 DMA 接收 + IDLE 中断 */
    BspUartRcv_Start();

    /* 3. 写 A1 为当前运行版本（标记为已下载） */
    VersionStore::Instance().WriteA1(
        APP_CURRENT_VERSION,
        VERSION_FRAME_FLAG_DOWNLOADED
    );
}

bool UpdateService::HandleUpgradeFrame(const uint8_t *data, uint16_t len)
{
    return ProcessFrame(data, len);
}

/**
 * 处理一帧数据
 *
 * 流程：
 * 1. 用 Domain 层解析协议（纯计算，不碰硬件）
 * 2. 用 Services 层写 A2（通过 BSP 层写 Flash）
 * 3. 如果 flag == NEED_DOWNLOAD，回 ACK 并复位
 *
 * @param data  帧数据
 * @param len   帧长度
 * @return true 处理成功, false 解析失败
 */
bool UpdateService::ProcessFrame(const uint8_t *data, uint16_t len)
{
    /* 1. 用 Domain 层解析协议 */
    VersionFrame vf;
    if (VersionFrame_Parse(data, len, &vf) < 0) {
        return false;
    }

    /* 2. 用 Services 层写 A2（目标升级版本） */
    VersionStore::Instance().WriteA2(vf.version, vf.flag);

    /* 3. 如果是"需要下载"，回 ACK 并复位进入 Bootloader */
    if (vf.flag == VERSION_FRAME_FLAG_NEED_DOWNLOAD) {
        /* 实验：改用 HAL 阻塞发送，验证 DMA 接收模式下 HAL_Transmit 是否会返回 BUSY */
        BspUartRcv_SendAck(
            reinterpret_cast<const uint8_t *>(VERSION_FRAME_ACK),
            sizeof(VERSION_FRAME_ACK) - 1
        );

        BspTimOs_DelayMs(50);    /* 等 ACK 完全发出 */
        BspTimOs_SystemReset();
    }

    return true;
}
