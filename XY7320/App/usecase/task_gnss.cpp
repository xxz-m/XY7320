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
constexpr uint8_t kGnssPayloadSize = 36U;

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

    uint8_t payload[kGnssPayloadSize]{};
    payload[0] = ModeManager::Instance().gpsModel();
    WriteU16Be(&payload[1], 0U);
    WriteU16Be(&payload[3], 0U);
    WriteU16Be(&payload[5], 0U);
    WriteU16Be(&payload[7], 0U);
    payload[9] = static_cast<uint8_t>(fix.fixQuality);
    payload[10] = fix.longitudeE7 < 0 ? static_cast<uint8_t>('W') : static_cast<uint8_t>('E');
    WriteU32Be(&payload[11], static_cast<uint32_t>(fix.longitudeE7 < 0 ? -fix.longitudeE7 : fix.longitudeE7) / 100U);
    payload[15] = fix.latitudeE7 < 0 ? static_cast<uint8_t>('S') : static_cast<uint8_t>('N');
    WriteU32Be(&payload[16], static_cast<uint32_t>(fix.latitudeE7 < 0 ? -fix.latitudeE7 : fix.latitudeE7) / 100U);
    payload[20] = fix.satelliteCount;
    payload[21] = fix.gpsSatelliteCount;
    payload[22] = fix.beidouSatelliteCount;
    payload[23] = fix.gpsMaxSnr;
    payload[24] = fix.beidouMaxSnr;
    WriteU16Be(&payload[25], fix.speedKmhX1000);
    WriteU16Be(&payload[27], static_cast<uint16_t>(fix.altitudeCm / 10));
    WriteU16Be(&payload[29], fix.utc.year);
    payload[31] = fix.utc.month;
    payload[32] = fix.utc.day;
    payload[33] = fix.utc.hour;
    payload[34] = fix.utc.minute;
    payload[35] = fix.utc.second;

    (void)UartTxService::Instance().PublishModeData(
        UPLINK_CMD_GNSS_MEAS,
        mode::MODE_GNSS,
        ModeManager::Instance().currentGeneration(),
        s_seq,
        payload,
        sizeof(payload));
}
