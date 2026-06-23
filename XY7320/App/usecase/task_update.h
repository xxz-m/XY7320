/**
 * @file    task_update.h
 * @brief   协议任务入口
 *          周期调用 ProtocolService::Update() 处理统一串口协议输入
 */

#ifndef XY7320_TASK_UPDATE_H
#define XY7320_TASK_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

/** 升级配置任务（周期调用，内部无死循环） */
void Task_UpdateConfig(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_TASK_UPDATE_H */
