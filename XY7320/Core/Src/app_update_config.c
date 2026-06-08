#include "app_update_config.h"

#include "app_version_config.h"
#include "usart.h"
#include <string.h>

#define APP_UPDATE_RX_BUF_SIZE 64U
#define APP_UPDATE_ACK_VERSION_FRAME "XYA1"

static uint8_t app_update_rx_buf[APP_UPDATE_RX_BUF_SIZE];
static uint8_t app_update_proc_buf[APP_UPDATE_RX_BUF_SIZE];
static volatile uint16_t app_update_rx_len = 0U;
static volatile uint8_t app_update_rx_done = 0U;
static volatile uint8_t app_update_rx_overflow = 0U;

void App_UpdateConfig_Init(void)
{
    (void)AppVersionConfig_UpdateA1(APP_CURRENT_VERSION, APP_VERSION_FLAG_DOWNLOADED);
}

void App_UpdateConfig_StartReceive(void)
{
    app_update_rx_done = 0U;
    app_update_rx_overflow = 0U;
    app_update_rx_len = 0U;

    (void)HAL_UART_Receive_DMA(&huart2, app_update_rx_buf, APP_UPDATE_RX_BUF_SIZE);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
}

void App_UpdateConfig_Poll(void)
{
    if (app_update_rx_done != 0U)
    {
        app_update_rx_done = 0U;
        (void)App_UpdateConfig_ProcessFrame(app_update_proc_buf, app_update_rx_len);
    }

    if (app_update_rx_overflow != 0U)
    {
        app_update_rx_overflow = 0U;
    }
}

void App_UpdateConfig_HandleUartIdleIrq(UART_HandleTypeDef *huart)
{
    uint16_t rx_len;

    if (huart == NULL || huart->Instance != USART2)
    {
        return;
    }

    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == RESET)
    {
        return;
    }

    __HAL_UART_CLEAR_IDLEFLAG(huart);
    HAL_UART_DMAStop(huart);

    rx_len = APP_UPDATE_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    if (rx_len > APP_UPDATE_RX_BUF_SIZE)
    {
        rx_len = APP_UPDATE_RX_BUF_SIZE;
    }

    if (app_update_rx_done == 0U)
    {
        app_update_rx_len = rx_len;
        memcpy(app_update_proc_buf, app_update_rx_buf, rx_len);
        app_update_rx_done = 1U;
    }
    else
    {
        app_update_rx_overflow = 1U;
    }

    (void)HAL_UART_Receive_DMA(huart, app_update_rx_buf, APP_UPDATE_RX_BUF_SIZE);
}

int App_UpdateConfig_ProcessFrame(const uint8_t *data, size_t len)
{
    uint64_t version = 0U;
    uint8_t flag = 0U;
    int ret;

    if (AppVersionConfig_ParseFrame(data, len, &version, &flag) < 0)
    {
        return -1;
    }

    ret = AppVersionConfig_UpdateA2(version, flag);
    if (ret == 0 && flag == APP_VERSION_FLAG_NEED_DOWNLOAD)
    {
        const uint8_t ack[] = APP_UPDATE_ACK_VERSION_FRAME;
        (void)HAL_UART_Transmit(&huart2, (uint8_t *)ack, sizeof(ack) - 1U, 100U);
        HAL_Delay(20U);
        NVIC_SystemReset();
    }

    return ret;
}
