//
// Created by Administrator on 2026/6/14.
//

/**
 * 升级任务入口
 * 
 * 职责：在 OS 任务中周期调用 update_service::Update()
 * 
 * 风格与 task_led.cpp 一致：一行调用 + 一行延时
 */

#include "task_update.h"
#include "os.h"
#include "update_service.h"

extern "C" void Task_UpdateConfig(void *arg)
{
    update_service::Instance().Update();
    OS_DelayMs(10);
}
