//
// Created by Administrator on 2026/6/14.
//

#include "bsp_gpio.h"

void BSP_GPIO_Init(BspGpio_t *dev) {
    BSP_GPIO_Off(dev);
}

void BSP_GPIO_On(BspGpio_t *dev) {
    HAL_GPIO_WritePin(dev->port, dev->pin,
        dev->active_high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void BSP_GPIO_Off(BspGpio_t *dev) {
    HAL_GPIO_WritePin(dev->port, dev->pin,
        dev->active_high ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void BSP_GPIO_Toggle(BspGpio_t *dev) {
    HAL_GPIO_TogglePin(dev->port, dev->pin);
}

bool BSP_GPIO_IsOn(BspGpio_t *dev) {
    GPIO_PinState state = HAL_GPIO_ReadPin(dev->port, dev->pin);
    return (state == GPIO_PIN_SET) ? dev->active_high : !dev->active_high;
}


