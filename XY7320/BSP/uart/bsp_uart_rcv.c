/**
 * @file    bsp_uart_rcv.c
 * @brief   可实例化 UART DMA + IDLE 接收 BSP 模块实现
 *
 * 工作原理：
 * 1. 每一路 UART 使用独立 BspUartRcv_t 上下文保存句柄、DMA 缓冲和状态。
 * 2. DMA 自动接收串口数据，不占用逐字节中断。
 * 3. UART IDLE 中断表示当前串口出现空闲间隔，BSP 将这一段原始字节复制到处理缓冲。
 * 4. 上层服务周期性取走 chunk，并按自己的协议继续拆帧。
 *
 * 注意：IDLE chunk 不等价于完整业务帧。NMEA、主协议等流式协议应在 Service/Domain 层继续拼流解析。
 */

#include "bsp_uart_rcv.h"
#include <string.h>

/* BSP 层只保存 UART 接收实例，不理解升级协议或 GNSS 协议语义。
 * 每个实例独立绑定 UART/DMA/缓冲区，避免多串口复用时互相覆盖状态。 */
static BspUartRcv_t s_upgradeUartRcv;
static BspUartRcv_t s_gnssUartRcv;

BspUartRcv_t *BspUartRcv_GetUpgrade(void)
{
    return &s_upgradeUartRcv;
}

BspUartRcv_t *BspUartRcv_GetGnss(void)
{
    return &s_gnssUartRcv;
}

void BspUartRcv_Init(BspUartRcv_t *ctx,
                     UART_HandleTypeDef *huart,
                     uint8_t *dmaBuf,
                     uint16_t dmaBufSize,
                     uint8_t *procBuf,
                     uint16_t procBufSize)
{
    if (ctx == NULL) {
        return;
    }

    ctx->huart = huart;
    ctx->dmaBuf = dmaBuf;
    ctx->dmaBufSize = dmaBufSize;
    ctx->procBuf = procBuf;
    ctx->procBufSize = procBufSize;
    ctx->frameReady = false;
    ctx->overflow = false;
    ctx->frameLen = 0;
}

void BspUartRcv_Start(BspUartRcv_t *ctx)
{
    HAL_StatusTypeDef status;

    if (ctx == NULL || ctx->huart == NULL || ctx->dmaBuf == NULL || ctx->dmaBufSize == 0U) {
        return;
    }

    ctx->frameReady = false;
    ctx->overflow = false;
    ctx->frameLen = 0U;

    if (ctx->huart->Instance == USART2) {
        /*
         * USART2 使用 HAL 的 ReceiveToIdle DMA。Normal DMA 在 IDLE/TC 后
         * 会结束本次接收，因此 RxEvent 回调必须显式重新启动。
         * 禁用 HT 中断后，每次只处理 IDLE 或 TC，避免把累计 position
         * 重复当作新增长度搬运。
         */
        status = HAL_UARTEx_ReceiveToIdle_DMA(ctx->huart, ctx->dmaBuf, ctx->dmaBufSize);
        if (status == HAL_OK && ctx->huart->hdmarx != NULL) {
            __HAL_DMA_DISABLE_IT(ctx->huart->hdmarx, DMA_IT_HT);
        }
    } else {
        /* USART3 GNSS 保持原手动 IDLE 路径。 */
        status = HAL_UART_Receive_DMA(ctx->huart, ctx->dmaBuf, ctx->dmaBufSize);
        if (status == HAL_OK) {
            __HAL_UART_ENABLE_IT(ctx->huart, UART_IT_IDLE);
        }
    }

    if (status != HAL_OK) {
        ctx->overflow = true;
    }
}

void BspUartRcv_Stop(BspUartRcv_t *ctx)
{
    if (ctx == NULL || ctx->huart == NULL) {
        return;
    }

    /* HAL_UART_AbortReceive 只终止 RX，并恢复 RxState/ReceptionType；
     * 不使用 HAL_UART_DMAStop，避免误停同一 UART 的 TX DMA。 */
    (void)HAL_UART_AbortReceive(ctx->huart);

    ctx->frameReady = false;
    ctx->overflow = false;
    ctx->frameLen = 0U;
}

void BspUartRcv_DeInit(BspUartRcv_t *ctx)
{
    if (ctx == NULL || ctx->huart == NULL) {
        return;
    }

    /* 升级链路会在确认 TX 已排空后进入这里。先只终止 RX，再释放 UART。 */
    (void)HAL_UART_AbortReceive(ctx->huart);
    HAL_UART_DeInit(ctx->huart);

    ctx->huart = NULL;
    ctx->dmaBuf = NULL;
    ctx->dmaBufSize = 0;
    ctx->procBuf = NULL;
    ctx->procBufSize = 0;
    ctx->frameReady = false;
    ctx->overflow = false;
    ctx->frameLen = 0;
}

bool BspUartRcv_IsFrameReady(const BspUartRcv_t *ctx)
{
    return (ctx != NULL) && ctx->frameReady;
}

