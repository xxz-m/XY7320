---
title: ADC采样+DMA+TIM配置与流控方案
tags:
  - docs/reference
  - stm32
  - adc
  - dma
  - timer
status: active
---

# ADC 采样 + DMA + TIM 配置与流控方案

## 硬件方案概述

采用 TIM3 触发 ADC1 扫描，DMA2 Stream0 Circular 模式搬运结果的标准链路：

```
TIM3 TRGO → ADC1 外部触发 → 8通道Scan转换 → DMA2_Stream0 → adc_dma_buf[512]
```

MCU 为 STM32F407ZGT6，ADC 时钟 PCLK2/4 = 42 MHz，12-bit 分辨率。

## 通道分配

| Rank | ADC通道 | GPIO | 归属 | 用途 |
|------|---------|------|------|------|
| 1 | IN0 | PA0 | TaskA | 算法处理，需存512样本 |
| 2 | IN1 | PA1 | TaskA | 算法处理，需存512样本 |
| 3 | IN4 | PA4 | TaskA | 算法处理，需存512样本 |
| 4 | IN5 | PA5 | TaskA | 算法处理，需存512样本 |
| 5 | IN6 | PA6 | TaskB | 业务处理，需存512样本 |
| 6 | IN7 | PA7 | TaskB | 业务处理，需存512样本 |
| 7 | IN8 | PB0 | Idle | 累加512次取平均 |
| 8 | IN9 | PB1 | Idle | 累加512次取平均 |

前 6 通道（TaskA + TaskB）需要存储 512 个独立样本供后续算法使用；后 2 通道（Idle）只需累加 512 次后求平均。

## CubeMX 配置要点

### TIM3（触发源）

- Clock Source: Internal Clock
- Prescaler: 0（84 MHz 计数频率，APB1 定时器）
- Counter Period: 839（84 MHz / 840 = 100 kHz 触发率）
- auto-reload preload: Enable
- Master Output Trigger (TRGO): Update Event
- NVIC: **不使能**（只需 TRGO 输出，不要中断）

### ADC1

- Clock Prescaler: ADC Clock Sync PCLK Divided by 4（42 MHz）
- Resolution: 12 bits
- Scan Conversion Mode: **Enable**
- Number of Conversion: 8
- Continuous Conversion Mode: Disable（由 TIM3 外部触发控制节奏）
- External Trigger Conversion Source: **TIM3 TRGO Event**
- External Trigger Edge: Rising Edge
- DMA Continuous Requests: Enable
- Data Alignment: Right
- Number of Rank: 8
- 各 Rank 采样时间: 15 Cycles

### DMA2 Stream0（ADC1 关联）

- Channel: DMA_CHANNEL_0
- Direction: Peripheral To Memory
- Data Width: HalfWord / HalfWord
- Mode: **Circular**（CubeMX 默认 Normal，必须手动改）
- MemInc: Enable
- PeriphInc: Disable
- Priority: High
- FIFO Mode: Disable（直接模式）
- NVIC: DMA2 Stream0 interrupt **使能**

> [!warning] CubeMX 易错点
> DMA Mode 默认是 Normal，搬完一轮就停了。必须手动改为 Circular，否则第二轮 TIM3 触发后 DMA 不工作，数据全丢。

## 时序与采样率分析

ADC 时钟 42 MHz，12-bit 转换固定 12 周期，加 15 周期采样 = 每通道 27 周期 = 0.643 µs。

8 通道一轮扫描 = 27 × 8 = 216 周期 = 5.14 µs。

TIM3 触发周期 = 10 µs（100 kHz），大于单轮扫描耗时，ADC 有充足时间完成转换。

DMA buffer 512 元素 / 8 通道 = 64 轮扫描填满一轮。每轮 TC 中断间隔 = 64 × 10 µs = 640 µs。

凑满 512 样本/通道需要 8 轮 TC = 8 × 640 µs = 5.12 ms。

## DMA Buffer 布局

```c
// CubeMX 生成的 DMA 缓冲区
static uint16_t adc_dma_buf[512];
```

DMA Circular 模式下，buffer 按扫描顺序循环填充：

