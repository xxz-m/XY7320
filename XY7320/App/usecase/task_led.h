/**
 * @file    task_led.h
 * @brief   LED 任务入口
 *
 *          由 App_Main::App_Main_Start 创建（优先级 1，最低）。
 *          每次只执行一步逻辑（LedService::Update），末尾让出 CPU。
 */

#ifndef XY7320_TASK_LED_H
#define XY7320_TASK_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED 任务主体（周期调用，内部无死循环）
 *
 * @param arg  未使用
 */
void Task_LED(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_TASK_LED_H */