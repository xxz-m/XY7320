/****************************************************************************
*  Copyright 2022 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

#ifndef __PERFORMANCE_COUNTER_H__
#define __PERFORMANCE_COUNTER_H__

/*============================ INCLUDES ======================================*/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "cmsis_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __PERF_COUNTER_VER_MAJOR__          2
#define __PERF_COUNTER_VER_MINOR__          2
#define __PERF_COUNTER_VER_REVISE__         4

#define __PERF_COUNTER_VER_STR__            ""

#define __PER_COUNTER_VER__    (__PERF_COUNTER_VER_MAJOR__ * 10000ul            \
                               +__PERF_COUNTER_VER_MINOR__ * 100ul              \
                               +__PERF_COUNTER_VER_REVISE__)

#undef __IS_COMPILER_IAR__
#if defined(__IAR_SYSTEMS_ICC__)
#   define __IS_COMPILER_IAR__                  1
#endif

#undef __IS_COMPILER_ARM_COMPILER_5__
#if ((__ARMCC_VERSION >= 5000000) && (__ARMCC_VERSION < 6000000))
#   define __IS_COMPILER_ARM_COMPILER_5__       1
#endif

#undef __IS_COMPILER_ARM_COMPILER_6__
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#   define __IS_COMPILER_ARM_COMPILER_6__       1
#endif
#undef __IS_COMPILER_ARM_COMPILER__
#if defined(__IS_COMPILER_ARM_COMPILER_5__) && __IS_COMPILER_ARM_COMPILER_5__   \
||  defined(__IS_COMPILER_ARM_COMPILER_6__) && __IS_COMPILER_ARM_COMPILER_6__
#   define __IS_COMPILER_ARM_COMPILER__         1
#endif

#undef  __IS_COMPILER_LLVM__
#if defined(__clang__) && !__IS_COMPILER_ARM_COMPILER_6__
#   define __IS_COMPILER_LLVM__                 1
#else
#   undef __IS_COMPILER_GCC__
#   if defined(__GNUC__) && !(  defined(__IS_COMPILER_ARM_COMPILER__)           \
                            ||  defined(__IS_COMPILER_LLVM__)                   \
                            ||  defined(__IS_COMPILER_IAR__))
#       define __IS_COMPILER_GCC__              1
#   endif
#endif

#ifdef __PERF_COUNT_PLATFORM_SPECIFIC_HEADER__
#   include __PERF_COUNT_PLATFORM_SPECIFIC_HEADER__
#endif

#ifndef __PLOOC_VA_NUM_ARGS_IMPL
#   define __PLOOC_VA_NUM_ARGS_IMPL( _0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,     \
                                    _12,_13,_14,_15,_16,__N,...)      __N
#endif

#ifndef __PLOOC_VA_NUM_ARGS
#define __PLOOC_VA_NUM_ARGS(...)                                                \
            __PLOOC_VA_NUM_ARGS_IMPL( 0,##__VA_ARGS__,16,15,14,13,12,11,10,9,   \
                                      8,7,6,5,4,3,2,1,0)
#endif

#ifndef UNUSED_PARAM
#   define UNUSED_PARAM(__VAR)     (void)(__VAR)
#endif

#undef __CONNECT2
#undef __CONNECT3
#undef __CONNECT4
#undef __CONNECT5
#undef __CONNECT6
#undef __CONNECT7
#undef __CONNECT8
#undef __CONNECT9
#undef CONNECT2
#undef CONNECT3
#undef CONNECT4
#undef CONNECT5
#undef CONNECT6
#undef CONNECT7
#undef CONNECT8
#undef CONNECT9
#undef CONNECT

#undef __MACRO_EXPANDING
#define __MACRO_EXPANDING(...)                      __VA_ARGS__

