//
// Created by XYKJ on 2026/7/8.
//

#ifndef XY7320_DMR_POWER_MEASUREMENT_H
#define XY7320_DMR_POWER_MEASUREMENT_H
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint16_t p1v_413;          ///< 正向 ADC/mV 滤波值，来源 wavePEP1_avg
    uint16_t p2v_413;          ///< 反向 ADC/mV 滤波值，来源 wavePEP2_avg
    uint16_t p1v_457;          ///< 正向 ADC/mV 滤波值，来源 wavePEP1_avg
    uint16_t p2v_457;          ///< 反向 ADC/mV 滤波值，来源 wavePEP2_avg


    uint32_t w1x_uw_413;       ///< 正向功率，单位 uW
    uint32_t w2x_uw_413;       ///< 反向功率，单位 uW
    uint32_t w1x_uw_457;       ///< 正向功率，单位 uW
    uint32_t w2x_uw_457;       ///< 反向功率，单位 uW

    uint32_t w1x_uw_pep_413;   ///< 正向峰值功率保持，单位 uW
    uint32_t w2x_uw_pep_413;   ///< 反向峰值功率保持，单位 uW
    uint32_t w1x_uw_pep_457;   ///< 正向峰值功率保持，单位 uW
    uint32_t w2x_uw_pep_457;   ///< 反向峰值功率保持，单位 uW

    int16_t dbm1_x100_413;     ///< 正向功率，单位 dBm * 100
    int16_t dbm2_x100_413;     ///< 反向功率，单位 dBm * 100

    int16_t dbm1_x100_457;     ///< 正向功率，单位 dBm * 100
    int16_t dbm2_x100_457;     ///< 反向功率，单位 dBm * 100
    bool valid;            ///< 当前输出是否来自有效信号或保持窗口
} DMRPowerData_t;


class DMRPowerMeasurement {
public:
    /**
     * @brief 清空保持状态和 PEP 峰值
     */
    void Reset();

    /**
     * @brief 更新 DMR 功率测量结果
     *
     * 公式：
     *   P1/P3：ADL8361 正向表 ADC/mV -> dBm = 查表插值结果 + 链路补偿
     *   P2/P4：1600 - 原始 ADC/mV 后使用 ADL8317 反向表换算 dBm
     *   uW = 1000 * 10 ^ (dBm / 10)
     *   dbm_x100 = round(dBm * 100)
     *
     * 保持逻辑：
     *   - 当前正向功率 >= 有效阈值：更新 lastValid。
     *   - 当前无效但未超过保持时间：输出 lastValid。
     *   - 超过保持时间：输出清零。
     *
     * @param p1v     413 正向 ADC/mV 滤波值，来源 wavePEP1_avg
     * @param p2v     413 反向 ADC/mV 滤波值，来源 wavePEP2_avg
     * @param p3v     457 正向 ADC/mV 滤波值，来源 wavePEP3_avg
     * @param p4v     457 反向 ADC/mV 滤波值，来源 wavePEP4_avg
     * @param now_ms  当前系统时间，单位 ms，通常来自 HAL_GetTick()
     * @param out     输出 DMR 功率测量数据
     *
     * @return true 更新成功，false 参数错误或校准换算失败
     */
    bool Update(uint16_t p1v,
                uint16_t p2v,
                uint16_t p3v,
                uint16_t p4v,
                uint32_t now_ms,
                DMRPowerData_t* out);

private:
    bool hasValidSignal_ = false;       ///< 是否已有可用于保持的有效信号
    uint32_t lastValidTimeMs_ = 0;      ///< 最近一次有效信号时间，单位 ms

    uint32_t lastw1xUw_ = 0;            ///< 最近一次有效正向功率，单位 uW
    uint32_t lastw2xUw_ = 0;            ///< 最近一次有效反向功率，单位 uW
    uint32_t lastw3xUw_ = 0;
    uint32_t lastw4xUw_ = 0;

    int16_t lastDbm1X100_ = 0;          ///< 最近一次有效正向功率，单位 dBm * 100
    int16_t lastDbm2X100_ = 0;          ///< 最近一次有效反向功率，单位 dBm * 100
    int16_t lastDbm3X100_ = 0;
    int16_t lastDbm4X100_ = 0;

    uint32_t pepW1xUw_ = 0;             ///< 正向 PEP 峰值功率，单位 uW
    uint32_t pepW2xUw_ = 0;             ///< 反向 PEP 峰值功率，单位 uW
    uint32_t pepW3xUw_ = 0;
    uint32_t pepW4xUw_ = 0;
    uint8_t w4zeroHoldCnt = 0;
    uint8_t w2zeroHoldCnt = 0;
};

#endif //XY7320_DMR_POWER_MEASUREMENT_H
