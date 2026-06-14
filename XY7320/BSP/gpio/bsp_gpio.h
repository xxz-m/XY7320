//
// Created by Administrator on 2026/6/14.
//

#ifndef XY7320_BSP_GPIO_H
#define XY7320_BSP_GPIO_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdbool.h"

/**
 * GPIO 设备句柄
 */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    bool active_high;
} BspGpio_t;
/**
 * 基础操作
 */
void BSP_GPIO_Init(BspGpio_t *dev);
void BSP_GPIO_On(BspGpio_t *dev);
void BSP_GPIO_Off(BspGpio_t *dev);
void BSP_GPIO_Toggle(BspGpio_t *dev);
bool BSP_GPIO_IsOn(BspGpio_t *dev);



#ifdef __cplusplus
}
#endif
#endif //XY7320_BSP_GPIO_LED_H
