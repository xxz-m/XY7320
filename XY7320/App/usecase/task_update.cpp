/**
 * @file    task_update.cpp
 * @brief   协议任务实现
 *
 *          每次只执行一步逻辑（ProtocolService::Update），末尾让出 CPU。
 */

#include "task_update.h"
#include "os.h"
#include "protocol_service.h"

/**
 * @brief 任务入口：每 10ms 调用 ProtocolService 协议分发
 *
 * 流程：从 BSP 串口接收层取出原始帧 → 拼流 → 拆帧 → 校验 → 分发到升级或命令处理器。
 */
extern "C" void Task_UpdateConfig(void *arg)
{
    (void)arg;
    ProtocolService::Instance().Update();
    OS_DelayMs(10);
}