/**
 * @file    task_led.h
 * @brief   LED 任务入口
 *          周期调用 LedService::Update() 驱动 LED 状态
 */

#ifndef XY7320_TASK_LED_H
#define XY7320_TASK_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/** LED 任务（周期调用，内部无死循环） */
void Task_LED(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_TASK_LED_H */
