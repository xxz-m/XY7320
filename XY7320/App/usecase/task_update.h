/**
 * @file    task_update.h
 * @brief   升级任务入口
 *          周期调用 UpdateService::Update() 处理版本帧
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
