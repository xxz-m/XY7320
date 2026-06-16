/**
 * @file    bsp_gpio.h
 * @brief   GPIO 驱动
 *          封装 GPIO 引脚的初始化、开关、翻转和状态读取
 */

#ifndef XY7320_BSP_GPIO_H
#define XY7320_BSP_GPIO_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GPIO 设备句柄
 *
 * 封装一个 GPIO 引脚的端口、引脚号和有效电平。
 * 调用 BspGpio_Init() 前必须填充好这三个字段。
 */
typedef struct {
    GPIO_TypeDef *port;       ///< GPIO 端口（如 GPIOF）
    uint16_t     pin;         ///< 引脚号（如 GPIO_PIN_10）
    bool         active_high; ///< true=高电平有效, false=低电平有效
} BspGpio_t;

/**
 * @brief  初始化 GPIO（默认关闭输出）
 * @param  dev  GPIO 设备句柄（不能为 NULL）
 */
void BspGpio_Init(BspGpio_t *dev);

/**
 * @brief  打开 GPIO（设置为有效电平）
 * @param  dev  GPIO 设备句柄
 */
void BspGpio_On(BspGpio_t *dev);

/**
 * @brief  关闭 GPIO（设置为无效电平）
 * @param  dev  GPIO 设备句柄
 */
void BspGpio_Off(BspGpio_t *dev);

/**
 * @brief  翻转 GPIO 输出电平
 * @param  dev  GPIO 设备句柄
 */
void BspGpio_Toggle(BspGpio_t *dev);

/**
 * @brief  读取 GPIO 当前有效状态
 * @param  dev  GPIO 设备句柄
 * @return true=当前为有效电平, false=当前为无效电平
 */
bool BspGpio_IsOn(BspGpio_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* XY7320_BSP_GPIO_H */
