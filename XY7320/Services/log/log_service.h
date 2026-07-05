/**
 * @file    log_service.h
 * @brief   日志输出服务
 *
 *          封装 nanoprintf 格式化 + BSP 串口发送，对上层提供 LOG_Printf 调试输出。
 *
 *          调用链：
 *              LOG_Printf(...) → LogService::Printf(...)
 *                                    ├── nanoprintf       (Middleware: 格式化)
 *                                    └── BspUart_LogPutChar (BSP: 串口发送)
 */

#ifndef XY7320_LOG_SERVICE_H
#define XY7320_LOG_SERVICE_H

#include <cstdarg>

/**
 * @brief 日志输出服务
 *
 * 单例，通过 Instance() 获取实例。
 */
class LogService
{
public:
    /**
     * @brief 获取单例
     */
    static LogService& Instance();

    /**
     * @brief 初始化日志服务，配置串口层日志串口
     *
     * 在 OS 启动前由 App_Main::App_Main_Init 调用。
     */
    void Init();

    /**
     * @brief 格式化输出（va_args 版本）
     *
     * @param fmt  printf 风格格式字符串
     * @param ...  可变参数
     */
    void Printf(const char *fmt, ...);

    /**
     * @brief va_list 版本，供 Printf 内部及外部包装器调用
     *
     * @param fmt   格式字符串
     * @param args  va_list 起始
     */
    void VPrintf(const char *fmt, va_list args);

private:
    LogService() = default;
};
#endif /* XY7320_LOG_SERVICE_H */