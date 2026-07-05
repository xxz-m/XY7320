/**
 * @file    app_config.h
 * @brief   APP 功能配置
 *          APP 版本号、功能开关等
 */

#ifndef XY7320_APP_CONFIG_H
#define XY7320_APP_CONFIG_H

#include <stdint.h>
#include "log_service.h"

#ifdef __cplusplus

/**
 * 当前 APP 版本号
 * 格式：yyyyMMddHHmm，如 202606082257 表示 2026-06-08 22:57
 *
 * 修改版本号后，UpdateService::Init() 会写 A1 = APP_CURRENT_VERSION。
 */
#define APP_CURRENT_VERSION  202606082257ULL

/** 调试输出开关（发布时改为 0） */
#define ENABLE_DEBUG_PRINT  1

#if ENABLE_DEBUG_PRINT
#define LOG_Printf(...)  LogService::Instance().Printf(__VA_ARGS__)
#else
#define LOG_Printf(...)
#endif

#endif /* __cplusplus */

#endif /* XY7320_APP_CONFIG_H */
