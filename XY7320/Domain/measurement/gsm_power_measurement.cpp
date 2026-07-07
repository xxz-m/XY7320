//
// Created by Administrator on 2026/7/7.
//

#include "gsm_power_measurement.h"
#include "calibration_config.h"
#include "calibration_table.h"

namespace {
constexpr float GSM_LINK_OFFSET_DB = 35.1f;
constexpr uint32_t GSM_TIMEOUT_MS = 1500u;
constexpr uint32_t GSM_VALID_POWER_UW = 1u;

/**
 * @brief 将 float dBm 转为协议使用的 dBm * 100 定点值
 *
 * 公式：
 *   dbm_x100 = round(dBm * 100)
 *
 * @param dbm 输入功率，单位 dBm
 * @return int16_t 输出功率，单位 dBm * 100
 */
int16_t DbmToX100(float dbm)
{
    return (int16_t)((dbm >= 0.0f)
        ? (dbm * 100.0f + 0.5f)
        : (dbm * 100.0f - 0.5f));
}
} // namespace

void GsmPowerMeasurement::Reset()
{
    hasValidSignal_ = false;
    lastValidTimeMs_ = 0;
    lastW1xUw_ = 0;
    lastW2xUw_ = 0;
    lastDbm1X100_ = 0;
    lastDbm2X100_ = 0;
    pepW1xUw_ = 0;
    pepW2xUw_ = 0;
}

bool GsmPowerMeasurement::Update(uint16_t p1v,
                                 uint16_t p2v,
                                 uint32_t now_ms,
                                 GsmPowerData_t* out)
{
    if (out == nullptr) {
        return false;
    }

#if CALIB_ENABLE_GSM_TABLE
    float dbm1 = 0.0f;
    float dbm2 = 0.0f;
    uint32_t w1x_uw = 0;
    uint32_t w2x_uw = 0;

    if (!Calibration_5906AdcToDbm(p1v, GSM_LINK_OFFSET_DB, &dbm1) ||
        !Calibration_5906AdcToDbm(p2v, GSM_LINK_OFFSET_DB, &dbm2)) {
        return false;
    }

    if (!Calibration_DbmToUw(dbm1, &w1x_uw) ||
        !Calibration_DbmToUw(dbm2, &w2x_uw)) {
        return false;
    }

    int16_t dbm1_x100 = (w1x_uw >= GSM_VALID_POWER_UW) ? DbmToX100(dbm1) : 0;
    int16_t dbm2_x100 = (w2x_uw > 0u) ? DbmToX100(dbm2) : 0;
    const bool currentValid = (w1x_uw >= GSM_VALID_POWER_UW);

    if (currentValid) {
        lastValidTimeMs_ = now_ms;
        hasValidSignal_ = true;
        lastW1xUw_ = w1x_uw;
        lastW2xUw_ = w2x_uw;
        lastDbm1X100_ = dbm1_x100;
        lastDbm2X100_ = dbm2_x100;
    } else if (hasValidSignal_ && ((now_ms - lastValidTimeMs_) <= GSM_TIMEOUT_MS)) {
        w1x_uw = lastW1xUw_;
        w2x_uw = lastW2xUw_;
        dbm1_x100 = lastDbm1X100_;
        dbm2_x100 = lastDbm2X100_;
    } else {
        hasValidSignal_ = false;
        w1x_uw = 0;
        w2x_uw = 0;
        dbm1_x100 = 0;
        dbm2_x100 = 0;
        lastW1xUw_ = 0;
        lastW2xUw_ = 0;
        lastDbm1X100_ = 0;
        lastDbm2X100_ = 0;
    }

    if (w1x_uw > pepW1xUw_) {
        pepW1xUw_ = w1x_uw;
        pepW2xUw_ = w2x_uw;
    }

    out->p1v = p1v;
    out->p2v = p2v;
    out->w1x_uw = w1x_uw;
    out->w2x_uw = w2x_uw;
    out->w1x_uw_pep = pepW1xUw_;
    out->w2x_uw_pep = pepW2xUw_;
    out->dbm1_x100 = dbm1_x100;
    out->dbm2_x100 = dbm2_x100;
    out->valid = (w1x_uw > 0u);
    return true;
#else
    (void)p1v;
    (void)p2v;
    (void)now_ms;
    out->p1v = 0;
    out->p2v = 0;
    out->w1x_uw = 0;
    out->w2x_uw = 0;
    out->w1x_uw_pep = pepW1xUw_;
    out->w2x_uw_pep = pepW2xUw_;
    out->dbm1_x100 = 0;
    out->dbm2_x100 = 0;
    out->valid = false;
    return false;
#endif
}