uint16_t BspUartRcv_GetFrameLength(const BspUartRcv_t *ctx)
{
    return (ctx != NULL) ? ctx->frameLen : 0U;
}

void BspUartRcv_CopyFrame(const BspUartRcv_t *ctx, uint8_t *dst)
{
    if (ctx != NULL && dst != NULL && ctx->procBuf != NULL && ctx->frameLen > 0U) {
        memcpy(dst, ctx->procBuf, ctx->frameLen);
    }
}

void BspUartRcv_ClearFlag(BspUartRcv_t *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->frameReady = false;
    ctx->frameLen = 0;
}

bool BspUartRcv_TakeOverflow(BspUartRcv_t *ctx)
{
    if (ctx == NULL) {
        return false;
    }

    const bool overflow = ctx->overflow;
    ctx->overflow = false;
    return overflow;
}

void BspUartRcv_SendAck(BspUartRcv_t *ctx, const uint8_t *data, uint16_t len)
{
    if (ctx != NULL && ctx->huart != NULL && data != NULL && len > 0U) {
        (void)HAL_UART_Transmit(ctx->huart, (uint8_t *)data, len, 100);
    }
}

/**
 * @brief 绕过 HAL 直接寄存器发送
 *
 * @note  设计意图：当升级主协议确认 STM32 HAL 状态机异常（例如 DMA busy 卡死），
 *        必须把当前应答送达上位机时使用。函数内会停 DMA、关 IDLE/RXNE，避免与
 *        正在发送的字节产生竞争，然后轮询 TXE/TC 写入 DR。
 * @warning 使用前必须评估对当前接收流程的影响：发送期间接收将暂停，发送完成后
 *          由调用方负责重启 DMA 接收和 IDLE 中断。
 */
void BspUartRcv_SendAckDirect(BspUartRcv_t *ctx, const uint8_t *data, uint16_t len)
{
    if (ctx == NULL || ctx->huart == NULL || data == NULL || len == 0U) {
        return;
    }

    USART_TypeDef *uart = ctx->huart->Instance;

    if (ctx->huart->hdmarx != NULL && ctx->huart->hdmarx->Instance != NULL) {
        DMA_Stream_TypeDef *stream = ctx->huart->hdmarx->Instance;
        // 直接关闭 RX DMA，避免在我们轮询 TX 期间 DMA 还在抢总线/缓冲区
        stream->CR &= ~DMA_SxCR_EN;
        while (stream->CR & DMA_SxCR_EN) {
            // 等 DMA 控制器真正停止，避免后续访问寄存器时序冲突
        }
    }

    // 关掉 IDLE 和 RXNE：发送期间不希望被接收事件打断寄存器发送流程，
    // 也防止 TX 期间产生的 RXNE 被当成下一帧起始
    __HAL_UART_DISABLE_IT(ctx->huart, UART_IT_IDLE);
    __HAL_UART_DISABLE_IT(ctx->huart, UART_IT_RXNE);

    for (uint16_t i = 0; i < len; i++) {
        while ((uart->SR & USART_SR_TXE) == 0U) {
            // TXE=1 表示数据寄存器空，可以写入下一个字节；轮询保证背靠背发送
        }
        uart->DR = data[i];
    }

    // TC=1 才说明最后一个字节真的从移位寄存器发出去了，
    // 否则上层可能误判发送完成，导致后续操作时序错位
    while ((uart->SR & USART_SR_TC) == 0U) {
    }
}

/**
 * @brief UART IDLE 中断处理入口（手动 IDLE 兼容路径，仅 USART3 仍使用）
 *
 * @warning USART2 已迁移到 HAL_UARTEx_ReceiveToIdle_DMA，不再调用本函数，
 *          而是由 HAL_UARTEx_RxEventCallback -> BspUartRcv_HandleRxEvent 处理。
 *          仅在 USART3（未配 ReceiveToIdle DMA）等场景下保留。
 *
 *          仅允许在 UART 中断服务函数里调用：
 *          - 调用顺序必须与 USARTx_IRQHandler 中 ISR 路由一致；
 *          - huart 必须与 ctx 绑定同一 UART 实例；
 *          - 函数内会停 DMA 再立即重启，不要在中断中插入日志或解析；
 *          - procBuf 被占用且数据未取走时，新到达的 chunk 会置 overflow。
 */
