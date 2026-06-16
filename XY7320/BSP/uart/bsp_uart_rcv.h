/**
 * @file    bsp_uart_rcv.h
 * @brief   串口 DMA 接收模块
 *          基于 DMA + IDLE 中断实现不定长帧接收，双缓冲机制
 */

#ifndef XY7320_BSP_UART_RCV_H
#define XY7320_BSP_UART_RCV_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  初始化串口 DMA 接收
 * @param  huart  串口句柄（由 CubeMX 生成，如 &huart2）
 * @param  buf    DMA 接收缓冲区（调用方分配）
 * @param  size   缓冲区大小（字节）
 */
void BspUartRcv_Init(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t size);

/** 启动 DMA 接收 + 使能 IDLE 中断 */
void BspUartRcv_Start(void);

/** 是否有完整帧到达 */
bool BspUartRcv_IsFrameReady(void);

/** 获取当前帧长度 */
uint16_t BspUartRcv_GetFrameLength(void);

/**
 * @brief  将帧数据拷贝到外部缓冲区
 * @param  dst  目标缓冲区（调用方确保容量 >= 帧长度）
 */
void BspUartRcv_CopyFrame(uint8_t *dst);

/** 清除帧就绪标志（必须在取走帧数据后调用） */
void BspUartRcv_ClearFlag(void);

/**
 * @brief  发送应答数据（阻塞）
 * @param  data  应答数据
 * @param  len   数据长度
 */
void BspUartRcv_SendAck(const uint8_t *data, uint16_t len);

/**
 * @brief  直接发送应答（绕过 HAL 状态机）
 *
 * 用于 DMA 接收模式下需要发送数据的场景。
 * 内部会停止 DMA、禁用中断，然后用寄存器直接发送。
 *
 * @param  data  应答数据
 * @param  len   数据长度
 */
void BspUartRcv_SendAckDirect(const uint8_t *data, uint16_t len);

/**
 * USART IDLE 中断处理入口
 * 在 stm32f4xx_it.c 的 USART2_IRQHandler 中调用
 */
void BspUartRcv_HandleIdleIrq(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_BSP_UART_RCV_H */
