/**
 * @file    App_Main.cpp
 * @brief   App 层入口实现
 *          初始化 Services，创建 OS 任务
 */

#include "App_Main.h"
#include "os.h"
#include "led_service.h"
#include "update_service.h"
#include "task_led.h"
#include "task_update.h"

extern "C" void App_Main_Init(void)
{
    /* App 层只调接口，不知道硬件细节 */
    LedService::Instance().Init();
    LedService::Instance().SetMode(LedService::Mode::BLINK);

    /* 初始化升级服务（串口 DMA + 写 A1 版本） */
    UpdateService::Instance().Init();
}

extern "C" void App_Main_Start(void)
{
    /* 创建所有 OS 任务 */
    OS_CreateTask(Task_LED, nullptr, 1);
    OS_CreateTask(Task_UpdateConfig, nullptr, 5);
}
