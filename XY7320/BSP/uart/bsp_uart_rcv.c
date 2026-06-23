/**
 * @file    bsp_uart_rcv.c
 * @brief   串口 DMA 接收模块实现
 *
 * 工作原理：
 * 1. 使用 DMA 自动接收串口数据，不占用 CPU
 * 2. 利用 IDLE 中断检测"一帧数据接收完成"
 *    - 当串口一段时间没收到新数据，触发 IDLE 中断
 *    - 这表示上位机发完了一帧，中间没有断开
 * 3. 在中断中计算接收长度，拷贝到内部缓冲，置标志
 * 4. 主循环轮询检查标志，取走数据进行处理
 *
 * 为什么用双缓冲：
 * - s_rx_buf：DMA 直接写入的缓冲区，中断中会被覆盖
 * - s_proc_buf：内部备份缓冲，主循环可以安全读取
 * - 这样主循环处理数据时，DMA 可以继续接收下一帧
 */

#include "bsp_uart_rcv.h"
#include <string.h>

/* 接收缓冲区由调用方提供（在 UpdateService 中分配），DMA 直接写入 */
static uint8_t *s_rx_buf = NULL;
static uint16_t s_rx_buf_size = 0;

/* 内部状态 */
static uint8_t s_proc_buf[256];               ///< 帧拷贝缓冲，主循环从这里读数据
static volatile uint16_t s_frame_len = 0;    ///< 这次 UART 新收到的原始字节长度
static volatile bool s_frame_ready = false;  ///< 帧就绪标志，有一段新的 UART 原始字节可取
static volatile bool s_overflow = false;     ///< 溢出标志，上一帧还没取走又来了新帧
static UART_HandleTypeDef *s_huart = NULL;

/**
 * 初始化串口 DMA 接收
 *
 * @param huart  串口句柄（如 &huart2）
 * @param buf    DMA 接收缓冲区（调用方分配）
 * @param size   缓冲区大小
 */
void BspUartRcv_Init(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t size)
{
    s_huart = huart;
    s_rx_buf = buf;
    s_rx_buf_size = size;
    s_frame_ready = false;
    s_overflow = false;
    s_frame_len = 0;
}

/**
 * 启动 DMA 接收 + 使能 IDLE 中断
 *
 * 调用后，DMA 会自动把串口数据写入 s_rx_buf
 * 当串口空闲时，触发 IDLE 中断
 */
void BspUartRcv_Start(void)
{
    s_frame_ready = false;
    s_overflow = false;
    s_frame_len = 0;

    /* 启动 DMA 接收，DMA 自动把串口数据搬到 s_rx_buf */
    (void)HAL_UART_Receive_DMA(s_huart, s_rx_buf, s_rx_buf_size);

    /* 使能 IDLE 中断：当串口空闲时触发，表示一帧接收完成 */
    __HAL_UART_ENABLE_IT(s_huart, UART_IT_IDLE);
}

/** 检查是否有完整帧到达 */
bool BspUartRcv_IsFrameReady(void)
{
    return s_frame_ready;
}

/** 获取当前帧长度 */
uint16_t BspUartRcv_GetFrameLength(void)
{
    return s_frame_len;
}

/**
 * 将帧数据拷贝到外部缓冲区
 *
 * @param dst  目标缓冲区（调用方确保容量 >= s_frame_len）
 */
void BspUartRcv_CopyFrame(uint8_t *dst)
{
    if (dst != NULL && s_frame_len > 0) {
        memcpy(dst, s_proc_buf, s_frame_len);
    }
}

/** 清除帧就绪标志，必须在 CopyFrame() 取走数据后调用 */
void BspUartRcv_ClearFlag(void)
{
    s_frame_ready = false;
}

/**
 * 发送应答数据（阻塞）
 *
 * 使用 HAL_UART_Transmit 发送。
 * NOTE: HAL UART 句柄中 gState 管 TX、RxState 管 RX，两者独立，
 *       DMA 接收模式下阻塞发送不会干扰接收，可放心使用。
 */
