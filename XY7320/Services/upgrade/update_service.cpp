//
// Created by XYKJ on 2026/6/15.
//

/**
 * 升级流程编排实现
 * 
 * 本模块是 Services 层的"胶水层"，负责协调：
 * - BSP 层：串口收发（Bsp_UartRcv_*）
 * - Domain 层：协议解析（VersionFrame_Parse）
 * - Services 层：版本存储（version_store::*）
 * 
 * 典型调用流程：
 * 1. APP 启动 → Init()：初始化串口，写 A1 为当前版本
 * 2. 上位机发版本帧 → UART IDLE 中断 → BSP 层收数据、置标志
 * 3. 主循环轮询 → Update()：检查标志 → 取帧 → 解析 → 写 A2 → 发 ACK → 复位
 * 4. Bootloader 检测 A2 → 进入升级模式
 */

#include "update_service.h"
#include "bsp_uart_rcv.h"      // BSP: 串口 DMA 接收
#include "version_frame.h"     // Domain: 版本帧协议解析
#include "version_store.h"     // Services: 版本配置存储
#include "app_config.h"        // Common: APP 全局配置（版本号）
#include "usart.h"             // CubeMX 生成：huart2 声明

/**
 * 获取单例
 */
update_service& update_service::Instance()
{
    static update_service instance;
    return instance;
}

/**
 * 初始化升级服务
 * 
 * 在 App_Main_Init() 中调用，完成：
 * 1. 初始化串口 DMA 接收（绑定 huart2，分配接收缓冲）
 * 2. 启动 DMA + IDLE 中断（开始接收数据）
 * 3. 写 A1 为当前运行版本（标记为 DOWNLOADED）
 * 
 * 为什么写 A1：
 * - Bootloader 需要知道"当前 APP 是什么版本"
 * - APP 每次启动都更新 A1，确保 Bootloader 拿到最新版本号
 */
void update_service::Init()
{
    // 1. 初始化 BSP 串口 DMA 接收
    //    参数：串口句柄、接收缓冲区、缓冲区大小
    Bsp_UartRcv_Init(&huart2, m_rxBuf, sizeof(m_rxBuf));

    // 2. 启动 DMA 接收 + IDLE 中断
    //    启动后，DMA 自动收数据，串口空闲时触发 IDLE 中断
    Bsp_UartRcv_Start();

    // 3. 写 A1 为当前运行版本（标记为已下载）
    //    这样 Bootloader 就知道"当前 APP 版本是 APP_CURRENT_VERSION"
    version_store::Instance().WriteA1(
        APP_CURRENT_VERSION,
        VERSION_FRAME_FLAG_DOWNLOADED
    );
}

/**
 * 周期轮询（每 10ms 调用一次）
 * 
 * 在 Task_UpdateConfig 中调用：
 *   extern "C" void Task_UpdateConfig(void *arg) {
 *       update_service::Instance().Update();
 *       OS_DelayMs(10);
 *   }
 * 
 * 流程：
 * 1. 检查是否有新帧（BSP 层在中断中置标志）
 * 2. 取帧数据（从 BSP 层拷贝到本地）
 * 3. 处理帧（解析 → 写 A2 → 发 ACK → 复位）
 */
void update_service::Update()
{
    // 没有新帧，直接返回（不阻塞）
    if (!Bsp_UartRcv_IsFrameReady()) {
        return;
    }

    // 取帧：从 BSP 层拷贝到本地缓冲
    uint8_t frame[64];
    uint16_t len = Bsp_UartRcv_GetFrameLength();
    Bsp_UartRcv_CopyFrame(frame);
    Bsp_UartRcv_ClearFlag();  // 清标志，允许接收下一帧

    // 处理帧
    ProcessFrame(frame, len);
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
bool update_service::ProcessFrame(const uint8_t *data, uint16_t len)
{
    // 1. 用 Domain 层解析协议
    //    输入：原始字节流
    //    输出：VersionFrame 结构体（version + flag）
    VersionFrame vf;
    if (VersionFrame_Parse(data, len, &vf) < 0) {
        return false;  // 解析失败（帧头/帧尾/格式错误）
    }

    // 2. 用 Services 层写 A2
    //    A2 表示"要升级到的目标版本"
    //    Bootloader 启动时会检查 A2，决定是否进入升级模式
    version_store::Instance().WriteA2(vf.version, vf.flag);

    // 3. 如果是"需要下载"，回 ACK 并复位进入 Bootloader
    if (vf.flag == VERSION_FRAME_FLAG_NEED_DOWNLOAD) {
        // 用 BSP 层的直接发送函数（绕过 HAL 状态机）
        // 内部会停止 DMA、禁用中断，然后用寄存器直接发送
        Bsp_UartRcv_SendAckDirect(
            reinterpret_cast<const uint8_t *>(VERSION_FRAME_ACK),
            sizeof(VERSION_FRAME_ACK) - 1
        );
        
        HAL_Delay(50);  // 等 ACK 完全发出
        NVIC_SystemReset();
    }

    return true;
}
