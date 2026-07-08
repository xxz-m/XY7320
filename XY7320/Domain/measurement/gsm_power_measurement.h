//
// Created by Administrator on 2026/7/7.
//

#ifndef XY7320_GSM_POWER_MEASUREMENT_H
#define XY7320_GSM_POWER_MEASUREMENT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief GSM 功率测量输出数据
 *
 * 数据来源：
 *   p1v <- ScopeResult_t::wavePEP5_avg，正向功率检测 ADC/mV 滤波值
 *   p2v <- ScopeResult_t::wavePEP6_avg，反向功率检测 ADC/mV 滤波值
 *
 * 单位约定：
 *   - p1v / p2v：ADC/mV 滤波值
 *   - w1x_uw / w2x_uw：线性功率，单位 uW
 *   - dbm1_x100 / dbm2_x100：对数功率，单位 dBm * 100
 */
typedef struct
{
    uint16_t p1v;          ///< 正向 ADC/mV 滤波值，来源 wavePEP5_avg
    uint16_t p2v;          ///< 反向 ADC/mV 滤波值，来源 wavePEP6_avg

    uint32_t w1x_uw;       ///< 正向功率，单位 uW
    uint32_t w2x_uw;       ///< 反向功率，单位 uW

    uint32_t w1x_uw_pep;   ///< 正向峰值功率保持，单位 uW
    uint32_t w2x_uw_pep;   ///< 反向峰值功率保持，单位 uW

    int16_t dbm1_x100;     ///< 正向功率，单位 dBm * 100
    int16_t dbm2_x100;     ///< 反向功率，单位 dBm * 100

    bool valid;            ///< 当前输出是否来自有效信号或保持窗口
} GsmPowerData_t;

/**
 * @brief GSM 功率测量器
 *
 * 职责：
 *   1. 接收 Oscilloscope 已滤波后的 GSM 两路 ADC/mV 值。
 *   2. 使用 ADL5906A 校准表换算 dBm。
 *   3. 换算 uW，生成协议上传需要的 dBm*100 与 uW 数据。
 *   4. 在短时间掉信号时保持上一次有效功率，避免显示闪烁。
 *   5. 维护 PEP 峰值功率。
 */
class GSMPowerMeasurement {
public:
    /**
     * @brief 清空保持状态和 PEP 峰值
     */
    void Reset();

    /**
     * @brief 更新 GSM 功率测量结果
     *
     * 公式：
     *   ADL5906A ADC/mV -> dBm = 查表插值结果 + GSM 链路补偿
     *   uW = 1000 * 10 ^ (dBm / 10)
     *   dbm_x100 = round(dBm * 100)
     *
     * 保持逻辑：
     *   - 当前正向功率 >= 有效阈值：更新 lastValid。
     *   - 当前无效但未超过保持时间：输出 lastValid。
     *   - 超过保持时间：输出清零。
     *
     * @param p1v     正向 ADC/mV 滤波值，来源 wavePEP5_avg
     * @param p2v     反向 ADC/mV 滤波值，来源 wavePEP6_avg
     * @param now_ms  当前系统时间，单位 ms，通常来自 HAL_GetTick()
     * @param out     输出 GSM 功率测量数据
     *
     * @return true 更新成功，false 参数错误或校准换算失败
     */
    bool Update(uint16_t p1v,
                uint16_t p2v,
                uint32_t now_ms,
                GsmPowerData_t* out);

private:
    bool hasValidSignal_ = false;       ///< 是否已有可用于保持的有效信号
    uint32_t lastValidTimeMs_ = 0;      ///< 最近一次有效信号时间，单位 ms

    uint32_t lastW1xUw_ = 0;            ///< 最近一次有效正向功率，单位 uW
    uint32_t lastW2xUw_ = 0;            ///< 最近一次有效反向功率，单位 uW
    int16_t lastDbm1X100_ = 0;          ///< 最近一次有效正向功率，单位 dBm * 100
    int16_t lastDbm2X100_ = 0;          ///< 最近一次有效反向功率，单位 dBm * 100

    uint32_t pepW1xUw_ = 0;             ///< 正向 PEP 峰值功率，单位 uW
    uint32_t pepW2xUw_ = 0;             ///< 反向 PEP 峰值功率，单位 uW
};

#endif // XY7320_GSM_POWER_MEASUREMENT_H
