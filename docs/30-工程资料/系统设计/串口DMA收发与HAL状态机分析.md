---
title: 串口 DMA 收发机制与 HAL 状态机分析
tags:
  - docs/learning
  - uart
  - dma
  - hal
status: active
---

# 串口 DMA 收发机制与 HAL 状态机分析

本文以 XY7320 升级模块的串口收发为案例，讲解 DMA + IDLE 中断接收不定长帧的工作原理，以及 HAL_UART_Transmit 在 DMA 接收模式下能否正常发送的问题。

---

## 1. 串口接收方案选型

### 为什么选 DMA + IDLE 中断

| 方案 | 优点 | 缺点 |
|------|------|------|
| 轮询读 DR | 简单 | CPU 一直在等串口，浪费算力 |
| RXNE 中断 | 不阻塞 CPU | 每收一个字节就进一次中断，高波特率下开销大 |
| **DMA + IDLE** | DMA 自动搬运，CPU 零参与；IDLE 中断按帧通知 | 需要理解 DMA 和 IDLE 机制 |

DMA + IDLE 的方案：DMA 控制器自动把串口收到的字节搬到内存里，CPU 完全不参与搬运。当串口总线"安静"了一段时间（没有新字节到来），硬件触发 IDLE 中断。在中断里一次性处理整帧数据。

---

## 2. 初始化流程

### BspUartRcv_Init + BspUartRcv_Start

```c
void BspUartRcv_Init(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t size)
{
    s_huart = huart;       // 记住用哪个串口
    s_rx_buf = buf;        // DMA 直接往这个缓冲区写
    s_rx_buf_size = size;  // 缓冲区上限
}

void BspUartRcv_Start(void)
{
    HAL_UART_Receive_DMA(s_huart, s_rx_buf, s_rx_buf_size);
    __HAL_UART_ENABLE_IT(s_huart, UART_IT_IDLE);
}
```

`HAL_UART_Receive_DMA` 做的事：配置 DMA 流（源地址 = UART 数据寄存器，目标地址 = `s_rx_buf`，传输数量 = `s_rx_buf_size`），然后使能 DMA 通道。之后每收到一个字节，DMA 硬件自动把 `USART2->DR` 里的值拷贝到 `s_rx_buf[当前偏移]`，偏移自动递增。

注意：`s_rx_buf` 是在 `UpdateService` 里定义的成员变量 `m_rxBuf[64]`，通过 `Init()` 传给 BSP 层。BSP 层用指针引用它，不拥有这块内存。"调用方分配缓冲"在嵌入式里很常见，因为内存布局由上层决定更灵活。

---

## 3. IDLE 中断处理

```c
void BspUartRcv_HandleIdleIrq(UART_HandleTypeDef *huart)
{
    // 第一步：确认是 IDLE 中断
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == RESET)
        return;
    __HAL_UART_CLEAR_IDLEFLAG(huart);

    // 第二步：停 DMA
    HAL_UART_DMAStop(huart);

    // 第三步：算接收了多少字节
    // DMA 计数器从 s_rx_buf_size 往下递减
    // 已接收 = 总量 - 剩余
    uint16_t rx_len = s_rx_buf_size - __HAL_DMA_GET_COUNTER(huart->hdmarx);

    // 第四步：拷贝到安全缓冲 + 置标志
    if (!s_frame_ready) {
        if (rx_len <= sizeof(s_proc_buf)) {
            s_frame_len = rx_len;
            memcpy(s_proc_buf, s_rx_buf, rx_len);
            s_frame_ready = true;
        }
    }

    // 第五步：重启 DMA
    HAL_UART_Receive_DMA(huart, s_rx_buf, s_rx_buf_size);
}
```

### 为什么要双缓冲

`s_rx_buf` 是 DMA 直接写入的区域，重启 DMA 后随时可能被新数据覆盖。拷贝到 `s_proc_buf` 之后，主循环可以慢慢处理，不用担心数据被踩掉。

### volatile 的意义

`s_frame_ready`、`s_frame_len` 这些变量在中断里写、在主循环里读。`volatile` 告诉编译器"不要优化掉对这个变量的读取"，否则编译器可能把标志值缓存在寄存器里，导致主循环永远看不到中断的更新。

---

## 4. 发送：阻塞式 vs 寄存器直写

### 两种发送方式

项目中 BSP 层提供了两个发送函数：

```c
// 方式一：HAL 阻塞发送
void BspUartRcv_SendAck(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(s_huart, (uint8_t *)data, len, 100);
}

// 方式二：寄存器直接发送（绕过 HAL）
void BspUartRcv_SendAckDirect(const uint8_t *data, uint16_t len)
{
    // 停 DMA → 禁中断 → 写 DR 寄存器 → 等 TC
}
```

最初 `SendAckDirect` 是为了绕过 HAL 状态机而写的，因为旧版 HAL 在 DMA 接收模式下 `HAL_UART_Transmit` 会返回 `HAL_BUSY`。

### 实测结论：HAL_UART_Transmit 在 DMA 接收模式下可以正常发送

