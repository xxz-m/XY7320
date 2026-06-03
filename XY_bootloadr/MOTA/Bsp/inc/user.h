/**
 * \file            user.h
 * \brief           XY_bootloadr 用户配置文件
 *
 * 由 mOTA common.h 引入，提供工程级配置选项。
 */

/*
 * Copyright (c) 2022 Dino Haw
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of mOTA - The Over-The-Air technology component for MCU.
 */

#ifndef __USER_H__
#define __USER_H__

/* ====== 基本开关 ====== */
#define ENABLE_ASSERT                       0       /* 阶段二暂不开启断言 */
#define ENABLE_DEBUG_PRINT                  1       /* 开启调试日志输出 */
#define EANBLE_PRINTF_USING_RTT             0       /* 不使用 SEGGER RTT，用 UART 输出 */

/* ====== RTOS ====== */
#define RTOS_USING_NONE                     0
#define USING_RTOS_TYPE                     RTOS_USING_NONE

/* ====== 通用 ====== */
#define MAX_NAME_LEN                        8
#define SEGGER_RTT_PRINTF_TERMINAL          0

#endif
