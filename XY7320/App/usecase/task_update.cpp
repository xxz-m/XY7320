/**
 * @file    task_update.cpp
 * @brief   升级任务实现
 *          每次只执行一步逻辑，末尾让出 CPU
 *
 * 职责：在 OS 任务中周期调用 UpdateService::Update()
 * 风格与 task_led.cpp 一致：一行调用 + 一行延时
 */

#include "task_update.h"
#include "os.h"
#include "update_service.h"

extern "C" void Task_UpdateConfig(void *arg)
{
    UpdateService::Instance().Update();
    OS_DelayMs(10);
}
