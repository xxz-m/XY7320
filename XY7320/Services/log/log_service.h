/**
* @file    log_service.h
 * @brief   日志输出服务
 *          封装 nanoprintf 格式化 + BSP 串口发送，提供 BSP_Printf 调试输出
 */
#ifndef XY7320_LOG_SERVICE_H
#define XY7320_LOG_SERVICE_H

#include <cstdarg>
/**
 * 日志输出服务
 *
 * 职责：把格式化字符串通过串口输出
 *
 * 调用链：
 *   LOG_Printf(...) → LogService::Printf(...)
 *                          ├── nanoprintf        (Middleware: 格式化)
 *                          └── BspUart_LogPutChar (BSP: 串口发送)
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class LogService
{
public:
  static LogService& Instance();
    /*初始化日志服务（配置串口层日志串口）*/
    void Init();
    /** 格式化输出 */
    void Printf(const char *fmt, ...);
    /** va_list 版本 */
    void VPrintf(const char *fmt, va_list args);
private:
    LogService()=default;
};
#endif //XY7320_LOG_SERVICE_H
