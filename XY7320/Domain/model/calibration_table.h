//
// Created by XYKJ on 2026/7/7.
//

#ifndef XY7320_CALIBRATION_TABLE_H
#define XY7320_CALIBRATION_TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include "calibration_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 校准表约定：
 * - ADC 表按采样值/mV 从大到小排列。
 * - dBm 表统一使用 dBm * 10 的定点格式。
 * - 对外输出 dBm 时使用 float，单位为 dBm。
 */

#if CALIB_ENABLE_GSM_TABLE
/* GSM：ADL5906A 功率检测芯片校准表 */
extern const uint16_t ad_5906_mv_adc[];
extern const int16_t ad_5906_dbmX10[];

uint16_t Calibration_Get5906TableLength(void);
bool Calibration_5906AdcToDbm(uint16_t value,
                              float offset_db,
                              float* out_dbm);
#endif

#if CALIB_ENABLE_DMR_TABLE
/* DMR：ADL8361 正向功率检测芯片校准表 */
extern const uint16_t ad_adl8361_mv_adc[];
extern const int16_t ad_adl8361_dbmX10[];
/* DMR：ADL8317 反向功率检测芯片校准表 */
extern const uint16_t ad_adl8317_mv_adc[];
extern const int16_t ad_adl8317_dbmX10[];

uint16_t Calibration_GetADL8361TableLength(void);
uint16_t Calibration_GetADL8317TableLength(void);
bool Calibration_ADL8361AdcToDbm(uint16_t value,
                                 float offset_db,
                                 float* out_dbm);
bool Calibration_ADL8317AdcToDbm(uint16_t value,
                                 float offset_db,
                                 float* out_dbm);
#endif

bool Calibration_AdcToDbm(const uint16_t ad_table[],
                          const int16_t dbm_x10_table[],
                          uint16_t len,
                          uint16_t value,
                          float offset_db,
                          float* out_dbm);

bool Calibration_AdcToUw(const uint16_t ad_table[],
                         const uint32_t uw_table[],
                         uint16_t len,
                         uint16_t value,
                         int32_t offset_uw,
                         uint32_t* out_uw);

bool Calibration_DbmToUw(float dbm, uint32_t* out_uw);
bool Calibration_UwToDbm(uint32_t uw, float* out_dbm);

#ifdef __cplusplus
}
#endif

#endif // XY7320_CALIBRATION_TABLE_H
