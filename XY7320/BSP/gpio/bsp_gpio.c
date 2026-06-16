/**
 * @file    bsp_gpio.c
 * @brief   GPIO 驱动实现
 *          基于 HAL_GPIO 封装引脚的开关、翻转和状态读取
 */

#include "bsp_gpio.h"

void BspGpio_Init(BspGpio_t *dev)
{
    if (dev == NULL) return;
    BspGpio_Off(dev);
}

void BspGpio_On(BspGpio_t *dev)
{
    if (dev == NULL) return;
    HAL_GPIO_WritePin(dev->port, dev->pin,
        dev->active_high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void BspGpio_Off(BspGpio_t *dev)
{
    if (dev == NULL) return;
    HAL_GPIO_WritePin(dev->port, dev->pin,
        dev->active_high ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void BspGpio_Toggle(BspGpio_t *dev)
{
    if (dev == NULL) return;
    HAL_GPIO_TogglePin(dev->port, dev->pin);
}

bool BspGpio_IsOn(BspGpio_t *dev)
{
    if (dev == NULL) return false;
    GPIO_PinState state = HAL_GPIO_ReadPin(dev->port, dev->pin);
    return (state == GPIO_PIN_SET) ? dev->active_high : !dev->active_high;
}