#define __CONNECT2(__A, __B)                        __A##__B
#define __CONNECT3(__A, __B, __C)                   __A##__B##__C
#define __CONNECT4(__A, __B, __C, __D)              __A##__B##__C##__D
#define __CONNECT5(__A, __B, __C, __D, __E)         __A##__B##__C##__D##__E
#define __CONNECT6(__A, __B, __C, __D, __E, __F)    __A##__B##__C##__D##__E##__F
#define __CONNECT7(__A, __B, __C, __D, __E, __F, __G)                           \
                                                    __A##__B##__C##__D##__E##__F##__G
#define __CONNECT8(__A, __B, __C, __D, __E, __F, __G, __H)                      \
                                                    __A##__B##__C##__D##__E##__F##__G##__H
#define __CONNECT9(__A, __B, __C, __D, __E, __F, __G, __H, __I)                 \
                                                    __A##__B##__C##__D##__E##__F##__G##__H##__I

#define ALT_CONNECT2(__A, __B)              __CONNECT2(__A, __B)
#define CONNECT2(__A, __B)                  __CONNECT2(__A, __B)
#define CONNECT3(__A, __B, __C)             __CONNECT3(__A, __B, __C)
#define CONNECT4(__A, __B, __C, __D)        __CONNECT4(__A, __B, __C, __D)
#define CONNECT5(__A, __B, __C, __D, __E)   __CONNECT5(__A, __B, __C, __D, __E)
#define CONNECT6(__A, __B, __C, __D, __E, __F)                                  \
                                            __CONNECT6(__A, __B, __C, __D, __E, __F)
#define CONNECT7(__A, __B, __C, __D, __E, __F, __G)                             \
                                            __CONNECT7(__A, __B, __C, __D, __E, __F, __G)
#define CONNECT8(__A, __B, __C, __D, __E, __F, __G, __H)                        \
                                            __CONNECT8(__A, __B, __C, __D, __E, __F, __G, __H)
#define CONNECT9(__A, __B, __C, __D, __E, __F, __G, __H, __I)                   \
                                            __CONNECT9(__A, __B, __C, __D, __E, __F, __G, __H, __I)

