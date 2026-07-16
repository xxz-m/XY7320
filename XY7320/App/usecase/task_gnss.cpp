/**
 * @file    task_gnss.cpp
 * @brief   GNSS 接收解析任务实现
 */

#include "task_gnss.h"
#include "gnss_service.h"
#include "app_config.h"
#include "uart_tx_service.h"
#include "mode_manager.h"
#include "configurations.h"
#include "bsp_tim_os.h"

namespace {
constexpr uint32_t kGnssUploadPeriodMs = 1000U;
constexpr uint8_t kGnssPayloadSize = 17U;

void WriteU16Be(uint8_t *dst, uint16_t value)
{
    dst[0] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    dst[1] = static_cast<uint8_t>(value & 0xFFU);
}

void WriteU32Be(uint8_t *dst, uint32_t value)
{
    dst[0] = static_cast<uint8_t>((value >> 24) & 0xFFU);
    dst[1] = static_cast<uint8_t>((value >> 16) & 0xFFU);
    dst[2] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    dst[3] = static_cast<uint8_t>(value & 0xFFU);
}
} // namespace

TaskstateGnss& TaskstateGnss::Instance()
{
    static TaskstateGnss instance;
    return instance;
}

TaskstateGnss::TaskstateGnss() : fsm::State("GNSS") {}

void TaskstateGnss::entry()
{
    /*
    * GnssService::Init() 已经在 App_Main_Init() 中执行，
    * 这里不重复初始化 USART3、DMA 和 IDLE 中断。
    */
    GnssService::Instance().Start();
    LOG_Printf("TaskStateGnss,Entry\n");
}

void TaskstateGnss::exit()
{
    GnssService::Instance().Stop();
    LOG_Printf("TaskStateGnss,Exit\n");
}

void TaskstateGnss::react(const fsm::Event& event)
{
    (void)event;
}

void TaskstateGnss::tick()
{
    GnssService::Instance().Update();

    static uint32_t s_lastUploadMs = 0U;
    static uint16_t s_seq = 0U;

    const uint32_t now = BspTimOs_GetTick();
    if ((now - s_lastUploadMs) < kGnssUploadPeriodMs) {
        return;
    }

    const GnssFix& fix = GnssService::Instance().GetFix();
    if (!fix.hasFix) {
        return;
    }
    s_lastUploadMs = now;
    ++s_seq;

    /*
     * GNSS payload 使用稳定的大端定点格式：latitudeE7、longitudeE7、
     * fixQuality、satelliteCount、hdopX100、altitudeCm、hasFix。
     */
    uint8_t payload[kGnssPayloadSize]{};
    WriteU32Be(&payload[0], static_cast<uint32_t>(fix.latitudeE7));
    WriteU32Be(&payload[4], static_cast<uint32_t>(fix.longitudeE7));
    payload[8] = static_cast<uint8_t>(fix.fixQuality);
    payload[9] = fix.satelliteCount;
    WriteU16Be(&payload[10], fix.hdopX100);
    WriteU32Be(&payload[12], static_cast<uint32_t>(fix.altitudeCm));
    payload[16] = fix.hasFix ? 1U : 0U;

    (void)UartTxService::Instance().PublishModeData(
        UPLINK_CMD_GNSS_MEAS,
        mode::MODE_GNSS,
        ModeManager::Instance().currentGeneration(),
        s_seq,
        payload,
        sizeof(payload));
}