```
adc_dma_buf[0]   = Rank1(CH0) 第1次扫描
adc_dma_buf[1]   = Rank2(CH1) 第1次扫描
...
adc_dma_buf[7]   = Rank8(CH9) 第1次扫描
adc_dma_buf[8]   = Rank1(CH0) 第2次扫描
...
adc_dma_buf[511] = Rank8(CH9) 第64次扫描
```

解交织公式：`adc_dma_buf[scan_index * 8 + channel_rank]`，其中 `scan_index` ∈ [0, 63]，`channel_rank` ∈ [0, 7]。

## 流控方案

### 设计思路

采集与消费解耦：DMA 持续采集，攒满 512 样本后暂停定时器，等 App 层处理完再恢复。

```
TIM3 运行 → 64轮扫描 → TC中断 → 搬数据 → 第1~7轮:立即恢复TIM3
                                         → 第8轮:  置flag,停TIM3
                                                      ↓
App任务看到flag → 处理数据 → 清flag → 调BspAdc_Resume() → 恢复TIM3
```

### TC 回调实现

```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    // 每轮TC：64次扫描的数据在adc_dma_buf里
    // 解交织搬运：前6通道存样本，后2通道累加
    for (int i = 0; i < 64; i++) {
        for (int ch = 0; ch < 6; ch++) {
            adc_store[ch][store_idx + i] = adc_dma_buf[i * 8 + ch];
        }
        adc_idle_acc[0] += adc_dma_buf[i * 8 + 6];
        adc_idle_acc[1] += adc_dma_buf[i * 8 + 7];
    }

    store_idx += 64;
    if (++round_cnt >= 8) {
        store_idx = 0;
        round_cnt = 0;
        adc_data_ready = 1;
        HAL_TIM_Base_Stop(&htim3);  // 暂停，等app消费后恢复
    }
    // 第1~7轮：TIM3不停，继续触发下一轮扫描
}
```

### BSP 恢复接口

App 任务消费完数据后调用此接口恢复采集：

```c
void BspAdc_Resume(void)
{
    adc_idle_acc[0] = 0;
    adc_idle_acc[1] = 0;
    adc_data_ready = 0;
    HAL_TIM_Base_Start(&htim3);
}
```

### 时序总结

| 阶段 | 时长 | TIM3状态 |
|------|------|----------|
| 第1~7轮采集（每轮640µs） | 4.48 ms | 运行 |
| 第8轮采集完成，置flag | 0 | 停止 |
| App处理耗时 | 不定 | 停止 |
| Resume后重新开始 | 5.12 ms | 运行 |

有效采样率受 App 处理耗时影响。极端情况下 App 瞬间消费，周期 = 5.12 ms，对应 100 kHz 扫描率。

## BSP 层数据结构

```c
// DMA缓冲区（CubeMX生成的DMA目标）
static uint16_t adc_dma_buf[512];

// 前6通道存储缓冲区（TaskA: 4ch + TaskB: 2ch）
static uint16_t adc_store[6][512];
static uint16_t store_idx;      // 当前写入偏移，0~448，步进64

// 后2通道累加器（Idle任务用）
static uint32_t adc_idle_acc[2];  // uint32防溢出：512×4095=2,096,640

// 轮次计数
static uint8_t round_cnt;        // 0~7

// 数据就绪标志
static volatile uint8_t adc_data_ready;
```

## 启动顺序

```c
// 1. 先启动DMA（挂上等ADC的请求）
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buf, 512);
// 2. 再启动TIM3（开始发触发脉冲）
HAL_TIM_Base_Start(&htim3);
```

顺序不能反：DMA 必须先于 TIM3，否则 TIM3 触发 ADC 转换后 DMA 没挂上，第一次转换结果丢失。

## 注意事项

1. `HAL_TIM_Base_Stop` 而非 `_Stop_IT`：TIM3 没有开中断，不需要清标志关中断源。
2. TC 回调中搬 64×6 = 384 次内存拷贝，耗时约 20~30 µs，远小于 DMA 回绕周期 640 µs，不会发生数据覆盖。
3. Idle 通道累加器用 `uint32_t`：512 × 4095 = 2,096,640，不溢出 uint32。
4. `adc_data_ready` 必须 `volatile`，跨中断上下文读写。