#define CONNECT(...)                                                            \
            ALT_CONNECT2(CONNECT, __PLOOC_VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#undef __using1
#undef __using2
#undef __using3
#undef __using4
#undef using

#define __using1(__declare)                                                     \
            for (__declare, *CONNECT3(__using_, __LINE__,_ptr) = NULL;          \
                 CONNECT3(__using_, __LINE__,_ptr)++ == NULL;                   \
                )

#define __using2(__declare, __on_leave_expr)                                    \
            for (__declare, *CONNECT3(__using_, __LINE__,_ptr) = NULL;          \
                 CONNECT3(__using_, __LINE__,_ptr)++ == NULL;                   \
                 (__on_leave_expr)                                              \
                )

#define __using3(__declare, __on_enter_expr, __on_leave_expr)                   \
            for (__declare, *CONNECT3(__using_, __LINE__,_ptr) = NULL;          \
                 CONNECT3(__using_, __LINE__,_ptr)++ == NULL ?                  \
                    ((__on_enter_expr),1) : 0;                                  \
                 (__on_leave_expr)                                              \
                )

#define __using4(__dcl1, __dcl2, __on_enter_expr, __on_leave_expr)              \
            for (__dcl1, __dcl2, *CONNECT3(__using_, __LINE__,_ptr) = NULL;     \
                 CONNECT3(__using_, __LINE__,_ptr)++ == NULL ?                  \
                    ((__on_enter_expr),1) : 0;                                  \
                 (__on_leave_expr)                                              \
                )

#define using(...)                                                              \
                CONNECT2(__using, __PLOOC_VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)


#undef __with2
#undef __with3
#undef with

#define __with2(__type, __addr)                                                 \
            using(__type *_=(__addr))
#define __with3(__type, __addr, __item)                                         \
            using(__type *_=(__addr), *__item = _, _=_,_=_ )

#define with(...)                                                               \
            CONNECT2(__with, __PLOOC_VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#undef _

#ifndef dimof
#   define dimof(__array)          (sizeof(__array)/sizeof(__array[0]))
#endif

#define SAFE_NAME(__NAME)   CONNECT3(__,__NAME,__LINE__)

#undef foreach2
#undef foreach3
#undef foreach

#define foreach2(__type, __array)                                               \
            using(__type *_ = __array)                                          \
            for (   uint_fast32_t SAFE_NAME(count) = dimof(__array);            \
                    SAFE_NAME(count) > 0;                                       \
                    _++, SAFE_NAME(count)--                                     \
                )

#define foreach3(__type, __array, __item)                                       \
            using(__type *_ = __array, *__item = _, _ = _, _ = _ )              \
            for (   uint_fast32_t SAFE_NAME(count) = dimof(__array);            \
                    SAFE_NAME(count) > 0;                                       \
                    _++, __item = _, SAFE_NAME(count)--                         \
                )

#define foreach(...)                                                            \
            CONNECT2(foreach, __PLOOC_VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#ifndef __IRQ_SAFE
#   define __IRQ_SAFE                                                           \
            using(  uint32_t SAFE_NAME(temp) =                                  \
                        ({  uint32_t SAFE_NAME(temp2)=__get_PRIMASK();          \
                            __disable_irq();                                    \
                            SAFE_NAME(temp2);}),                                \
                        __set_PRIMASK(SAFE_NAME(temp)))
#endif

#ifndef __perf_counter_printf__
#   define __perf_counter_printf__      printf
#endif

typedef struct {
    int64_t             lStart;
    int64_t             lUsedTotal;
    int32_t             nUsedRecent;
    uint16_t            hwActiveCount;
    uint16_t                        : 15;
    uint16_t            bEnabled    : 1;
} task_cycle_info_t;

typedef struct task_cycle_info_agent_t task_cycle_info_agent_t;

struct task_cycle_info_agent_t {
    task_cycle_info_t *ptInfo;
    task_cycle_info_agent_t *ptNext;
    task_cycle_info_agent_t *ptPrev;
};

/*============================ GLOBAL VARIABLES ==============================*/
extern volatile int64_t g_lLastTimeStamp;
extern volatile int32_t g_nOffset;

/*============================ PROTOTYPES ====================================*/

__attribute__((noinline))
extern int64_t get_system_ticks(void);

__STATIC_INLINE
void start_cycle_counter(void)
{
    g_lLastTimeStamp = get_system_ticks();
}

__STATIC_INLINE
int64_t stop_cycle_counter(void)
{
    int64_t lTemp = (get_system_ticks() - g_lLastTimeStamp);
    return lTemp - g_nOffset;
}

extern int32_t get_system_ms(void);
extern int32_t get_system_us(void);
extern void delay_us(int32_t nUs);
extern void delay_ms(int32_t nMs);
extern int64_t perfc_convert_ticks_to_ms(int64_t lTick);
extern int64_t perfc_convert_ms_to_ticks(uint32_t wMS);
extern int64_t perfc_convert_ticks_to_us(int64_t lTick);
extern int64_t perfc_convert_us_to_ticks(uint32_t wUS);
extern bool __perfc_is_time_out(int64_t lPeriod, int64_t *plTimestamp, bool bAutoReload);

#define start_task_cycle_counter(...)        start_cycle_counter()
#define stop_task_cycle_counter(...)         stop_cycle_counter()
#define init_task_cycle_counter(...)
#define register_task_cycle_agent(...)
#define unregister_task_cycle_agent(...)
#define init_task_cycle_info(...)            (NULL)
#define enable_task_cycle_info(...)          (false)
#define disable_task_cycle_info(...)         (false)
#define resume_task_cycle_info(...)
#define perfc_check_task_stack_canary_safe() (false)

extern void init_cycle_counter(bool bIsSysTickOccupied);
extern void user_code_insert_to_systick_handler(void);
extern void update_perf_counter(void);
extern void before_cycle_counter_reconfiguration(void);

#ifdef __cplusplus
}
#endif

#endif
