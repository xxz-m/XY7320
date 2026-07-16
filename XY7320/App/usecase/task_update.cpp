/**
 * @file    task_update.cpp
 * @brief   协议任务实现
 *
 *          每次只执行一步逻辑（ProtocolService::Update + UartTxService::Update），末尾让出 CPU。
 */

#include "task_update.h"
#include "os.h"
#include "protocol_service.h"
#include "uart_tx_service.h"
#include "update_service.h"

/**
 * @brief 任务入口：每 10ms 驱动协议 RX 解析与 TX 调度
 *
 * 流程：
 *  1. ProtocolService::Update：从 BSP 串口接收层取出原始帧 → 拼流 → 拆帧 → 校验 → 分发到升级或命令处理器。
 *  2. UartTxService::Update：消费控制 ACK FIFO 与 Mode ping-pong，按"控制优先 + 会话校验"发送。
 *  3. UpdateService::Update：非阻塞检查升级 ACK 是否排空，满足条件后复位。
 */
extern "C" void Task_UpdateConfig(void *arg)
{
    (void)arg;
    ProtocolService::Instance().Update();
    UartTxService::Instance().Update();
    UpdateService::Instance().Update();
    OS_DelayMs(10);
}