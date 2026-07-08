//
// Created by XYKJ on 2026/7/8.
//

#include "dmr_power_measurement.h"
#include "calibration_config.h"
#include "calibration_table.h"
namespace
{
    constexpr float DMR_P1_OFFSET_DB = 30.72f;
    constexpr float DMR_P2_OFFSET_DB = -2.2f;
    constexpr float DMR_P3_OFFSET_DB = 30.55f;
    constexpr float DMR_P4_OFFSET_DB = -2.42f;
    constexpr uint32_t DMR_TIMEOUT_MS = 1500u;
    constexpr uint32_t DMR_VALID_POWER_UW = 1u;
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
}// namespace

void DMRPowerMeasurement::Reset()
{
    hasValidSignal_ = false;
    lastValidTimeMs_ = 0;
    lastw1xUw_ = 0;            ///< 最近一次有效正向功率，单位 uW
    lastw2xUw_ = 0;            ///< 最近一次有效反向功率，单位 uW
    lastw3xUw_ = 0;
    lastw4xUw_ = 0;

    lastDbm1X100_ = 0;          ///< 最近一次有效正向功率，单位 dBm * 100
    lastDbm2X100_ = 0;          ///< 最近一次有效反向功率，单位 dBm * 100
    lastDbm3X100_ = 0;
    lastDbm4X100_ = 0;
    pepW1xUw_ = 0;
    pepW2xUw_ = 0;
    pepW3xUw_ = 0;
    pepW4xUw_ = 0;
}

static inline uint32_t hold_if_transient_zero(uint32_t newv, uint32_t lastv, uint8_t *zeroCnt)
{
    const uint8_t ZERO_ACCEPT_COUNT = 8;

    if (lastv > 0 && newv == 0) {
        if (*zeroCnt < ZERO_ACCEPT_COUNT) {
            (*zeroCnt)++;
            return lastv;
        }

        return 0;
    }

    *zeroCnt = 0;
    return newv;
}
/**
  * @brief  绝对值
  * @note   None
  */

static inline float hold_if_within_pct(float newv, float lastv, float pct)
{
    // lastv 太小的时候，百分比会失真（比如 lastv≈0）
    // 这里用一个最小参考值，避免 0 除
    const float MIN_REF = 1e-6f;

    float ref = (lastv > MIN_REF) ? lastv : MIN_REF;
    float diff = newv - lastv;
    if (diff < 0) diff = -diff;

    // diff/ref <= pct  => 保持 lastv
    if (diff <= ref * pct) {
        return lastv;
    }
    return newv;
}

