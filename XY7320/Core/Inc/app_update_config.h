#ifndef APP_UPDATE_CONFIG_H
#define APP_UPDATE_CONFIG_H

#include "main.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_CURRENT_VERSION 202606082257ULL

void App_UpdateConfig_Init(void);
void App_UpdateConfig_StartReceive(void);
void App_UpdateConfig_Poll(void);
void App_UpdateConfig_HandleUartIdleIrq(UART_HandleTypeDef *huart);
int App_UpdateConfig_ProcessFrame(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
