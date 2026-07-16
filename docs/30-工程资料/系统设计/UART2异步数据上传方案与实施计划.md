---
title: UART2 异步数据上传方案与实施计划
aliases:
  - UART2数据上传方案
  - TX DMA与乒乓缓冲方案
tags:
  - architecture
  - uart
  - dma
  - protocol
  - telemetry
status: proposed
---

# UART2 异步数据上传方案与实施计划

## 设计背景

USART2 当前承担上位机通信：RX 使用 DMA + IDLE 接收命令，TX 使用 `HAL_UART_Transmit()` 发送短协议应答。当前代码没有把 DMR、GSM 或 GNSS 业务结果上传给上位机，也没有 USART2 TX DMA、统一发送调度或业务数据缓存。

现有业务可分为两类：

- 控制应答：模式切换 ACK、升级 ACK，由 `ProtocolService` 在收到命令后产生。
- ModeManager 业务数据：Idle、DMR、GSM、GNSS 互斥状态中当前状态产生的数据。

未来可能增加电池电量、设备状态等独立数据源。这些数据可能与 ModeManager 当前业务同时产生，因此 UART2 发送不能长期依赖某个业务状态直接操作 HAL。

> [!important] 方案边界
> 当前代码没有告警上传业务，本方案不设计告警队列。未来如果出现明确的告警协议和可靠性要求，再增加相应数据源和调度规则。

## 设计目标

- UART2 的正常发送只有一个所有者，避免多个模块同时操作 HAL UART 状态机。
- 业务模块提交数据后立即返回，不等待串口物理发送完成。
- 控制 ACK 保持先后顺序，并优先于普通业务数据发送。
- ModeManager 状态切换时丢弃旧模式尚未发送的数据。
- DMA 正在发送的缓冲在完成前保持不变，不产生半帧或数据撕裂。
- 固定内存，不使用 `malloc`、`new` 或动态容器。
- 后续增加电池数据时不改变 ModeManager 和 UART2 底层职责。

## 总体结构

```text
上位机命令
  ↓ USART2 RX DMA + IDLE
ProtocolService::Update()
  ├─ 解析命令并请求 ModeManager 切换
  └─ 编码 ACK ───────────────┐
                              ↓
TaskStateDmr/Gsm/Gnss      Control FIFO
  ↓ 新业务结果                │
ProtocolService 编码          │ 高优先级
  ↓                           │
Mode 乒乓缓冲 ────────────────┤
                              ↓
                        UartTxService
                              ↓
                  HAL_UART_Transmit_DMA()
                              ↓
                  HAL_UART_TxCpltCallback()
```

各层职责：

| 层级 | 责任 | 禁止承担的责任 |
| --- | --- | --- |
| ModeManager 状态 | 产生 DMR/GSM/GNSS 业务结果，决定何时发布 | 不直接调用 UART HAL，不管理 DMA busy |
| ProtocolService | 定义命令码、序列化字段、编码协议帧 | 不等待串口发送完成 |
| UartTxService | 保存待发送帧、选择下一帧、管理 DMA 生命周期 | 不解释 DMR/GSM 业务含义 |
| BSP/HAL | USART2 TX DMA 初始化和硬件发送 | 不处理协议优先级和模式切换 |

## 为什么使用异步发送

XY7320 使用协作式调度器。任务调用 `HAL_UART_Transmit()` 后，会在当前任务上下文中等待整帧发送完成。115200 baud、8N1 下，每个字节在线路上约占 86.8 us；128 字节约占 11.1 ms。周期性上传会延迟 `Task_ModeManager`、协议接收和未来其他任务。

异步发送的业务调用只执行：

```text
协议编码 → 复制到内部缓存 → 标记待发送 → 返回
```

串口物理发送由 DMA 完成，业务状态无需等待。

## USART2 TX DMA

当前 USART2 只配置 `hdma_usart2_rx`。实施时通过 CubeMX 为 USART2 增加 TX DMA，具体 Stream/Channel 以 CubeMX 对 STM32F407ZGT6 的合法映射为准，不在手写代码中猜测。

TX DMA 使用 Normal 模式：

