/**
 * @file    task_gnss.h
 * @brief   GNSS 接收解析任务入口
 */

#ifndef XY7320_TASK_GNSS_H
#define XY7320_TASK_GNSS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GNSS 任务入口
 *
 * 由 App_Main_Start 创建，周期调用 GnssService::Update()。
 */
void Task_Gnss(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_TASK_GNSS_H */
