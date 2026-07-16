/**
 * @file    App_Main.cpp
 * @brief   App 层入口实现
 *
 *          初始化 Services，创建 OS 任务。
 */

#include "App_Main.h"

#include "app_config.h"
#include "os.h"
#include "led_service.h"
#include "log_service.h"
#include "protocol_service.h"
#include "update_service.h"
#include "task_led.h"
#include "adc_service.h"
#include "task_update.h"
#include "task_gnss.h"
#include "mode_manager.h"
#include "gnss_service.h"
#include "input_capture_service.h"
#include "uart_tx_service.h"

/**
 * @brief 初始化所有 Services
 *
 * 按依赖顺序：
 *  1. LogService 先于其他 Init，让后续初始化日志可写
 *  2. LedService 立即切到 BLINK，便于裸机阶段可见
 *  3. UpdateService 初始化协议串口 DMA 并写 A1 当前版本
 *  4. ProtocolService 初始化协议解析缓冲
 *  5. UartTxService 初始化发送 FIFO/ping-pong 与统计计数（在 ProtocolService
 *     之后，确保首批 ACK 入队时 Service 已就绪）
 *  6. ModeManager 初始化状态机（进入 Idle）；其 OnModeChanged 不依赖 TX
 *  7. AdcService 初始化采样服务
 *  8. GnssService 初始化 USART3 GNSS DMA+IDLE 接收
 */
extern "C" void App_Main_Init(void)
{
    /* App 层只调接口，不知道硬件细节 */
    LogService::Instance().Init();
    LedService::Instance().Init();
    LedService::Instance().SetMode(LedService::Mode::BLINK);
    /* 初始化升级服务（串口 DMA + 写 A1 版本） */
    UpdateService::Instance().Init();
    ProtocolService::Instance().Init();
    UartTxService::Instance().Init();
    ModeManager::Instance().Init();
    AdcService::Instance().Init();
    GnssService::Instance().Init();
    InputCaptureService::Instance().Init();
    (void)InputCaptureService::Instance().Start();
}

static void Task_ModeManager(void *arg);

/**
 * @brief 创建所有 OS 任务
 *
 * Priority 含义（数值越大优先级越高）：
 *  - Task_UpdateConfig = 5（最高优先：保证协议帧及时处理）
 *  - Task_ModeManager  = 3（次高优先：每 1ms 驱动当前 FSM 状态）
 *  - Task_LED          = 1（最低优先：LED 闪烁不需要严格实时）
 *
 * GNSS 不创建独立 OS 任务；进入 GNSS 状态后，
 * 由 TaskstateGnss::tick() 调用 GnssService::Update()。
 */
extern "C" void App_Main_Start(void)
{
    /* 创建所有 OS 任务 */
    OS_CreateTask(Task_LED, nullptr, 1);
    LOG_Printf("Task_LED,Create,OK\r\n");
    OS_CreateTask(Task_UpdateConfig, nullptr, 5);
    LOG_Printf("Task_UpdateConfig,Create,OK\r\n");
    OS_CreateTask(Task_ModeManager, nullptr, 3);
    LOG_Printf("Task_ModeManager,Create,OK\r\n");
}

/**
 * @brief ModeManager 调度任务：每 1ms Tick 一次
 *
 * 调用 ModeManager::Tick() 驱动当前状态机的 tick() 回调，
 * 通常会触发 AdcService::Update 或其他业务逻辑。
 */
static void Task_ModeManager(void *arg)
{
    (void)arg;
    ModeManager::Instance().Tick();
    InputCaptureService::Instance().Update();
    OS_DelayMs(1);
}