```text
Direction           Memory to peripheral
Peripheral increment Disabled
Memory increment     Enabled
Data alignment       Byte
Mode                 Normal
```

发送入口：

```cpp
HAL_UART_Transmit_DMA(&huart2, frame.data, frame.length);
```

DMA 完成回调只通知发送服务：

```cpp
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart != nullptr && huart->Instance == USART2) {
        UartTxService::Instance().OnTxCompleteFromIsr();
    }
}
```

ISR 不编码协议、不选择优先级、不写日志。推荐只设置 `volatile` 完成标志，由通信任务释放当前缓冲并启动下一帧。

## 统一发送服务

建议新增 `UartTxService`，作为正常 USART2 TX 的唯一入口。

概念接口：

```cpp
class UartTxService
{
public:
    static UartTxService& Instance();

    void Init();
    void Update();

    bool EnqueueControl(const uint8_t* data, uint16_t length);
    bool PublishModeData(mode::ModeType mode,
                         uint32_t generation,
                         const uint8_t* data,
                         uint16_t length);

    void OnModeChanged(mode::ModeType mode, uint32_t generation);
    void OnTxCompleteFromIsr();
};
```

所有正常发送统一进入该 Service：

- `ProtocolService::SendPacket()` 编码 ACK 后调用 `EnqueueControl()`。
- DMR/GSM/GNSS 编码业务帧后调用 `PublishModeData()`。
- 后续电池业务通过独立发布接口进入发送调度。

`BspUartRcv_SendAckDirect()` 保留为升级异常时的寄存器轮询兜底，不参与正常发送。

## 控制 ACK FIFO

FIFO 表示先进先出。连续收到多个上位机命令时，应答必须保持顺序：

```text
ACK A → ACK B → ACK C
```

建议使用固定环形 FIFO：

```cpp
struct TxFrame
{
    uint16_t length;
    uint8_t data[TX_FRAME_MAX];
};

struct ControlQueue
{
    TxFrame frames[CONTROL_QUEUE_SIZE];
    uint8_t readIndex;
    uint8_t writeIndex;
    uint8_t count;
};
```

初始建议：

```text
CONTROL_QUEUE_SIZE = 4 或 8
TX_FRAME_MAX       = 按协议最大编码帧确定
```

队列满时 `EnqueueControl()` 返回 `false` 并累计 `overflowCount`。不能覆盖尚未发送的旧 ACK，也不能阻塞当前业务任务等待空间。

## Mode 业务乒乓缓冲

DMR、GSM 和 GNSS 由 ModeManager 互斥运行，可以共用一组双缓冲：

```text
Buffer A
Buffer B
```

每个缓冲具有显式状态：

```cpp
enum class TxBufferState : uint8_t
{
    Free,
    Pending,
    Sending
};

struct ModeTxBuffer
{
    TxBufferState state;
    mode::ModeType mode;
    uint32_t generation;
    uint16_t length;
    uint8_t data[TX_FRAME_MAX];
};
```

合法运行过程：

```text
A = Sending，B = Free
  ↓ 新数据产生
A = Sending，B = Pending
  ↓ 又产生更新数据
A = Sending，B 仍为 Pending，但内容覆盖为最新帧
  ↓ A 发送完成
A = Free，B = Sending
```

任何时候都不能覆盖 `Sending` 缓冲。业务数据只能写入 `Free`，没有 Free 时可覆盖同一会话的 `Pending`，从而只保留等待区中的最新测量结果。

乒乓缓冲用于 Mode 业务，不替代 ACK FIFO。ACK 需要保序；Mode 测量数据更关注最新值。

## 模式切换与缓存清理

状态切换时需要处理测量缓存和上传缓存：

```text
旧状态 exit()
  ↓ 停止旧 ADC
UartTxService::OnModeChanged(nextMode, nextGeneration)
  ↓ 清除旧模式 Pending
ModeManager 更新 currentMode/currentState
  ↓
新状态 entry()
  ↓ 清除输入捕获和测量历史
```

缓存处理规则：

