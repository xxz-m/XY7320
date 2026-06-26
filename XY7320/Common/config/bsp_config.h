/**
 * @file    bsp_config.h
 * @brief   BSP 硬件配置
 *          集中管理所有外设选择，修改硬件时只改这一个文件
 *
 * @note    仅供 BSP 层和依赖 BSP 的 Services 使用
 *          Domain 层禁止 include 此文件
 */

#ifndef XY7320_BSP_CONFIG_H
#define XY7320_BSP_CONFIG_H

#include "usart.h"
#include "bsp_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ====== 串口配置 ====== */

/** 日志输出串口（调试用） */
#define LOG_HUART           huart1

/** 升级通信串口（上位机版本帧） */
#define UPGRADE_HUART       huart2

/* ====== GPIO 配置 ====== */

/** LED 指示灯 GPIO */
#define LED_GPIO_PORT       GPIOF
#define LED_GPIO_PIN        GPIO_PIN_10
#define LED_ACTIVE_HIGH     true

/* ====== SPI 配置（预留） ====== */
// #define FLASH_SPI           hspi1

/* ====== ADC 配置 ====== */
#define BSP_ADC_CHANNEL_COUNT          8u
#define BSP_ADC_ACTIVE_CHANNEL_COUNT   6u
#define BSP_ADC_IDLE_CHANNEL_COUNT     2u
#define BSP_ADC_DMA_BUF_LEN            512u
#define BSP_ADC_SCAN_COUNT_PER_DMA     64u
#define BSP_ADC_TARGET_SAMPLE_COUNT    512u
#define BSP_ADC_DMA_ROUND_COUNT        8u
#define BSP_ADC_SAMPLE_A_COUNT           6u
#define BSP_ADC_SAMPLE_B_COUNT           2u
#ifdef __cplusplus
}
#endif

#endif /* XY7320_BSP_CONFIG_H */
