/**
 * \file            bootloader_port.c
 * \brief           Bootloader 可移植接口 — 阶段二简化版
 *
 * 阶段二目标：先让工程编译通过，Flash 读写擦跑通。
 * Init / HostDataProcess / Reset 暂时留空，待阶段三接入 BSP Timer/UART/DT/PP 后再补。
 *
 * 保留原版版权声明。
 */

/*
 * Copyright (c) 2023 Dino Haw
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of mOTA - The Over-The-Air technology component for MCU.
 *
 * Author:          Dino Haw <347341799@qq.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "bootloader.h"


/* Exported functions ---------------------------------------------------------*/
/**
 * @brief  通讯与协议相关初始化（阶段二留空）
 */
void Bootloader_Port_Init(void)
{
    /* 阶段二暂不初始化 UART/Timer/DT/PP，阶段三再补 */
}


/**
 * @brief  主机数据接收处理函数（阶段二留空）
 */
void Bootloader_Port_HostDataProcess(void)
{
    /* 阶段二暂无数据接收逻辑，阶段三接入 YModem 数据链路后再补 */
}


/**
 * @brief  复位内部状态（阶段二留空）
 */
void Bootloader_Port_Reset(void)
{
    /* 阶段二暂无明显状态需要复位 */
}


/**
 * @brief  跳转至 APP
 *
 * 关闭中断、复位外设、设置 MSP / VTOR，然后跳转到 APP 的 Reset_Handler。
 * APP 地址由 bootloader_define.h 中的 APP_ADDRESS 宏指定。
 */
__NO_RETURN
void Bootloader_Port_JumpToAPP(void)
{
    uint32_t stack_addr;
    uint32_t reset_handler;

    /* 读取 APP 分区的栈顶地址和复位向量 */
    stack_addr    = *(volatile uint32_t *)APP_ADDRESS;
    reset_handler = *(volatile uint32_t *)(APP_ADDRESS + 4);

    /* 关闭全局中断 */
    __disable_irq();

    /* 关闭滴答定时器 */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* 关闭所有中断，清除挂起标志 */
    for (uint8_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    /* 设置 MSP 为 APP 的栈顶 */
    __set_MSP(stack_addr);

    /* 特权模式，使用 MSP */
    __set_CONTROL(0);

    /* 重映射中断向量表到 APP 首地址 */
    SCB->VTOR = APP_ADDRESS;

    /* 跳转到 APP 的 Reset_Handler */
    ((void (*)(void))reset_handler)();

    /* 不会执行到这里 */
    while (1) {}
}


/**
 * @brief  系统复位重启
 */
void Bootloader_Port_SystemReset(void)
{
    HAL_NVIC_SystemReset();
}