| 缓存 | 切换时行为 | 原因 |
| --- | --- | --- |
| Control FIFO | 保留 | 模式 ACK 和升级 ACK 不能被业务切换删除 |
| Mode Pending | 清除 | 旧模式尚未发送的数据已经过期 |
| Mode Sending | 保留到完整发送结束 | 中断 DMA 会在线路上留下半帧 |
| InputCapture 结果 | 新状态 entry 时清除 | 防止旧周期脉宽触发新状态 |
| ADC 状态 | 旧状态 exit 时停止 | 防止旧模式继续更新功率 |

### generation 会话编号

只保存 `mode` 不能覆盖 `DMR → GSM → DMR` 的情况。旧 DMR Pending 和新 DMR 名称相同，但属于不同会话。

ModeManager 每次成功切换时递增会话编号：

```cpp
++m_modeGeneration;
```

Mode 上传帧携带：

```text
mode + generation
```

发送前检查：

```cpp
if (buffer.mode != currentMode ||
    buffer.generation != currentGeneration) {
    buffer.state = TxBufferState::Free;
}
```

这既是模式切换清理的保险，也能避免清理路径遗漏后发送旧会话数据。

### 推荐切换顺序

```text
1. 收到 GSM 命令
2. 选择 nextState / nextMode
3. 当前 DMR 状态 exit，停止 ADC
4. generation 递增
5. 清除 DMR Pending 上传帧
6. 更新 currentState/currentMode
7. GSM 状态 entry，清除 GSM 捕获历史
8. GSM ACK 进入 Control FIFO
9. 当前正在发送的完整 DMR 帧结束
10. 优先发送 GSM ACK
11. GSM 新测量有效后发布第一帧 GSM 数据
```

允许线上出现：

```text
最后一帧完整 DMR → GSM ACK → 第一帧 GSM
```

禁止出现：

```text
半帧 DMR → GSM ACK → 旧 DMR Pending → GSM
```

## 发送调度

当前业务只需要两级调度：

```text
1. Control FIFO
2. Mode Pending
```

调度伪代码：

```cpp
void UartTxService::Update()
{
    HandleTxCompletion();

    if (m_txBusy) {
        return;
    }

    if (ControlQueueNotEmpty()) {
        StartControlFrame();
        return;
    }

    if (ModePendingAvailable()) {
        StartModeFrame();
    }
}
```

优先级只决定下一帧，不抢占当前 DMA 帧。普通业务帧应控制长度，降低 ACK 的最坏等待时间。

## 与现有任务的结合

当前不新增独立 OS 任务。发送 Service 由现有通信任务驱动：

```cpp
extern "C" void Task_UpdateConfig(void* arg)
{
    (void)arg;

    ProtocolService::Instance().Update();
    UartTxService::Instance().Update();
    UpdateService::Instance().Update();

    OS_DelayMs(10);
}
```

长期可将任务改名为 `Task_Protocol` 或 `Task_Communication`，因为它将同时负责上位机 RX 和 TX。是否缩短 10 ms 周期，需要根据 ACK 延迟要求和实测任务耗时决定。

DMA 完成 ISR 只设置完成标志，队列出队、缓冲释放和下一帧选择全部在该任务上下文执行，减少 ISR 与任务并发修改状态。

## ProtocolService 改造

正常 ACK 当前调用：

```text
ProtocolService::SendPacket()
  ↓
BspUartRcv_SendAck()
  ↓
HAL_UART_Transmit()
```

TX DMA 上线后改为：

```text
ProtocolService::SendPacket()
  ↓ Protocol::EncodePacket()
UartTxService::EnqueueControl()
```

Mode 业务建议增加明确发布接口：

```cpp
bool PublishDmr(const DmrUploadData& data,
                uint32_t generation);

bool PublishGsm(const GsmUploadData& data,
                uint32_t generation);

bool PublishGnss(const GnssUploadData& data,
                 uint32_t generation);
```

这些接口逐字段序列化 payload，再调用 `Protocol::EncodePacket()`。禁止直接上传结构体内存，避免字节序、对齐和编译器填充差异。

业务状态只在新测量完成或达到上传周期时发布，不能在每次 `tick()` 中无条件重复发送同一结果。

## 未来电池数据扩展

