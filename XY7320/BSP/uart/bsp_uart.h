/**
 * @file    bsp_uart.h
 * @brief   串口通用收发与日志输出 BSP 模块
 *
 *          提供两类接口：
 *          - 日志通道（LogInit / LogPutChar / LogWrite）：单字符阻塞发送，
 *            专供 nanoprintf 的 putc 回调使用；
 *          - 通用通道（Send）：阻塞发送一帧数据。
 *
 *          与 bsp_uart_rcv.h 的区别：本模块不管理 DMA / IDLE 中断，
 *          仅做最基础的字节/字符串发送。
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
 * @brief  初始化日志串口
 *
 * @param  huart  日志串口句柄（通常为 &huart1）
 *
 * @note   调用本函数后，BspUart_LogPutChar / BspUart_LogWrite 才可使用。
 */
void BspUart_LogInit(UART_HandleTypeDef *huart);

/**
 * @brief  日志通道发送单个字符（阻塞）
 *
 * @param  ch  要发送的字符
 *
 * @note   是 nanoprintf 的 putc 回调契约入口；调用方应先完成 LogInit。
 */
void BspUart_LogPutChar(char ch);

/**
 * @brief  日志通道发送字符串
 *
 * @param  str  以 '\\0' 结尾的 C 字符串
 *
 * @note   内部循环调用 LogPutChar。
 */
void BspUart_LogWrite(const char *str);

/**
 * @brief  通用串口阻塞发送
 *
 * @param  huart    串口句柄
 * @param  data     待发送数据
 * @param  len      数据长度（字节）
 * @param  timeout  HAL 单次发送超时（ms）
 * @return true 成功；false 失败（HAL 返回非 HAL_OK 或参数非法）
 */
bool BspUart_Send(UART_HandleTypeDef *huart, const uint8_t *data, uint16_t len, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif /* XY7320_BSP_UART_H */