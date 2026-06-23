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

typedef struct Config_t
{
    workStatus_t workStatus;
    uint16_t Version = 201; //V2.01

}SystemConfig_t;


extern SystemConfig_t systemConfig;
extern uint8_t GPSMeterModel;
#endif //XY7000X_MAIN_CONFIGURATIONS_H