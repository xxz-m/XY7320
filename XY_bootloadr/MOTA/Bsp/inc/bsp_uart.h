/**
 * \file            bsp_uart.h
 * \brief           UART driver
 */

/*
 * Copyright (c) 2022 Dino Haw
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of mOTA - The Over-The-Air technology component for MCU.
 *
 * Author:          Dino Haw <347341799@qq.com>
 * Version:         v1.0.0
 */

#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "bsp_uart_stm32.h"

#if (USING_RTOS_TYPE)
#define UART_INIT_PARA(x)                       \
{                                               \
    .id              = BSP_UART##x,             \
    .rx_buff         = _uart##x##_buff,         \
    .name            = "_u"#x,                  \
    .rx_buff_max_len = BSP_UART_BUFF_SIZE,      \
}
#else
#define UART_INIT_PARA(x)                       \
{                                               \
    .id              = BSP_UART##x,             \
    .rx_buff         = _uart##x##_buff,         \
    .rx_buff_max_len = BSP_UART_BUFF_SIZE,      \
}
#endif

#define UART(x)             _uart##x

#define UART_CREATE(x)      static uint8_t _uart##x##_buff[BSP_UART_BUFF_SIZE];     \
                            static struct UART_STRUCT _uart##x = UART_INIT_PARA(x);

typedef enum
{
#if (BSP_USING_UART1)
    BSP_UART1    = 0x01U,
#endif
#if (BSP_USING_UART2)
    BSP_UART2    = 0x02U,
#endif
#if (BSP_USING_UART2_RE)
    BSP_UART2_RE = 0x04U,
#endif
#if (BSP_USING_UART3)
    BSP_UART3    = 0x08U,
#endif
#if (BSP_USING_UART3_RE)
    BSP_UART3_RE = 0x10U,
#endif
#if (BSP_USING_UART4)
    BSP_UART4    = 0x20U,
#endif
#if (BSP_USING_UART5)
    BSP_UART5    = 0x40U,
#endif
#if (BSP_USING_UART6)
    BSP_UART6    = 0x80U,
#endif
} BSP_UART_ID;

typedef enum
{
    BSP_UART_ERR_OK                 = 0x00U,
    BSP_UART_ERR_COMM_ERR           = 0x01U,
    BSP_UART_ERR_BUSY               = 0x02U,
    BSP_UART_ERR_TIMEOUT            = 0x03U,
    BSP_UART_ERR_NOT_FOUND          = 0x04U,
    BSP_UART_ERR_LOCK_INIT_ERR      = 0x05U,
    BSP_UART_ERR_LOCK_ERR           = 0x06U,
    BSP_UART_ERR_UNLOCK_ERR         = 0x07U,
    BSP_UART_ERR_NO_RECV_FRAME      = 0x08U,
    BSP_UART_ERR_NO_INIT            = 0x09U,
    BSP_UART_ERR_NAME_DUPLICATE     = 0x0AU,

} BSP_UART_ERR;

struct UART_STRUCT
{
    UART_HandleTypeDef      handle;

#if defined(RTOS_USING_RTTHREAD) && (USING_RTOS_TYPE == RTOS_USING_RTTHREAD)
    struct rt_semaphore     rx_sem;
    struct rt_semaphore     tx_lock;
#elif defined(RTOS_USING_UCOS) && (USING_RTOS_TYPE == RTOS_USING_UCOS)
    OS_SEM     rx_sem;
    OS_SEM     tx_lock;
#endif

    const BSP_UART_ID id;
#if (USING_RTOS_TYPE)
    const char name[ MAX_NAME_LEN ];
#endif

    uint8_t  *rx_data;
    uint16_t * volatile rx_data_len;
    uint16_t rx_data_max_len;

    volatile bool is_init;
    volatile bool is_rx_init;
    volatile bool is_idle_int;

    const uint8_t  *rx_buff;
    const uint16_t rx_buff_max_len;

    uint16_t old_pos;

    uint8_t (*RX_Indicate)(struct UART_STRUCT *uart);
    uint8_t (*TX_Complete)(struct UART_STRUCT *uart);

    void *user_data;
};

typedef void (*UART_Callback_t)(struct UART_STRUCT *uart);

#if (USING_RTOS_TYPE)
BSP_UART_ERR    BSP_UART_WaitForData        (BSP_UART_ID  id);
#endif
BSP_UART_ERR    BSP_UART_Init               (BSP_UART_ID  id);
BSP_UART_ERR    BSP_UART_EnableReceive      (BSP_UART_ID  id, uint8_t *data, uint16_t *len, uint16_t max_len);
BSP_UART_ERR    BSP_UART_DisableReceive     (BSP_UART_ID  id);
BSP_UART_ERR    BSP_UART_LinkUserData       (BSP_UART_ID  id, void *user_data);
#if (USING_RTOS_TYPE)
BSP_UART_ERR    BSP_UART_Send               (BSP_UART_ID  id, const uint8_t *data, uint16_t len);
BSP_UART_ERR    BSP_UART_SendBlocking       (BSP_UART_ID  id, const uint8_t *data, uint16_t len, uint16_t timeout);
#else
BSP_UART_ERR    BSP_UART_Send               (BSP_UART_ID  id, const uint8_t *data, uint16_t len, uint16_t timeout);
#endif
#if (ENABLE_DEBUG_PRINT && EANBLE_PRINTF_USING_RTT == 0)
void            BSP_Printf                  (const char *fmt, ...);
#endif
BSP_UART_ERR    BSP_UART_SetTxIndicate      (BSP_UART_ID  id, uint8_t (*TX_Complete)(struct UART_STRUCT *uart));
BSP_UART_ERR    BSP_UART_ClearUserBuff      (BSP_UART_ID  id);
BSP_UART_ERR    BSP_UART_IsFrameEnd         (BSP_UART_ID  id);


#endif
