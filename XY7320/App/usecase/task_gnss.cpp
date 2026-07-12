/**
 * @file    task_gnss.cpp
 * @brief   GNSS 接收解析任务实现
 */

#include "task_gnss.h"
#include "gnss_service.h"
#include "app_config.h"

TaskstateGnss& TaskstateGnss::Instance()
{
    static TaskstateGnss instance;
    return instance;
}

TaskstateGnss::TaskstateGnss() : fsm::State("GNSS"){}

void TaskstateGnss::entry()
{
    /*
    * GnssService::Init() 已经在 App_Main_Init() 中执行，
    * 这里不重复初始化 USART3、DMA 和 IDLE 中断。
    */
    GnssService::Instance().Start();
    LOG_Printf("TaskStateGnss,Entry\n");
}

void TaskstateGnss::exit()
{
    GnssService::Instance().Stop();
    LOG_Printf("TaskStateGnss,Exit\n");
}

void TaskstateGnss::react(const fsm::Event& event)
{
    (void)event;
}

void TaskstateGnss::tick()
{
    GnssService::Instance().Update();
}
