//
// Created by XYKJ on 2026/6/15.
//

#ifndef XY7320_BSP_FLASH_H
#define XY7320_BSP_FLASH_H
#include <stdint.h>
#include "main.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * 读取 Flash 数据（内部 Flash 直接 memcpy）
 */
int BspFlash_Read(uint32_t addr, uint8_t *buf, uint32_t len);
/**
 * 写入 Flash 数据（按字编程）
 */
int BspFlash_Write(uint32_t addr, const uint8_t *buf, uint32_t len);
/**
 * 擦除一个 Flash 扇区
 */
int BspFlash_EraseSector(uint8_t sector);
#ifdef __cplusplus
}
#endif
#endif //XY7320_BSP_FLASH_H
