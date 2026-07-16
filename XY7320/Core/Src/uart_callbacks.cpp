/**
 * @file    uart_callbacks.cpp
 * @brief   UART 用户回调（C++ 实现，避免 stm32f4xx_it.c 引用 C++ 头）
 *
 * 这里集中实现 HAL 的 __weak 用户回调，由 Core/Src/stm32f4xx_it.c 中的
 * 中断入口转发调用。放在独立 .cpp 源里可以避开 stm32f4xx_it.c（C 文件）
 * 不能 include configurations.h / uart_tx_service.h（C++ 头）的问题。
 *
 * 职责：
 *  - HAL_UARTEx_RxEventCallback：USART2 IDLE / 半包切帧，转发给
 *    BspUartRcv_HandleRxEvent，避免 HAL_UART_DMAStop 误关 TX DMA。
 *  - HAL_UART_TxCpltCallback：USART2 TX DMA 完成，转发到
 *    UartTxService::OnTxCompleteFromIsr，仅置完成标志。
 *  - HAL_UART_ErrorCallback：USART2 RX 错误恢复 ReceiveToIdle；仅当存在
 *    活动 TX 且 HAL 报 DMA 错误时，才通知 UartTxService 重置发送状态。
 *
 * 所有回调内只做最小状态恢复或标记，不做协议/编码/排队操作。
 */

#include "usart.h"
#include "bsp_uart_rcv.h"
#include "uart_tx_service.h"

extern "C" {

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if ((huart == NULL) || (huart->Instance == NULL)) {
        return;
    }

    if (huart->Instance == USART2) {
        BspUartRcv_HandleRxEvent(BspUartRcv_GetUpgrade(), huart, size);
    }
    /* 其他 UART（如 USART3）当前仍走手动 IDLE 路径，本回调不处理 */
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart == NULL) || (huart->Instance == NULL)) {
        return;
    }

    if (huart->Instance == USART2) {
        UartTxService::Instance().OnTxCompleteFromIsr();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if ((huart == NULL) || (huart->Instance == NULL)) {
        return;
    }

    if (huart->Instance == USART2) {
        const uint32_t errorCode = static_cast<uint32_t>(huart->ErrorCode);

        /* ORE/NE/FE/PE 属于接收路径；HAL 结束 RX 后在此恢复 ReceiveToIdle。 */
        BspUartRcv_RecoverRxFromError(BspUartRcv_GetUpgrade(), huart);

        /* UART HAL 只有 DMA 错误可表示 TX DMA 失败，且必须确有活动 TX。 */
        if (((errorCode & HAL_UART_ERROR_DMA) != 0U) &&
            UartTxService::Instance().HasActiveTxFromIsr()) {
            UartTxService::Instance().OnTxErrorFromIsr(errorCode);
        }
    }
}

} /* extern "C" */