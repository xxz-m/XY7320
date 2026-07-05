/**
 * @file    task_led.cpp
 * @brief   LED 任务实现
 *
 *          每次只执行一步逻辑（LedService::Update），末尾让出 CPU。
 */

#include "task_led.h"
#include "led_service.h"
#include "os.h"

/**
 * @brief 任务入口：每 10ms 驱动一次 LedService 状态机
 *
 * 协作式调度：调用一次 Update 后立即 DelayMs 让出 CPU。
 */
extern "C" void Task_LED(void *arg)
{
    (void)arg;
    LedService::Instance().Update();
    OS_DelayMs(10);
}