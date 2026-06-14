/**
 * @file    os_config.h
 * @brief   OS 调度器业务配置
 *          只定义业务需求（Tick 周期、任务数量），不涉及硬件细节
 *          硬件参数由 BSP 层根据此配置自动计算
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
 * BSP 层会根据此值自动计算定时器的 PSC/ARR
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
 * 默认空操作，应用层可覆盖为低功耗指令
 * 示例：#define OS_IDLE_ACTION()  __WFI()
 */
#ifndef OS_IDLE_ACTION
#define OS_IDLE_ACTION()  __WFI()
#endif

#ifdef __cplusplus
}
#endif

#endif /* OS_CONFIG_H */