电池数据独立于 ModeManager，会与 DMR/GSM/GNSS 同时产生。加入电池业务时，不建议写入 Mode 乒乓缓冲，否则高频 Mode 数据可能覆盖电池数据。

建议增加独立最新值槽位：

```text
Control FIFO
Mode Ping-Pong
Battery Latest Slot
```

发送调度可以扩展为：

```text
Control → Mode/Battery 公平轮询
```

电池数据通常按固定周期或变化阈值发布。具体周期、可靠性和公平策略等电池业务需求明确后再决定，不在当前阶段提前实现。

## 错误和统计

建议 `UartTxService` 暴露只读统计：

```cpp
struct UartTxStats
{
    uint32_t sentFrames;
    uint32_t controlQueueOverflow;
    uint32_t modePendingOverwrite;
    uint32_t staleModeDrop;
    uint32_t dmaStartBusy;
    uint32_t dmaStartError;
};
```

处理原则：

- `HAL_BUSY`：保留当前待发送帧，下轮重试，并累计计数。
- `HAL_ERROR`：不在 ISR 中循环重试，记录错误并由任务上下文恢复。
- Mode Pending 被新数据覆盖：属于设计允许行为，累计覆盖次数便于观察带宽是否不足。
- generation 不匹配：丢弃旧会话帧并累计 `staleModeDrop`。

## 内存和临界区

发送队列和乒乓缓冲必须保存完整帧副本。不能把局部数组指针交给 DMA，因为函数返回后该内存不再可靠。

推荐并发边界：

- `ProtocolService`、ModeManager 和 `UartTxService::Update()` 在任务上下文操作队列和缓冲状态。
- TX DMA ISR 只写 `txComplete` 标志。
- 若多个 OS 任务未来都调用发布接口，入队和缓冲状态切换使用极短临界区。
- 临界区内只修改索引、状态和长度；大块 `memcpy` 尽量放在临界区外，通过“先复制、后提交状态”完成发布。

## 实施计划

### 阶段一：硬件发送能力

1. 在 CubeMX 为 USART2 增加 TX DMA。
2. 重新生成并核对 `.ioc`、`usart.c`、DMA IRQ 和句柄链接。
3. 使用固定静态缓冲调用 `HAL_UART_Transmit_DMA()` 发送测试帧。
4. 验证 TX 完成回调只对 USART2 生效。
5. 验证 RX DMA + IDLE 在 TX DMA 工作期间仍能接收命令。

完成标准：连续发送测试帧时无 `HAL_BUSY` 卡死，USART2 RX 命令仍可解析。

### 阶段二：UartTxService 与 ACK FIFO

1. 新增 `UartTxService` 和固定 Control FIFO。
2. 实现 `Init()`、`Update()`、`EnqueueControl()` 和完成标志处理。
3. 将正常 `ProtocolService::SendPacket()` 改为 ACK 入队。
4. 保留 `BspUartRcv_SendAckDirect()` 作为升级异常兜底。
5. 增加发送统计和队列满返回值。

完成标准：连续发送多条模式命令时 ACK 顺序正确，不出现阻塞 TX 与 DMA TX 并发操作 USART2。

### 阶段三：Mode 乒乓缓冲

1. 增加两个 `ModeTxBuffer` 和 `Free/Pending/Sending` 状态。
2. 实现 `PublishModeData()`，禁止覆盖 `Sending`，允许更新 `Pending` 为最新帧。
3. 增加 Mode 业务协议命令和逐字段序列化。
4. DMR/GSM/GNSS 只在新结果或上传周期到达时发布。
5. 调度顺序设为 Control FIFO 优先、Mode Pending 次之。

完成标准：DMA 发送期间业务可持续更新 Pending，上位机收到的每帧 CRC 和字段一致，不出现数据撕裂。

### 阶段四：模式切换清理

1. ModeManager 增加 `modeGeneration`。
2. 在统一切换路径调用 `OnModeChanged(nextMode, generation)`。
3. 清除旧 Mode Pending，保留当前 Sending 和 Control FIFO。
4. 发送前再次检查 `mode + generation`。
5. 覆盖 DMR → GSM、GSM → Idle、DMR → Idle → DMR 等切换场景。