void BspUartRcv_HandleIdleIrq(BspUartRcv_t *ctx, UART_HandleTypeDef *huart)
{
    if (ctx == NULL || huart == NULL || ctx->huart == NULL || huart->Instance != ctx->huart->Instance) {
        // huart 不匹配说明 ISR 路由写错或 ctx 没绑同一路串口，
        // 直接退出避免错搬其他串口的数据
        return;
    }

    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == RESET) {
        // 不是本路 UART 触发的 IDLE，防止共享 ISR 误触发
        return;
    }

    __HAL_UART_CLEAR_IDLEFLAG(huart);
    /*
     * 兼容路径：USART3 仍走 HAL_UART_Receive_DMA + 手动 IDLE，
     * HAL_UART_DMAStop 只关闭 RX DMA（USART3 未配置 TX DMA，无副作用）。
     */
    if (huart->hdmarx != NULL && huart->hdmarx->Instance != NULL) {
        DMA_Stream_TypeDef *rx_stream = huart->hdmarx->Instance;
        rx_stream->CR &= ~DMA_SxCR_EN;
        while ((rx_stream->CR & DMA_SxCR_EN) != 0U) {
            /* 等 DMA 控制器真正停止 */
        }
    }

    if (ctx->dmaBuf == NULL || ctx->procBuf == NULL || ctx->dmaBufSize == 0U || huart->hdmarx == NULL) {
        ctx->overflow = true;
        return;
    }

    uint16_t rxLen = ctx->dmaBufSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    if (rxLen > ctx->dmaBufSize) {
        // 防御性裁剪：NDTR 在异常情况下可能读出比 size 大的值，避免反转
        rxLen = ctx->dmaBufSize;
    }

    if (!ctx->frameReady) {
        if (rxLen <= ctx->procBufSize) {
            ctx->frameLen = rxLen;
            if (rxLen > 0U) {
                // 仅在收到非空数据时复制，避免 0 长度 memcpy 留下脏数据
                memcpy(ctx->procBuf, ctx->dmaBuf, rxLen);
            }
            ctx->frameReady = (rxLen > 0U);
        } else {
            // 处理缓冲放不下当前 chunk，置 overflow 等待上层处理
            ctx->overflow = true;
        }
    } else {
        // 上一次 chunk 还没被取走，新数据到达只能丢弃并记 overflow
        ctx->overflow = true;
    }

    // 立刻重启 DMA，保持连续接收；这里不需要再次开启 IDLE 中断，
    // 因为 USART 的 IDLE 中断使能位在停 DMA 过程中不会被清除
    (void)HAL_UART_Receive_DMA(huart, ctx->dmaBuf, ctx->dmaBufSize);
}

/**
 * @brief UART IDLE / 半包切帧处理入口（HAL_UARTEx_RxEventCallback 调用）
 *
 * ReceiveToIdle DMA 路径：HAL 在 IDLE/完成时会回调
 * HAL_UARTEx_RxEventCallback(huart, size)，由调用方转发到这里。
 * 与 HandleIdleIrq 的区别：本函数不调用 HAL_UART_DMAStop；Normal DMA
 * 回调后由本函数显式重启 ReceiveToIdle DMA，因此可以与 TX DMA 并存。
 *
 * @param  ctx    接收实例上下文
 * @param  huart  当前 IRQ 对应的 UART 句柄
 * @param  size   HAL 给出的本次接收字节数（dmaBuf 前 size 字节）
 */
void BspUartRcv_HandleRxEvent(BspUartRcv_t *ctx,
                              UART_HandleTypeDef *huart,
                              uint16_t size)
{
    if (ctx == NULL || huart == NULL || ctx->huart == NULL || huart->Instance != ctx->huart->Instance) {
        return;
    }

    if (ctx->dmaBuf == NULL || ctx->procBuf == NULL || ctx->dmaBufSize == 0U) {
        ctx->overflow = true;
        return;
    }

    if (size > ctx->dmaBufSize) {
        size = ctx->dmaBufSize;
    }

    if (!ctx->frameReady) {
        if (size <= ctx->procBufSize) {
            ctx->frameLen = size;
            if (size > 0U) {
                memcpy(ctx->procBuf, ctx->dmaBuf, size);
            }
            ctx->frameReady = (size > 0U);
        } else {
            ctx->overflow = true;
        }
    } else {
        ctx->overflow = true;
    }

    /*
     * 当前 USART2 RX DMA 为 Normal 模式。HAL 在 IDLE/TC 回调前已经结束
     * 本次接收并恢复 RxState，因此必须显式重新启动 ReceiveToIdle DMA。
     * 这里不调用 HAL_UART_DMAStop，不会影响同一 UART 的 TX DMA。
     */
    if (HAL_UARTEx_ReceiveToIdle_DMA(huart, ctx->dmaBuf, ctx->dmaBufSize) == HAL_OK) {
        if (huart->hdmarx != NULL) {
            __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        }
    } else {
        ctx->overflow = true;
    }
}

void BspUartRcv_RecoverRxFromError(BspUartRcv_t *ctx,
                                   UART_HandleTypeDef *huart)
{
    if (ctx == NULL || huart == NULL || ctx->huart == NULL ||
        huart->Instance != USART2 || huart->Instance != ctx->huart->Instance ||
        ctx->dmaBuf == NULL || ctx->dmaBufSize == 0U) {
        return;
    }

    /* 非阻塞噪声错误不会结束接收，此时不能重复启动同一 RX DMA。 */
    if (huart->RxState == HAL_UART_STATE_BUSY_RX) {
        return;
    }

    if (HAL_UARTEx_ReceiveToIdle_DMA(huart, ctx->dmaBuf, ctx->dmaBufSize) == HAL_OK) {
        if (huart->hdmarx != NULL) {
            __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        }
    } else {
        ctx->overflow = true;
    }
}
