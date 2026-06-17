/**
* @file    bsp_uart.c
 * @brief   串口通用收发实现
 */
#include "bsp_uart.h"
#include <string.h>

/** 日志输出专用串口句柄 */
static UART_HandleTypeDef *s_logUart = NULL;
/**
 * 检测串口是否已初始化
 *
 * 检查条件：
 * 1. 句柄指针非空
 * 2. 串口实例非空（如 USART1 != NULL）
 * 3. gState 为 HAL_UART_STATE_READY（说明 HAL_UART_Init 成功执行过）
 */
static bool IsUartReady(UART_HandleTypeDef *huart)
{
    if (huart == NULL) return false;
    if (huart->Instance == NULL) return false;
    if (huart->gState != HAL_UART_STATE_READY) return false;
    return true;
}
/**
 * @brief  初始化日志输出串口
 * @param  huart  用于日志输出的串口句柄（如 &huart1）
 * @note   内部会检测串口是否已由 HAL 初始化，未初始化则拒绝配置
 *         调用后，BspUart_LogPutChar() 会使用这个串口
 */
void BspUart_LogInit(UART_HandleTypeDef *huart)
{
    /* 初始化前检查：串口必须已经由 CubeMX/HAL 初始化过 */
    if (!IsUartReady(huart)) {
        return;  /* 串口未就绪，拒绝配置 */
    }
    s_logUart = huart;
}
/** 日志输出：发送单个字符（需先调用 BspUart_LogInit） */
void BspUart_LogPutChar(char ch)
{
    /* 每次发送都检查：防止运行中串口被 DeInit */
    if (!IsUartReady(s_logUart)) return;

    uint8_t data = (uint8_t)ch;
    HAL_UART_Transmit(s_logUart, &data, 1, 100);
}
/** 日志输出：发送字符串 */
void BspUart_LogWrite(const char *str)
{
    if (!IsUartReady(s_logUart) || str == NULL) return;

    while (*str != '\0') {
        BspUart_LogPutChar(*str++);
    }
}
/**
 * @brief  发送数据（通用接口）
 * @param  huart   串口句柄
 * @param  data    待发送数据
 * @param  len     数据长度
 * @param  timeout 超时时间（ms）
 * @return true 成功, false 失败（串口未就绪或超时）
 */
bool BspUart_Send(UART_HandleTypeDef *huart, const uint8_t *data, uint16_t len, uint32_t timeout)
{
    if (!IsUartReady(huart) || data == NULL || len == 0) {
        return false;
    }
    return HAL_UART_Transmit(huart, (uint8_t *)data, len, timeout) == HAL_OK;
}
