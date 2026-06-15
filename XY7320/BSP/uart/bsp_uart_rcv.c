//
// Created by XYKJ on 2026/6/15.
//

#include "bsp_uart_rcv.h"
#include <string.h>

/**
 * 串口 DMA 接收模块
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

// 接收缓冲区由调用方提供（在 update_service 中分配）
// DMA 会直接往这个缓冲区写数据
static uint8_t *s_rx_buf = NULL;
static uint16_t s_rx_buf_size = 0;

// 内部状态
static uint8_t s_proc_buf[64];          // 帧拷贝缓冲，主循环从这里读数据
static volatile uint16_t s_frame_len = 0;  // 当前帧长度
static volatile bool s_frame_ready = false;  // 帧就绪标志，中断置位，主循环清除
static volatile bool s_overflow = false;     // 溢出标志，上一帧还没取走又来了新帧

static UART_HandleTypeDef *s_huart = NULL;

/**
 * 初始化串口 DMA 接收
 * 
 * @param huart  串口句柄（如 &huart2）
 * @param buf    DMA 接收缓冲区（调用方分配）
 * @param size   缓冲区大小
 */
void Bsp_UartRcv_Init(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t size)
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
void Bsp_UartRcv_Start(void)
{
    s_frame_ready = false;
    s_overflow = false;
    s_frame_len = 0;

    // 启动 DMA 接收，DMA 会自动把串口数据搬到 s_rx_buf
    // 参数：串口句柄、目标缓冲区、最大接收长度
    (void)HAL_UART_Receive_DMA(s_huart, s_rx_buf, s_rx_buf_size);
    
    // 使能 IDLE 中断
    // IDLE 中断：当串口空闲（一段时间没收到新数据）时触发
    // 这表示上位机发完了一帧数据
    __HAL_UART_ENABLE_IT(s_huart, UART_IT_IDLE);
}

/**
 * 检查是否有完整帧到达
 * 
 * @return true  有新帧数据可以读取
 * @return false 没有新数据
 */
bool Bsp_UartRcv_IsFrameReady(void)
{
    return s_frame_ready;
}

/**
 * 获取当前帧长度
 * 
 * @return 帧长度（字节数）
 */
uint16_t Bsp_UartRcv_GetFrameLength(void)
{
    return s_frame_len;
}

/**
 * 将帧数据拷贝到外部缓冲区
 * 
 * 注意：调用前必须先检查 IsFrameReady()
 * 
 * @param dst  目标缓冲区（调用方分配，至少 GetFrameLength() 字节）
 */
void Bsp_UartRcv_CopyFrame(uint8_t *dst)
{
    if (dst != NULL && s_frame_len > 0) {
        // 从内部缓冲 s_proc_buf 拷贝到调用方提供的 dst
        memcpy(dst, s_proc_buf, s_frame_len);
    }
}

/**
 * 清除帧就绪标志
 * 
 * 必须在 CopyFrame() 取走数据后调用
 * 清除后，DMA 可以继续接收下一帧
 */
void Bsp_UartRcv_ClearFlag(void)
{
    s_frame_ready = false;
}

/**
 * 发送应答数据（阻塞）
 * 
 * 使用 HAL_UART_Transmit 发送
 * 注意：在 DMA 接收模式下可能失败，请使用 Bsp_UartRcv_SendAckDirect
 * 
 * @param data  应答数据
 * @param len   数据长度
 */
void Bsp_UartRcv_SendAck(const uint8_t *data, uint16_t len)
{
    if (s_huart != NULL && data != NULL && len > 0) {
        // HAL_UART_Transmit 是阻塞发送，超时 100ms
        (void)HAL_UART_Transmit(s_huart, (uint8_t *)data, len, 100);
    }
}

/**
 * 直接发送应答数据（绕过 HAL 状态机）
 * 
 * 用于 DMA 接收模式下需要发送数据的场景
 * 流程：
 * 1. 停止 DMA 接收
 * 2. 禁用 USART 中断（防止 IDLE 中断干扰发送）
 * 3. 用寄存器直接发送（不经过 HAL 状态机）
 * 4. 等待发送完成
 * 
 * @param data  应答数据
 * @param len   数据长度
 */
void Bsp_UartRcv_SendAckDirect(const uint8_t *data, uint16_t len)
{
    if (s_huart == NULL || data == NULL || len == 0) {
        return;
    }
    
    USART_TypeDef *uart = s_huart->Instance;
    
    // === 第一步：停止 DMA 接收 ===
    // 直接操作寄存器关闭 DMA 流
    if (uart == USART2) {
        DMA1_Stream5->CR &= ~DMA_SxCR_EN;
        while (DMA1_Stream5->CR & DMA_SxCR_EN);  // 等 DMA 真正停下
    }
    
    // === 第二步：禁用 USART 中断 ===
    // 防止发送过程中 IDLE 中断触发干扰
    __HAL_UART_DISABLE_IT(s_huart, UART_IT_IDLE);
    __HAL_UART_DISABLE_IT(s_huart, UART_IT_RXNE);
    
    // === 第三步：寄存器直接发送 ===
    // 不经过 HAL 状态机，直接写数据寄存器
    for (uint16_t i = 0; i < len; i++) {
        // 等待发送寄存器空（TXE = 1 表示可以写下一个字节）
        while (!(uart->SR & USART_SR_TXE));
        // 写入数据寄存器
        uart->DR = data[i];
    }
    
    // === 第四步：等待发送完成 ===
    // TC = 1 表示整个帧已发送完毕（包括停止位）
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
 * 3. 停止 DMA（防止继续写入）
 * 4. 计算接收长度
 * 5. 拷贝数据到内部缓冲
 * 6. 置就绪标志
 * 7. 重启 DMA 接收下一帧
 * 
 * @param huart  串口句柄
 */
void Bsp_UartRcv_HandleIdleIrq(UART_HandleTypeDef *huart)
{
    // 检查串口句柄是否匹配
    if (huart == NULL || huart->Instance != s_huart->Instance) {
        return;
    }

    // 检查是否真的是 IDLE 中断（而不是其他中断如 RXNE、TXE 等）
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == RESET) {
        return;
    }

    // 清除 IDLE 标志（必须手动清除，否则会一直触发）
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    
    // 停止 DMA，防止继续写入缓冲区
    HAL_UART_DMAStop(huart);

    // 计算实际接收长度
    // 原理：DMA 计数器从 s_rx_buf_size 递减到 0
    //       剩余值 = s_rx_buf_size - 已接收长度
    //       所以：已接收长度 = s_rx_buf_size - 剩余值
    uint16_t rx_len = s_rx_buf_size - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    if (rx_len > s_rx_buf_size) {
        rx_len = s_rx_buf_size;  // 防止溢出（理论上不会发生）
    }

    // 如果上一帧还没取走，新帧又来了，标记溢出
    if (!s_frame_ready) {
        // 正常情况：拷贝数据到内部缓冲，置标志
        s_frame_len = rx_len;
        memcpy(s_proc_buf, s_rx_buf, rx_len);  // 从 DMA 缓冲拷贝到内部缓冲
        s_frame_ready = true;
    } else {
        // 溢出情况：主循环还没取走上一帧，新帧又来了
        // 这里简单丢弃新帧，实际项目可以根据需求处理
        s_overflow = true;
    }

    // 重启 DMA 接收，准备接收下一帧
    // 即使溢出了也要重启，否则就收不到后续数据了
    (void)HAL_UART_Receive_DMA(huart, s_rx_buf, s_rx_buf_size);
}

