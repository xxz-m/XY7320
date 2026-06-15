//
// Created by XYKJ on 2026/6/15.
//

#ifndef XY7320_BSP_UART_RCV_H
#define XY7320_BSP_UART_RCV_H
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化串口 DMA 接收
 */
void Bsp_UartRcv_Init(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t size);

/**
 * 启动 DMA 接收 + 使能 IDLE 中断
 */
void Bsp_UartRcv_Start(void);

/**
 * 是否有完整帧到达（IDLE 触发）
 */
bool Bsp_UartRcv_IsFrameReady(void);

/**
 * 获取当前帧长度
 */
uint16_t Bsp_UartRcv_GetFrameLength(void);

/**
 * 将帧数据拷贝到外部缓冲区
 */
void Bsp_UartRcv_CopyFrame(uint8_t *dst);

/**
 * 清除帧就绪标志（必须在取走帧数据后调用）
 */
void Bsp_UartRcv_ClearFlag(void);

/**
 * 发送应答数据（阻塞）
 */
void Bsp_UartRcv_SendAck(const uint8_t *data, uint16_t len);

/**
 * 直接发送应答数据（绕过 HAL 状态机）
 * 
 * 用于 DMA 接收模式下需要发送数据的场景
 * 内部会停止 DMA、禁用中断，然后用寄存器直接发送
 * 
 * @param data  应答数据
 * @param len   数据长度
 */
void Bsp_UartRcv_SendAckDirect(const uint8_t *data, uint16_t len);

/**
 * USART IDLE 中断处理入口
 * 在 stm32f4xx_it.c 的 USART2_IRQHandler 中调用
 */
void Bsp_UartRcv_HandleIdleIrq(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif
#endif //XY7320_BSP_UART_RCV_H
