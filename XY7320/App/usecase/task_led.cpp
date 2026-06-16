/**
 * @file    task_led.cpp
 * @brief   LED 任务实现
 *          每次只执行一步逻辑，末尾让出 CPU
 */

#include "task_led.h"
#include "led_service.h"
#include "os.h"

extern "C" void Task_LED(void* arg)
{
    LedService::Instance().Update();
    OS_DelayMs(10);
}