bool DMRPowerMeasurement::Update(uint16_t p1v,
            uint16_t p2v,
            uint16_t p3v,
            uint16_t p4v,
            uint32_t now_ms,
            DMRPowerData_t* out)
{
    if (out == nullptr) {
        return false;
    }
#if CALIB_ENABLE_DMR_TABLE
    float dbm1 = 0.0f;
    float dbm2 = 0.0f;
    float dbm3 = 0.0f;
    float dbm4 = 0.0f;
    uint32_t w1x_uw = 0;
    uint32_t w2x_uw = 0;
    uint32_t w3x_uw = 0;
    uint32_t w4x_uw = 0;

    const uint16_t raw_p2v = p2v;
    const uint16_t raw_p4v = p4v;
    const uint16_t lookup_p2v = (p2v <= 1600u) ? (uint16_t)(1600u - p2v) : 0u;
    const uint16_t lookup_p4v = (p4v <= 1600u) ? (uint16_t)(1600u - p4v) : 0u;

    if (!Calibration_ADL8361AdcToDbm(p1v, DMR_P1_OFFSET_DB, &dbm1) ||
        !Calibration_ADL8317AdcToDbm(lookup_p2v, DMR_P2_OFFSET_DB, &dbm2) ||
        !Calibration_ADL8361AdcToDbm(p3v, DMR_P3_OFFSET_DB, &dbm3) ||
        !Calibration_ADL8317AdcToDbm(lookup_p4v, DMR_P4_OFFSET_DB, &dbm4)) {
        return false;
    }

    if (!Calibration_DbmToUw(dbm1, &w1x_uw) ||
        !Calibration_DbmToUw(dbm2, &w2x_uw) ||
        !Calibration_DbmToUw(dbm3, &w3x_uw) ||
        !Calibration_DbmToUw(dbm4, &w4x_uw)) {
        return false;
    }

    int16_t dbm1_x100 = (w1x_uw >= DMR_VALID_POWER_UW) ? DbmToX100(dbm1) : 0;
    int16_t dbm2_x100 = (w2x_uw >= DMR_VALID_POWER_UW) ? DbmToX100(dbm2) : 0;
    int16_t dbm3_x100 = (w3x_uw >= DMR_VALID_POWER_UW) ? DbmToX100(dbm3) : 0;
    int16_t dbm4_x100 = (w4x_uw >= DMR_VALID_POWER_UW) ? DbmToX100(dbm4) : 0;

    const bool currentValid = (w1x_uw >= DMR_VALID_POWER_UW) ||
                              (w3x_uw >= DMR_VALID_POWER_UW);

    if (currentValid) {
        if (w1x_uw >= DMR_VALID_POWER_UW) {
            w2x_uw = hold_if_transient_zero(w2x_uw, lastw2xUw_, &w2zeroHoldCnt);
        }
        if (w3x_uw >= DMR_VALID_POWER_UW) {
            w4x_uw = hold_if_transient_zero(w4x_uw, lastw4xUw_, &w4zeroHoldCnt);
        }

        w1x_uw = (uint32_t)hold_if_within_pct((float)w1x_uw, (float)lastw1xUw_, 0.015f);
        w2x_uw = (uint32_t)hold_if_within_pct((float)w2x_uw, (float)lastw2xUw_, 0.015f);
        w3x_uw = (uint32_t)hold_if_within_pct((float)w3x_uw, (float)lastw3xUw_, 0.015f);
        w4x_uw = (uint32_t)hold_if_within_pct((float)w4x_uw, (float)lastw4xUw_, 0.015f);

        lastValidTimeMs_ = now_ms;
        hasValidSignal_ = true;
        lastw1xUw_ = w1x_uw;
        lastw2xUw_ = w2x_uw;
        lastw3xUw_ = w3x_uw;
        lastw4xUw_ = w4x_uw;
        lastDbm1X100_ = dbm1_x100;
        lastDbm2X100_ = dbm2_x100;
        lastDbm3X100_ = dbm3_x100;
        lastDbm4X100_ = dbm4_x100;
    } else if (hasValidSignal_ && ((now_ms - lastValidTimeMs_) <= DMR_TIMEOUT_MS)) {
        w1x_uw = lastw1xUw_;
        w2x_uw = lastw2xUw_;
        w3x_uw = lastw3xUw_;
        w4x_uw = lastw4xUw_;
        dbm1_x100 = lastDbm1X100_;
        dbm2_x100 = lastDbm2X100_;
        dbm3_x100 = lastDbm3X100_;
        dbm4_x100 = lastDbm4X100_;
    } else {
        hasValidSignal_ = false;
        w1x_uw = 0;
        w2x_uw = 0;
        w3x_uw = 0;
        w4x_uw = 0;
        dbm1_x100 = 0;
        dbm2_x100 = 0;
        dbm3_x100 = 0;
        dbm4_x100 = 0;
        lastw1xUw_ = 0;
        lastw2xUw_ = 0;
        lastw3xUw_ = 0;
        lastw4xUw_ = 0;
        lastDbm1X100_ = 0;
        lastDbm2X100_ = 0;
        lastDbm3X100_ = 0;
        lastDbm4X100_ = 0;
        w2zeroHoldCnt = 0;
        w4zeroHoldCnt = 0;
    }

    if (w1x_uw > pepW1xUw_) {
        pepW1xUw_ = w1x_uw;
    }
    if (w2x_uw > pepW2xUw_) {
        pepW2xUw_ = w2x_uw;
    }
    if (w3x_uw > pepW3xUw_) {
        pepW3xUw_ = w3x_uw;
    }
    if (w4x_uw > pepW4xUw_) {
        pepW4xUw_ = w4x_uw;
    }

    out->p1v_413 = p1v;
    out->p2v_413 = raw_p2v;
    out->p1v_457 = p3v;
    out->p2v_457 = raw_p4v;
    out->w1x_uw_413 = w1x_uw;
    out->w2x_uw_413 = w2x_uw;
    out->w1x_uw_457 = w3x_uw;
    out->w2x_uw_457 = w4x_uw;
    out->w1x_uw_pep_413 = pepW1xUw_;
    out->w2x_uw_pep_413 = pepW2xUw_;
    out->w1x_uw_pep_457 = pepW3xUw_;
    out->w2x_uw_pep_457 = pepW4xUw_;
    out->dbm1_x100_413 = dbm1_x100;
    out->dbm2_x100_413 = dbm2_x100;
    out->dbm1_x100_457 = dbm3_x100;
    out->dbm2_x100_457 = dbm4_x100;
    out->valid = (w1x_uw > 0u) || (w3x_uw > 0u);
    return true;
#else
    (void)p1v;
    (void)p2v;
    (void)p3v;
    (void)p4v;
    (void)now_ms;
    *out = {};
    return false;
#endif

}
