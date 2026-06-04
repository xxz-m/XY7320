#ifndef __BSP_LOG_H__
#define __BSP_LOG_H__

#include <stdarg.h>
#include <stdint.h>

void BSP_LogPrintf(const char *fmt, ...);
void BSP_LogVPrintf(const char *fmt, va_list args);
void BSP_LogPutChar(char ch);
void BSP_LogWrite(const char *str);

#endif /* __BSP_LOG_H__ */
