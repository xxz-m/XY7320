/**
 * @file    bsp_uart_rcv.h
 * @brief   可实例化 UART DMA + IDLE 接收 BSP 模块
 *
 *          本模块只负责把 UART DMA 接收到的一段原始字节安全搬运到处理缓冲，
 *          不理解上层协议语义。每一路 UART 需要独立的 BspUartRcv_t 上下文，
 *          避免多个串口复用时互相覆盖状态。
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
 * @brief  UART DMA + IDLE 接收实例上下文
 *
 * @note   调用方负责提供 dmaBuf/procBuf 的静态存储空间，BSP 只保存指针。
 *         dmaBuf 由 DMA 写入；procBuf 保存最近一次 IDLE 分段后的数据副本。
 */
typedef struct {
    UART_HandleTypeDef *huart;        /**< 绑定的 UART 句柄 */
    uint8_t *dmaBuf;                  /**< DMA 接收缓冲 */
    uint16_t dmaBufSize;              /**< DMA 接收缓冲大小 */
    uint8_t *procBuf;                 /**< 处理缓冲，主循环从这里复制数据 */
    uint16_t procBufSize;             /**< 处理缓冲大小 */
    volatile uint16_t frameLen;       /**< 当前可取数据长度 */
    volatile bool frameReady;         /**< 数据就绪标志 */
    volatile bool overflow;           /**< 上一段未取走时新数据到达 */
} BspUartRcv_t;

/** 获取升级串口接收实例（当前绑定 USART2）。 */
BspUartRcv_t *BspUartRcv_GetUpgrade(void);

/** 获取 GNSS/传感器串口接收实例（预留给 USART3）。 */
BspUartRcv_t *BspUartRcv_GetGnss(void);

/**
 * @brief  初始化 UART DMA 接收实例
 * @param  ctx          接收实例上下文
 * @param  huart        UART 句柄（由 CubeMX 生成，如 &huart2）
 * @param  dmaBuf       DMA 接收缓冲区（调用方分配）
 * @param  dmaBufSize   DMA 接收缓冲区大小
 * @param  procBuf      处理缓冲区（调用方分配）
 * @param  procBufSize  处理缓冲区大小
 */
void BspUartRcv_Init(BspUartRcv_t *ctx,
                     UART_HandleTypeDef *huart,
                     uint8_t *dmaBuf,
                     uint16_t dmaBufSize,
                     uint8_t *procBuf,
                     uint16_t procBufSize);

/**
 * @brief 启动指定实例的 DMA 接收和 IDLE 中断。
 * @param ctx 接收实例上下文，必须已由 BspUartRcv_Init() 配置。
 */
void BspUartRcv_Start(BspUartRcv_t *ctx);
/**
 * @brief 停止指定实例的 DMA 接收和 IDLE 中断
 *
 * 保留 UART、DMA 缓冲区和实例配置，允许后续再次调用
 * BspUartRcv_Start() 重新启动接收。
 *
 * @param ctx UART 接收实例
 */
void BspUartRcv_Stop(BspUartRcv_t *ctx);
/**
 * @brief 释放指定实例的 DMA 接收配置。
 *
 * 除停止 DMA 和 IDLE 中断外，还会清空 UART、缓冲区和长度配置；
 * 后续重新使用前必须再次调用 BspUartRcv_Init()。
 *
 * @param ctx 接收实例上下文。
 */
void BspUartRcv_DeInit(BspUartRcv_t *ctx);

/** 指定实例是否有数据段到达。 */
bool BspUartRcv_IsFrameReady(const BspUartRcv_t *ctx);

/** 获取指定实例当前数据段长度。 */
uint16_t BspUartRcv_GetFrameLength(const BspUartRcv_t *ctx);

/**
 * @brief  将指定实例的数据段拷贝到外部缓冲区
 * @param  ctx  接收实例上下文
 * @param  dst  目标缓冲区（调用方确保容量 >= frameLen）
 */
void BspUartRcv_CopyFrame(const BspUartRcv_t *ctx, uint8_t *dst);

/** 清除指定实例的数据就绪标志（必须在取走数据后调用）。 */
void BspUartRcv_ClearFlag(BspUartRcv_t *ctx);

