/**
  ******************************************************************************
  * @file           : configurations.h
  * @author         : TXD
  * @brief          : None
  * @attention      : None
  * @date           : 2022/12/28
  ******************************************************************************
  */
#ifndef __CONFIGURATIONS_H
#define __CONFIGURATIONS_H

/*---------------------------- C Scope ---------------------------*/
#include "stdint.h"
#define FEATURE_GPS 0

typedef enum workStatus_t
{
    WAIT_MODEL = 0x20,
    POWER_MODEL = 0x21,
    GPS_MODEL = 0x22,
    MOHMS_MODEL = 0x23,
    CALI_MODEL = 0x24,
    BOOT_MODEL = 0x25,
    BUZZER_MODEL = 0x26,
    Analysis_MODEL = 0x27,
    POWER_MODEL_GSM = 0x28
}workStatus_t;

/* ---------------------------------------------------------------------------
 * UART2 上行业务帧命令码与字段长度（占位定义，后续有真值后再覆盖）
 *
 * 设计意图：
 *  1. 把上行帧命令码集中放到 Domain 层，避免散落到 Service/BSP；
 *  2. 与 protocol_xy.h 中读命令码（如 DeviceButton=0x30）数值相同，
 *     但方向由 model 字段区分：读 model=0x01 unRead、上行 model=0x02 unWrite，
 *     不会在协议解析时冲突；
 *  3. UPLINK_CMD_*_MEAS 在 App/usecase 的对应 FSM 状态里使用；
 *     UPLINK_CMD_BATTERY 暂不实现，留作后续 battery/temp 扩展位。
 *  4. 字段长度/上限集中定义，便于 UartTxService 与上位机共享。
 * --------------------------------------------------------------------------- */
#define UPLINK_CMD_DMR_MEAS   0x30U  /* DMR 模式测量数据上行 */
#define UPLINK_CMD_GSM_MEAS   0x31U  /* GSM 模式测量数据上行 */
#define UPLINK_CMD_GNSS_MEAS  0x32U  /* GNSS 模式数据上行 */
#define UPLINK_CMD_BATTERY    0x33U  /* 电池数据上行（预留，当前阶段不实现） */

#define UPLINK_FIELD_MODE_LEN         1U
#define UPLINK_FIELD_GENERATION_LEN   2U
#define UPLINK_FIELD_SEQ_LEN          2U
#define UPLINK_FIELD_PAYLOAD_LEN_LEN  1U
#define UPLINK_FIELD_RESERVED_LEN     2U

#define UPLINK_HEADER_LEN             (UPLINK_FIELD_MODE_LEN \
                                      + UPLINK_FIELD_GENERATION_LEN \
                                      + UPLINK_FIELD_SEQ_LEN \
                                      + UPLINK_FIELD_PAYLOAD_LEN_LEN)
#define UPLINK_PAYLOAD_MAX            96U   /* 占位 payload 上限 */
#define UPLINK_TOTAL_MAX              (UPLINK_HEADER_LEN \
                                      + UPLINK_PAYLOAD_MAX \
                                      + UPLINK_FIELD_RESERVED_LEN)

typedef struct Config_t
{
    workStatus_t workStatus;
    uint16_t Version = 201; //V2.01

}SystemConfig_t;


extern SystemConfig_t systemConfig;
extern uint8_t GPSMeterModel;
#endif //XY7000X_MAIN_CONFIGURATIONS_H