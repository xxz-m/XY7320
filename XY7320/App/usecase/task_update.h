/**
 * @file    task_update.h
 * @brief   协议任务入口
 *
 *          由 App_Main::App_Main_Start 创建（优先级 5，最高）。
 *          周期调用 ProtocolService::Update() 处理统一串口协议输入。
 *
 *          命名说明：函数名 Task_UpdateConfig 与 UpdateService（升级服务）无关，
 *          实际跑的是协议分发；如有歧义可后续重构命名。
 */

#ifndef XY7320_TASK_UPDATE_H
#define XY7320_TASK_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 协议任务主体（周期调用，内部无死循环）
 *
 * @param arg  未使用
 */
void Task_UpdateConfig(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_TASK_UPDATE_H */