/** 查询并清除溢出状态。 */
bool BspUartRcv_TakeOverflow(BspUartRcv_t *ctx);

/**
 * @brief  发送应答数据（阻塞，HAL_UART_Transmit 100ms 超时）
 *
 * @warning 业务路径必须改用 Services/uart_service/UartTxService::EnqueueControl。
 *          本函数为阻塞发送，会卡住调用任务，并直接操作 HAL UART 状态机；
 *          一旦与 UartTxService 的异步 TX DMA 并存会破坏 HAL 状态机。
 *          仅保留作为 Bootloader 升级链路以及单元测试的兜底通道。
 *
 * @param  ctx   接收实例上下文，用于确定发送 UART
 * @param  data  应答数据
 * @param  len   数据长度
 */
void BspUartRcv_SendAck(BspUartRcv_t *ctx, const uint8_t *data, uint16_t len);

/**
 * @brief  直接发送应答（绕过 HAL 状态机）
 *
 * 用于 DMA 接收模式下 HAL 状态机异常的兜底场景。内部会停止 DMA、
 * 禁用 IDLE/RXNE 中断，然后用寄存器直接发送。
 *
 * @param  ctx   接收实例上下文，用于确定发送 UART
 * @param  data  应答数据
 * @param  len   数据长度
 */
void BspUartRcv_SendAckDirect(BspUartRcv_t *ctx, const uint8_t *data, uint16_t len);

/**
 * @brief  UART IDLE 中断处理入口（手动 IDLE 兼容路径，仅 USART3 仍使用）
 *
 * @warning 仅在 USART3 等未迁移到 ReceiveToIdle DMA 的串口上调用。
 *          USART2 已切到 HAL_UARTEx_ReceiveToIdle_DMA，IDLE 切帧走
 *          HAL_UARTEx_RxEventCallback -> BspUartRcv_HandleRxEvent 路径，
 *          本入口对 USART2 不再调用，避免误关 TX DMA。
 *
 *          函数内会调用 HAL_UART_DMAStop 并立刻重启 DMA，不要在任务上下文调用。
 *          不要在本函数内执行日志、协议解析或任何阻塞操作。
 *
 * @param  ctx    接收实例上下文
 * @param  huart  当前 IRQ 对应的 UART 句柄
 */
void BspUartRcv_HandleIdleIrq(BspUartRcv_t *ctx, UART_HandleTypeDef *huart);

/**
 * @brief  UART IDLE / DMA 完成切帧入口（HAL_UARTEx_RxEventCallback 调用）
 *
 * @note   ReceiveToIdle DMA 路径：HAL 在 IDLE/完成时会回调
 *         HAL_UARTEx_RxEventCallback(huart, size)，由调用方转发到这里。
 *         USART2 当前由这条路径接收数据，避免手动 HAL_UART_DMAStop
 *         误关 TX DMA。
 *
 *         USART2 使用 Normal DMA，函数搬运当前 chunk 后会显式重新调用
 *         HAL_UARTEx_ReceiveToIdle_DMA，并关闭 HT 中断，只处理 IDLE/TC。
 *         不要在任务上下文调用，也不要在本函数内执行日志、协议解析或阻塞操作。
 *
 * @param  ctx    接收实例上下文
 * @param  huart  当前 IRQ 对应的 UART 句柄
 * @param  size   HAL 给出的本次接收字节数（已在 dmaBuf 前 size 字节处）
 */
void BspUartRcv_HandleRxEvent(BspUartRcv_t *ctx,
                              UART_HandleTypeDef *huart,
                              uint16_t size);

/**
 * @brief  USART2 错误回调后的 ReceiveToIdle DMA 恢复入口
 *
 * 仅在 HAL 已结束当前 RX 传输时重新启动，不清除尚未被上层取走的 procBuf。
 * 若 RX 仍在运行则保持现状；启动失败会记录 overflow。
 *
 * @param  ctx    接收实例上下文
 * @param  huart  发生错误的 UART 句柄
 */
void BspUartRcv_RecoverRxFromError(BspUartRcv_t *ctx,
                                   UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_BSP_UART_RCV_H */