完成标准：切换后不再发送旧模式 Pending；已开始的帧完整结束；随后优先发送切换 ACK 和新模式数据。

### 阶段五：带宽与长期扩展

1. 实测 DMR/GSM/GNSS 帧长和发布频率。
2. 记录 `modePendingOverwrite` 和 Control FIFO 峰值。
3. 根据实测决定通信任务周期是否从 10 ms 缩短。
4. 电池业务明确后增加独立最新值槽位。
5. 数据源增多后再设计 Mode 与电池的公平轮询，不提前增加告警队列。

完成标准：在目标上传频率下 ACK 延迟满足上位机要求，控制队列无持续溢出，普通数据覆盖率处于可接受范围。

## 验证场景

| 场景 | 预期结果 |
| --- | --- |
| 空闲时收到 DMR 命令 | DMR ACK 入 FIFO 并发送，状态进入 DMR |
| DMR 数据正在发送时收到 GSM 命令 | 当前 DMR 帧完整结束，旧 DMR Pending 清除，下一帧优先 GSM ACK |
| DMR DMA 发送期间连续产生多组 DMR 数据 | Pending 最终保留最新一组，不覆盖 Sending |
| 连续收到多个控制命令 | ACK 按接收顺序发送 |
| DMR → Idle → DMR | 旧 DMR generation 数据不允许在新 DMR 会话发送 |
| Control FIFO 满 | 新 ACK 入队失败并记录溢出，不覆盖旧 ACK |
| HAL 返回 BUSY | 当前帧保留，下轮重试，不错误出队 |
| TX DMA 运行时接收上位机命令 | RX DMA + IDLE 继续工作，协议可解析 |

## 关键决策

- 正常 USART2 TX 统一使用 `UartTxService`，业务模块不直接调用 HAL。
- 控制 ACK 使用 FIFO，ModeManager 业务使用乒乓缓冲。
- 当前仅设计 Control 与 Mode 两类数据，不实现告警队列。
- 模式切换清除 Pending，不中断 Sending，不清除 Control FIFO。
- 使用 `mode + generation` 防止旧会话数据误发。
- 当前复用通信任务驱动 TX，不新增独立 OS 任务。
- 未来电池数据使用独立最新值槽位，不与 Mode 乒乓缓冲互相覆盖。

## 上行 payload 字段与命令码（实施版本）

实施版本把上行帧命令码与字段长度集中放到 `Domain/protocol/configurations.h`：

```text
UPLINK_CMD_DMR_MEAS  = 0x30
UPLINK_CMD_GSM_MEAS  = 0x31
UPLINK_CMD_GNSS_MEAS = 0x32
UPLINK_CMD_BATTERY   = 0x33  (预留)

业务头布局（大端）：
  mode(1) + generation低16位(2) + seq(2) + payload_len(1) + payload(N) + reserved(2)

DMR/GSM payload（12 字节）：
  timestamp_ms(4) + period_us(4) + pulse_width_us(4)

GNSS payload（17 字节）：
  latitude_e7(4) + longitude_e7(4) + fix_quality(1) + satellites(1)
  + hdop_x100(2) + altitude_cm(4) + has_fix(1)
```

上述业务头和 payload 会先写入 `ProtocolPacket::data`，再通过 `Protocol::EncodePacket()` 生成包含端口、model、cmd、CRC、帧尾与转义的完整 USART2 帧。DMR/GSM 当前数值仍是周期/脉宽占位数据，但传输格式已固定，不上传 C++ 原生结构体内存。

> 数值 0x30 与 `protocol_xy.h::DeviceButton` 读命令相同，但方向由 `model` 区分（读 unRead / 上行 unWrite），不会在协议解析时冲突。

实施版本同步落地在 `.trae/documents/UART2异步数据上传-ReceiveToIdleDMA迁移与发送服务实施计划.md`，可作为本文档的代码级落地清单。

## 相关文档

- [[30-工程资料/系统设计/XY7320任务详解]]
- [[30-工程资料/系统设计/串口DMA收发与HAL状态机分析]]
- [[30-工程资料/系统设计/XY7320-ModeManager与TinyFSM设计说明]]
- [[20-开发记录/决策记录]]
