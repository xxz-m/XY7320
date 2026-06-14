/**
 * @file    os.h
 * @brief   轻量级协作式任务调度器 - 接口
 *          非抢占式，每个任务入口函数执行一步逻辑后 return
 *          纯软件层，不依赖任何硬件外设
 */
#ifndef OS_H
#define OS_H

#include <stdint.h>
#include "os_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OS_INVALID_TASK     (-1)        /* 无效任务 ID */

/* ============================================================
 *  任务状态
 * ============================================================ */
typedef enum {
    OS_TASK_UNUSED    = 0,    /* 未使用（空闲槽位） */
    OS_TASK_READY     = 1,    /* 就绪，等待调度 */
    OS_TASK_RUNNING   = 2,    /* 正在执行 */
    OS_TASK_SUSPENDED = 3,    /* 挂起（手动暂停） */
    OS_TASK_DELAYED   = 4     /* 延时中（等待 Tick 倒计时） */
} OS_TaskState_t;

/* ============================================================
 *  任务控制块 (TCB)
 * ============================================================ */
typedef struct {
    void            (*entry)(void *arg);  /* 任务入口函数 */
    void            *arg;                 /* 任务参数 */
    uint8_t         priority;             /* 优先级（数值越大越优先） */
    OS_TaskState_t  state;                /* 当前状态 */
    uint32_t        delayTicks;           /* 剩余延时 Tick 数 */
} OS_TCB_t;

/* ============================================================
 *  内核接口
 * ============================================================ */

/**
 * @brief  初始化调度器（清空所有任务槽位）
 *         在 main() 中 HAL_Init() 之后调用
 */
void OS_Init(void);

/**
 * @brief  启动调度器（进入主循环，不会返回）
 *         在 main() 最后调用，替代 while(1)
 */
void OS_Start(void);

/**
 * @brief  Tick 中断回调
 *         由 BSP 层在定时器中断中调用，驱动延时倒计时
 *         应用层不应直接调用
 */
void OS_Tick(void);

/* ============================================================
 *  任务管理接口
 * ============================================================ */

/**
 * @brief  创建任务
 * @param  entry   任务入口函数（不能写 while(1)，只做一步就 return）
 * @param  arg     传给任务的参数（可为 NULL）
 * @param  priority 优先级（0~255，数值越大越优先）
 * @return 任务 ID（>=0 成功，OS_INVALID_TASK 失败）
 */
int32_t OS_CreateTask(void (*entry)(void *), void *arg, uint8_t priority);

/**
 * @brief  删除任务
 * @param  id  任务 ID（由 OS_CreateTask 返回）
 */
void OS_DeleteTask(int32_t id);

/**
 * @brief  挂起任务（暂停调度，不消耗 CPU）
 * @param  id  任务 ID
 */
void OS_SuspendTask(int32_t id);

/**
 * @brief  恢复任务（从挂起状态回到 READY）
 * @param  id  任务 ID
 */
void OS_ResumeTask(int32_t id);

/**
 * @brief  当前任务主动让出 CPU
 *         任务内部调用，把自己改回 READY，让调度器调度其他任务
 */
void OS_Yield(void);

/* ============================================================
 *  非阻塞延时
 * ============================================================ */

/**
 * @brief  延时 N 个 Tick
 *         非阻塞，任务进入 DELAYED 状态，Tick 倒计时结束后自动恢复
 * @param  ticks  Tick 数量
 */
void OS_DelayTicks(uint32_t ticks);

/**
 * @brief  延时 N 毫秒
 *         根据 OS_TICK_MS 自动换算成 Tick
 * @param  ms  毫秒数
 */
void OS_DelayMs(uint32_t ms);

/* ============================================================
 *  状态查询（调试用）
 * ============================================================ */

/**
 * @brief  获取任务当前状态
 * @param  id  任务 ID
 * @return 任务状态（OS_TaskState_t）
 */
OS_TaskState_t OS_GetTaskState(int32_t id);

/**
 * @brief  获取当前正在运行的任务 ID
 * @return 任务 ID（OS_INVALID_TASK 表示没有任务在运行）
 */
int32_t OS_GetCurrentTaskId(void);

/**
 * @brief  获取就绪任务数量
 * @return 就绪任务数
 */
uint8_t OS_GetReadyCount(void);

#ifdef __cplusplus
}
#endif

#endif /* OS_H */
