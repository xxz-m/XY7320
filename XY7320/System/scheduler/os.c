/**
 * @file    os.c
 * @brief   轻量级协作式任务调度器 - 实现
 *          非抢占式调度，纯软件层，不依赖任何硬件外设
 *
 * 依赖关系：
 *   os.c → os.h → os_config.h（只读取 OS_TICK_MS、OS_MAX_TASKS）
 *   不依赖 HAL、BSP 或任何硬件头文件
 */
#include "os.h"

/* ============================================================
 *  内部变量
 * ============================================================ */
static OS_TCB_t  g_tasks[OS_MAX_TASKS];       /* 任务控制块数组 */
static int32_t   g_currentTask = OS_INVALID_TASK;  /* 当前运行任务 ID */

/* ============================================================
 *  内部函数声明
 * ============================================================ */
static int32_t OS_FindEmptySlot(void);
static int32_t OS_SelectNextTask(void);
static void    OS_ScheduleOnce(void);

/* ============================================================
 *  内核接口实现
 * ============================================================ */

void OS_Init(void)
{
    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        g_tasks[i].entry      = NULL;
        g_tasks[i].arg        = NULL;
        g_tasks[i].priority   = 0;
        g_tasks[i].state      = OS_TASK_UNUSED;
        g_tasks[i].delayTicks = 0;
    }
    g_currentTask = OS_INVALID_TASK;
}

void OS_Start(void)
{
    while (1) {
        OS_ScheduleOnce();
        OS_IDLE_ACTION();  /* 空闲行为由 os_config.h 配置 */
    }
}

void OS_Tick(void)
{
    /* 扫描所有 DELAYED 任务，做倒计时 */
    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == OS_TASK_DELAYED && g_tasks[i].delayTicks > 0) {
            g_tasks[i].delayTicks--;
            if (g_tasks[i].delayTicks == 0) {
                g_tasks[i].state = OS_TASK_READY;
            }
        }
    }
}

/* ============================================================
 *  任务管理接口实现
 * ============================================================ */

int32_t OS_CreateTask(void (*entry)(void *), void *arg, uint8_t priority)
{
    int32_t id = OS_FindEmptySlot();
    if (id == OS_INVALID_TASK || entry == NULL) {
        return OS_INVALID_TASK;
    }

    g_tasks[id].entry      = entry;
    g_tasks[id].arg        = arg;
    g_tasks[id].priority   = priority;
    g_tasks[id].state      = OS_TASK_READY;
    g_tasks[id].delayTicks = 0;

    return id;
}

void OS_DeleteTask(int32_t id)
{
    if (id < 0 || id >= OS_MAX_TASKS) {
        return;
    }

    g_tasks[id].state      = OS_TASK_UNUSED;
    g_tasks[id].entry      = NULL;
    g_tasks[id].arg        = NULL;
    g_tasks[id].priority   = 0;
    g_tasks[id].delayTicks = 0;

    if (g_currentTask == id) {
        g_currentTask = OS_INVALID_TASK;
    }
}

void OS_SuspendTask(int32_t id)
{
    if (id < 0 || id >= OS_MAX_TASKS) {
        return;
    }
    if (g_tasks[id].state == OS_TASK_UNUSED) {
        return;
    }

    /* 不管是 READY / RUNNING / DELAYED，都可以直接挂起 */
    g_tasks[id].state      = OS_TASK_SUSPENDED;
    g_tasks[id].delayTicks = 0;  /* 清除残留延时，下次恢复直接 READY */

    if (g_currentTask == id) {
        g_currentTask = OS_INVALID_TASK;
    }
}

void OS_ResumeTask(int32_t id)
{
    if (id < 0 || id >= OS_MAX_TASKS) {
        return;
    }
    if (g_tasks[id].state == OS_TASK_SUSPENDED) {
        g_tasks[id].state = OS_TASK_READY;
    }
}

void OS_Yield(void)
{
    if (g_currentTask < 0 || g_currentTask >= OS_MAX_TASKS) {
        return;
    }

    /* 把自己改回 READY，让调度器有机会调度别人 */
    if (g_tasks[g_currentTask].state == OS_TASK_RUNNING) {
        g_tasks[g_currentTask].state = OS_TASK_READY;
        g_currentTask = OS_INVALID_TASK;
    }
}

/* ============================================================
 *  非阻塞延时实现
 * ============================================================ */

void OS_DelayTicks(uint32_t ticks)
{
    if (ticks == 0) {
        return;
    }
    if (g_currentTask < 0 || g_currentTask >= OS_MAX_TASKS) {
        return;
    }

    g_tasks[g_currentTask].delayTicks = ticks;
    g_tasks[g_currentTask].state      = OS_TASK_DELAYED;
    g_currentTask = OS_INVALID_TASK;
}

void OS_DelayMs(uint32_t ms)
{
    if (ms == 0) {
        return;
    }

#if (OS_TICK_MS == 1)
    OS_DelayTicks(ms);
#else
    uint32_t ticks = ms / OS_TICK_MS;
    if (ticks == 0) {
        ticks = 1;  /* 至少延时 1 个 Tick */
    }
    OS_DelayTicks(ticks);
#endif
}

/* ============================================================
 *  状态查询接口实现
 * ============================================================ */

OS_TaskState_t OS_GetTaskState(int32_t id)
{
    if (id < 0 || id >= OS_MAX_TASKS) {
        return OS_TASK_UNUSED;
    }
    return g_tasks[id].state;
}

int32_t OS_GetCurrentTaskId(void)
{
    return g_currentTask;
}

uint8_t OS_GetReadyCount(void)
{
    uint8_t count = 0;
    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == OS_TASK_READY) {
            count++;
        }
    }
    return count;
}

/* ============================================================
 *  内部函数实现
 * ============================================================ */

/**
 * @brief  找一个空闲的任务槽位
 */
static int32_t OS_FindEmptySlot(void)
{
    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == OS_TASK_UNUSED) {
            return i;
        }
    }
    return OS_INVALID_TASK;
}

/**
 * @brief  从所有 READY 任务中选出优先级最高的一个
 *         优先级数值越大越优先
 */
static int32_t OS_SelectNextTask(void)
{
    int32_t bestId  = OS_INVALID_TASK;
    uint8_t bestPrio = 0;

    for (int i = 0; i < OS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == OS_TASK_READY) {
            if (bestId == OS_INVALID_TASK || g_tasks[i].priority > bestPrio) {
                bestId   = i;
                bestPrio = g_tasks[i].priority;
            }
        }
    }
    return bestId;
}

/**
 * @brief  调度一次：选任务 -> 调用其函数一次
 */
static void OS_ScheduleOnce(void)
{
    int32_t next = OS_SelectNextTask();
    if (next == OS_INVALID_TASK) {
        /* 没有 READY 任务，啥也不干 */
        return;
    }

    g_currentTask = next;
    g_tasks[next].state = OS_TASK_RUNNING;

    /* 调用任务函数：任务内部不要写 while(1)，只做一步逻辑就 return */
    g_tasks[next].entry(g_tasks[next].arg);

    /* 如果任务没自己改状态（比如延时/挂起/删除），默认跑完一次回到 READY */
    if (g_currentTask == next &&
        g_tasks[next].state == OS_TASK_RUNNING) {
        g_tasks[next].state = OS_TASK_READY;
        g_currentTask = OS_INVALID_TASK;
    } else {
        /* 任务在内部可能调用了 Delay/Suspend/Delete 等，尊重它的状态 */
        g_currentTask = OS_INVALID_TASK;
    }
}
