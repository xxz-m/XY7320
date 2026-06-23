/**
 * @file    update_service.cpp
 * @brief   升级流程编排实现
 *
 * 本模块负责：
 * - APP 启动时写 A1（当前版本）
 * - 处理主协议升级握手载荷
 * - 写 A2（目标升级版本）
 * - 主协议 ACK 发出后复位，让 BootLoader 按原逻辑接管
 */

#include "update_service.h"
#include "bsp_tim_os.h"        /* BSP: 延时、系统复位 */
#include "bsp_config.h"        /* Common: BSP 硬件配置 */
#include "version_store.h"     /* Services: 版本配置存储 */
#include "app_config.h"        /* Common: APP 功能配置（版本号） */
#include <cstring>

namespace {
constexpr uint8_t PROTOCOL_UPGRADE_REQUEST_LEN = 13U;
}

UpdateService& UpdateService::Instance()
{
    static UpdateService instance;
    return instance;
}

/**
 * 初始化升级服务
 *
 * 在 App_Main_Init() 中调用，完成：
 * 1. 预留升级串口接收缓冲
 * 2. 写 A1 为当前运行版本（标记为 DOWNLOADED）
 *
 * 为什么写 A1：
 * - Bootloader 需要知道"当前 APP 是什么版本"
 * - APP 每次启动都更新 A1，确保 Bootloader 拿到最新版本号
 */
void UpdateService::Init()
{
    /* 1. 清空本地缓冲（保留成员，避免后续接口调整引起布局变化） */
    memset(m_rxBuf, 0, sizeof(m_rxBuf));

    /* 2. 写 A1 为当前运行版本（标记为已下载） */
    VersionStore::Instance().WriteA1(
        APP_CURRENT_VERSION,
        VERSION_FRAME_FLAG_DOWNLOADED
    );
}

bool UpdateService::HandleProtocolUpgradeRequest(const uint8_t *data, uint8_t len, bool &should_reset)
{
    should_reset = false;

    if (data == nullptr || len != PROTOCOL_UPGRADE_REQUEST_LEN) {
        return false;
    }

    uint64_t version = 0;
    if (!ParseAsciiVersion(data, version)) {
        return false;
    }

    const uint8_t flag = data[12];
    if (flag != VERSION_FRAME_FLAG_NEED_DOWNLOAD &&
        flag != VERSION_FRAME_FLAG_DOWNLOADED) {
        return false;
    }

    if (!VersionStore::Instance().WriteA2(version, flag)) {
        return false;
    }

    should_reset = (flag == VERSION_FRAME_FLAG_NEED_DOWNLOAD);
    return true;
}

void UpdateService::ResetToBootloaderAfterAck()
{
    BspTimOs_DelayMs(50);
    BspTimOs_SystemReset();
}

bool UpdateService::ParseAsciiVersion(const uint8_t *data, uint64_t &version)
{
    if (data == nullptr) {
        return false;
    }

    uint64_t value = 0;
    for (uint8_t i = 0; i < 12; ++i) {
        if (data[i] < '0' || data[i] > '9') {
            return false;
        }
        value = value * 10U + static_cast<uint64_t>(data[i] - '0');
    }

    version = value;
    return true;
}
