//
// Created by Administrator on 2026/6/14.
//

#include "task_led.h"
#include "led_service.h"
#include "os.h"

extern "C" void Task_LED(void* arg)
{
    Led_service::Instance().Update();
    OS_DelayMs(10);
}