实验验证，直接用 `BspUartRcv_SendAck`（即 `HAL_UART_Transmit`）发送 ACK，上位机正常收到，MCU 正常复位。`SendAckDirect` 的寄存器级操作在当前 HAL 版本下不再需要。

---

## 5. HAL UART 状态机深度分析

### 双状态字段设计

HAL 的 UART 句柄里有**两个独立的状态字段**：

```c
// stm32f4xx_hal_uart.h
__IO HAL_UART_StateTypeDef  gState;    // 管 TX（发送）+ 全局管理
__IO HAL_UART_StateTypeDef  RxState;   // 管 RX（接收）
```

状态值用位编码：

```
HAL_UART_STATE_RESET      = 0x00  // 0000 0000  未初始化
HAL_UART_STATE_READY      = 0x20  // 0010 0000  初始化完成，空闲
HAL_UART_STATE_BUSY_TX    = 0x21  // 0010 0001  发送中（gState 用）
HAL_UART_STATE_BUSY_RX    = 0x22  // 0010 0010  接收中（RxState 用）
HAL_UART_STATE_BUSY_TX_RX = 0x23  // 0010 0011  收发同时（组合值）
```

`bit0` 是 TX 忙标志，`bit1` 是 RX 忙标志，`bit5` 是初始化完成标志。`BUSY_TX_RX = 0x23` 刚好是 `READY(0x20) | TX(0x01) | RX(0x02)` 的按位或结果。

### DMA 接收启动时：只改 RxState

```c
HAL_StatusTypeDef UART_Start_Receive_DMA(...)
{
    huart->RxState = HAL_UART_STATE_BUSY_RX;   // ← 只改 RxState
    // gState 没碰，还是 READY
    
    HAL_DMA_Start_IT(huart->hdmarx, ...);
    ATOMIC_SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);
    return HAL_OK;
}
```

此时句柄状态：

```
gState  = 0x20 (READY)     ← 没变
RxState = 0x22 (BUSY_RX)   ← 标记接收中
```

### 阻塞发送时：只检查 gState

```c
HAL_StatusTypeDef HAL_UART_Transmit(...)
{
    if (huart->gState == HAL_UART_STATE_READY)   // 0x20 == 0x20 ✓ 通过！
    {
        huart->gState = HAL_UART_STATE_BUSY_TX;  // 标记发送中
        
        while (huart->TxXferCount > 0U) {
            UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_TXE, ...);
            huart->Instance->DR = (*pdata8bits & 0xFFU);
            huart->TxXferCount--;
        }
        
        UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_TC, ...);
        huart->gState = HAL_UART_STATE_READY;    // 发完恢复
        return HAL_OK;
    }
    else
        return HAL_BUSY;
}
```

整个发送过程中 `RxState` 始终是 `BUSY_RX`，HAL 完全不管它。发送完成后 `gState` 恢复 `READY`，DMA 接收继续运行。

### 状态流转图

```
                    gState        RxState
初始化后:            READY(0x20)   READY(0x20)
启动DMA接收:         READY(0x20)   BUSY_RX(0x22)    ← 只改RxState
调用Transmit:        BUSY_TX(0x21) BUSY_RX(0x22)    ← 只改gState
Transmit完成:        READY(0x20)   BUSY_RX(0x22)    ← 只恢复gState
DMA接收完成:         READY(0x20)   READY(0x20)      ← 只恢复RxState
```

### 结论

旧版 HAL 只有一个 `State` 字段，启动 DMA 接收后整个句柄标记为 `BUSY`，发送函数直接返回 `HAL_BUSY`。新版 HAL 拆成 `gState` 和 `RxState` 两个字段，TX 和 RX 各走各的状态机，不再互相阻塞。因此在 DMA 接收模式下可以直接使用 `HAL_UART_Transmit` 阻塞发送，无需绕过 HAL 做寄存器级操作。

---

## 6. C 语言封装手法总结

BSP 层的 `bsp_uart_rcv` 模块展示了 C 语言模块化封装的典型手法：

### static 变量藏状态

```c
static uint8_t *s_rx_buf = NULL;
static uint8_t  s_proc_buf[64];
static volatile bool s_frame_ready = false;
```

`static` 在文件作用域意味着"只在本文件内可见"。外部代码无法直接访问 `s_frame_ready`，只能通过 `BspUartRcv_IsFrameReady()` 查询。`s_` 前缀（static 缩写）一眼能看出是模块级静态变量。

### 统一前缀模拟命名空间

所有对外函数都用 `BspUartRcv_` 前缀。C 语言没有命名空间，前缀就是模拟命名空间的方式，避免不同模块的函数名冲突。

### extern "C" 桥接 C++

```c
#ifdef __cplusplus
extern "C" {
#endif

void BspUartRcv_Init(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t size);

#ifdef __cplusplus
}
#endif
```

`extern "C"` 告诉 C++ 编译器这些函数用 C 的链接方式，不做函数名修饰（name mangling），这样 C++ 代码调用时能正确找到符号。
