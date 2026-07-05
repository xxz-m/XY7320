/**
 * @file    bsp_flash.h
 * @brief   内部 Flash 读写擦除 BSP 模块
 *
 *          基于 STM32 HAL FLASH 接口，提供按字节读取、按字编程写入、
 *          按扇区擦除三类原子操作。供 Services 层（如 VersionStore）调用。
 *
 * @note    写入 / 擦除前必须确保对应地址已解锁（内部已处理）；
 *          写入前必须先擦除（HAL FLASH 不会自动擦除）。
 */

#ifndef XY7320_BSP_FLASH_H
#define XY7320_BSP_FLASH_H

#include <stdint.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  从内部 Flash 拷贝数据到缓冲区
 *
 * 内部 Flash 是只读可直接 memcpy，无需解锁。
 *
 * @param  addr  Flash 起始地址（必须 4 字节对齐且落在有效范围内）
 * @param  buf   目标缓冲区（调用方保证容量 >= len）
 * @param  len   字节数
 * @return 0 成功；-1 参数非法（addr / buf 异常）
 */
int BspFlash_Read(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * @brief  按字编程写入数据到 Flash
 *
 * 写入前**必须先擦除**所在扇区，否则 HAL FLASH 会返回错误。
 * 函数内部会处理 FLASH 解锁 / 锁回。
 *
 * @param  addr  Flash 起始地址（必须 4 字节对齐）
 * @param  buf   源数据
 * @param  len   字节数
 * @return 0 成功；-1 参数非法或 HAL FLASH 写入失败
 */
int BspFlash_Write(uint32_t addr, const uint8_t *buf, uint32_t len);

/**
 * @brief  擦除指定扇区
 *
 * 扇区号与 STM32F407 参考手册保持一致（扇区 0..11）。
 *
 * @param  sector  扇区号（0..11）
 * @return 0 成功；-1 参数非法或 HAL FLASH 擦除失败
 */
int BspFlash_EraseSector(uint8_t sector);

#ifdef __cplusplus
}
#endif
#endif /* XY7320_BSP_FLASH_H */