void BspUartRcv_SendAck(const uint8_t *data, uint16_t len)
{
    if (s_huart != NULL && data != NULL && len > 0) {
        (void)HAL_UART_Transmit(s_huart, (uint8_t *)data, len, 100);
    }
}

/**
 * 直接发送应答数据（绕过 HAL 状态机）
 *
 * 绕过 HAL 直接用寄存器发送。当前 HAL 版本下 SendAck 已可正常工作，
 * 此函数保留作为备用方案。
 *
 * DEPRECATED: 优先使用 BspUartRcv_SendAck()，仅在 HAL 状态机异常时使用本函数。
 * FIXME: SendAckDirect 硬编码了 DMA1_Stream5，只支持 USART2
 */
void BspUartRcv_SendAckDirect(const uint8_t *data, uint16_t len)
{
    if (s_huart == NULL || data == NULL || len == 0) {
        return;
    }

    USART_TypeDef *uart = s_huart->Instance;

    /* 第一步：停止 DMA 接收（避免寄存器操作与 DMA 传输竞争） */
    if (uart == USART2) {
        DMA1_Stream5->CR &= ~DMA_SxCR_EN;
        while (DMA1_Stream5->CR & DMA_SxCR_EN);
    }

    /* 第二步：禁用 USART 中断，防止发送过程中 IDLE 中断触发干扰 */
    __HAL_UART_DISABLE_IT(s_huart, UART_IT_IDLE);
    __HAL_UART_DISABLE_IT(s_huart, UART_IT_RXNE);

    /* 第三步：寄存器直接发送（不经过 HAL 状态机） */
    for (uint16_t i = 0; i < len; i++) {
        while (!(uart->SR & USART_SR_TXE));  /* 等待发送寄存器空 */
        uart->DR = data[i];
    }

    /* 第四步：等待发送完成（TC = 1 表示包括停止位都已发出） */
    while (!(uart->SR & USART_SR_TC));
}

/**
 * USART IDLE 中断处理入口
 *
 * 在 stm32f4xx_it.c 的 USART2_IRQHandler 中调用
 *
 * 工作流程：
 * 1. 检查是否是 IDLE 中断
 * 2. 清除 IDLE 标志
 * 3. 停止 DMA
 * 4. 计算接收长度
 * 5. 拷贝数据到内部缓冲
 * 6. 置就绪标志
 * 7. 重启 DMA 接收下一帧
 */
void BspUartRcv_HandleIdleIrq(UART_HandleTypeDef *huart)
{
    if (huart == NULL || s_huart == NULL || huart->Instance != s_huart->Instance) {
        return;
    }

    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == RESET) {
        return;
    }

    __HAL_UART_CLEAR_IDLEFLAG(huart);
    HAL_UART_DMAStop(huart);

    /*
     * 计算实际接收长度
     * 原理：DMA 计数器从 s_rx_buf_size 递减到 0
     *       剩余值 = s_rx_buf_size - 已接收长度
     *       所以：已接收长度 = s_rx_buf_size - 剩余值
     */
    uint16_t rx_len = s_rx_buf_size - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    if (rx_len > s_rx_buf_size) {
        rx_len = s_rx_buf_size;
    }

    /* TODO: 增加帧长度越界检查，当前 s_proc_buf 固定 256 字节 */
    if (!s_frame_ready) {
        if (rx_len <= sizeof(s_proc_buf)) {
            s_frame_len = rx_len;
            memcpy(s_proc_buf, s_rx_buf, rx_len);
            s_frame_ready = true;
        }
    } else {
        s_overflow = true;
    }

    /* 重启 DMA 接收下一帧 */
    (void)HAL_UART_Receive_DMA(huart, s_rx_buf, s_rx_buf_size);
}
