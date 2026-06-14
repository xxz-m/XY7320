//
// Created by Administrator on 2026/6/14.
//

#include "task_update.h"
#include "os.h"
#include "app_update_config.h"

extern "C" void Task_UpdateConfig(void *arg)
{
    App_UpdateConfig_Poll();
    OS_DelayMs(10);
}
