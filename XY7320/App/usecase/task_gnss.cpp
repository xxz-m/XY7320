/**
 * @file    task_gnss.cpp
 * @brief   GNSS 接收解析任务实现
 */

#include "task_gnss.h"
#include "gnss_service.h"
#include "os.h"

extern "C" void Task_Gnss(void *arg)
{
    (void)arg;
    GnssService::Instance().Update();
    OS_DelayMs(5);
}
