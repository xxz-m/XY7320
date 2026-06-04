#include "bsp_log.h"
#include "usart.h"

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

static void BSP_LogPutcCallback(int c, void *ctx)
{
    (void)ctx;
    BSP_LogPutChar((char)c);
}

void BSP_LogPutChar(char ch)
{
    uint8_t data = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &data, 1, 100);
}

void BSP_LogWrite(const char *str)
{
    if (str == 0)
    {
        return;
    }

    while (*str != '\0')
    {
        BSP_LogPutChar(*str++);
    }
}

void BSP_LogVPrintf(const char *fmt, va_list args)
{
    if (fmt == 0)
    {
        return;
    }

    npf_vpprintf(BSP_LogPutcCallback, 0, fmt, args);
}

void BSP_LogPrintf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    BSP_LogVPrintf(fmt, args);
    va_end(args);
}
