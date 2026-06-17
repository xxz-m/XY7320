/**
* @file    bsp_uart.h
 * @brief   串口通用收发实现
 */
#ifndef XY7320_BSP_UART_H
#define XY7320_BSP_UART_H
#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  初始化日志输出串口
 */
void BspUart_LogInit(UART_HandleTypeDef *huart);

/** 日志输出：发送单个字符（需先调用 BspUart_LogInit） */
void BspUart_LogPutChar(char ch);

/** 日志输出：发送字符串 */
void BspUart_LogWrite(const char *str);

/**
 * @brief  发送数据（通用接口）
 */
bool BspUart_Send(UART_HandleTypeDef *huart, const uint8_t *data, uint16_t len, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif //XY7320_BSP_UART_H
