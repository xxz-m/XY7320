/**
 * @file    log_service.cpp
 * @brief   日志输出服务实现
 *          连接 nanoprintf（格式化）+ BspUart（串口发送）
 */
#include "log_service.h"
#include "bsp_uart.h"
#include "bsp_config.h"

/* nanoprintf 配置 */
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_ALT_FORM_FLAG 0
#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"

LogService& LogService::Instance()
{
    static LogService instance;
    return instance;
}

void LogService::Init()
{
    /* 配置 BSP 层日志输出串口 */
    BspUart_LogInit(&LOG_HUART);
}

/** nanoprintf 回调：每生成一个字符调一次 */
static void PutcCallback(int c, void *ctx)
{
    (void)ctx;
    BspUart_LogPutChar(static_cast<char>(c));
}

void LogService::VPrintf(const char *fmt, va_list args)
{
    if (fmt == nullptr) return;
    npf_vpprintf(PutcCallback, nullptr, fmt, args);
}

void LogService::Printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    VPrintf(fmt, args);
    va_end(args);
}
