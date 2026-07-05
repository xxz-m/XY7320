/**
 * @file    os_config.h
 * @brief   OS 调度器配置
 *          定义调度器运行参数，不依赖 HAL 头文件
 */
#ifndef OS_CONFIG_H
#define OS_CONFIG_H

#include <stdint.h>
#include <stddef.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 *  业务需求配置
 * ============================================================ */

/**
 * OS Tick 周期（毫秒）
 * 系统调度的基本时间单位
 * 由 bsp_tim_os.c 读取 OS_TICK_MS 自动配置 TIM14 的 PSC/ARR。
 */
#define OS_TICK_MS          1u

/**
 * 最大任务数量
 * 根据项目实际需求调整，占用 RAM = OS_MAX_TASKS * sizeof(OS_TCB_t)
 */
#define OS_MAX_TASKS        10

/* ============================================================
 *  平台适配配置
 * ============================================================ */

/**
 * 空闲时 CPU 行为
 * __WFI()：Wait For Interrupt，Cortex-M 低功耗等待指令
 */
#ifndef OS_IDLE_ACTION
#define OS_IDLE_ACTION()  __WFI()
#endif

#ifdef __cplusplus
}
#endif

#endif /* OS_CONFIG_H */
