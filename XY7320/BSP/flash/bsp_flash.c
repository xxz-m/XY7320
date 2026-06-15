//
// Created by XYKJ on 2026/6/15.
//

#include "bsp_flash.h"
#include <string.h>

/**
 * 读取内部 Flash 数据
 * 
 * 原理：STM32 内部 Flash 是直接映射到内存地址空间的，
 *       所以读取 Flash 就像读取普通内存一样，直接 memcpy 即可。
 * 
 * @param addr  Flash 地址（如 0x080E0000）
 * @param buf   目标缓冲区，读取的数据会拷贝到这里
 * @param len   读取字节数
 * @return 0 成功, -1 参数错误
 */
int BspFlash_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    if (buf == NULL || len == 0) {
        return -1;
    }
    
    // volatile 告诉编译器不要优化这次读取，因为 Flash 内容可能被修改
    // 这里直接把 Flash 地址的内容拷贝到 buf
    memcpy(buf, (const volatile void *)addr, len);
    return 0;
}

/**
 * 写入内部 Flash 数据
 * 
 * 原理：STM32 Flash 写入有以下限制：
 *       1. 写入前必须解锁（HAL_FLASH_Unlock）
 *       2. 只能按字（4字节）编程，不能按字节写
 *       3. 只能把 1 写成 0，不能把 0 写成 1（要写 1 必须先擦除）
 *       4. 写入后必须回读校验，确保写入成功
 * 
 * @param addr  Flash 地址（必须已经擦除过，全 0xFF）
 * @param buf   源数据
 * @param len   写入字节数（必须是 4 的倍数）
 * @return 0 成功, -1 失败（参数错误、写入失败、校验失败）
 */
int BspFlash_Write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    // 参数检查：Flash 只能按 4 字节写入
    if (buf == NULL || len == 0 || (len % 4) != 0) {
        return -1;
    }

    // 解锁 Flash，允许写入操作
    // STM32 默认锁定 Flash 防止误操作，写入前必须解锁
    HAL_FLASH_Unlock();

    // 每次写 4 字节（一个字）
    for (uint32_t i = 0; i < len; i += 4) {
        uint32_t word;
        // 从 buf 中取出 4 字节，组成一个 32 位字
        memcpy(&word, buf + i, sizeof(word));

        // 调用 HAL 写入一个字
        // FLASH_TYPEPROGRAM_WORD 表示按字编程
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, word) != HAL_OK) {
            HAL_FLASH_Lock();  // 写入失败，锁定 Flash
            return -1;
        }

        // 回读校验：确保写入的内容和预期一致
        // Flash 写入可能因为电压不稳等原因失败，必须校验
        if (*(volatile uint32_t *)(addr + i) != word) {
            HAL_FLASH_Lock();  // 校验失败，锁定 Flash
            return -1;
        }
    }

    // 写入完成，锁定 Flash，防止后续误操作
    HAL_FLASH_Lock();
    return 0;
}

/**
 * 擦除一个 Flash 扇区
 * 
 * 原理：STM32 Flash 按扇区组织，擦除必须按扇区进行。
 *       擦除后整个扇区内容变为 0xFF，之后才能写入新数据。
 * 
 * STM32F407 扇区划分（1MB 版本）：
 *   Sector 0-3:   各 16KB  (0x08000000 - 0x0800FFFF)  Bootloader 用
 *   Sector 4:     64KB     (0x08010000 - 0x0801FFFF)  APP 起始
 *   Sector 5-7:   各 128KB (0x08020000 - 0x0807FFFF)
 *   Sector 8-11:  各 128KB (0x08080000 - 0x080FFFFF)
 * 
 * @param sector  扇区编号（0-11）
 * @return 0 成功, -1 擦除失败
 */
int BspFlash_EraseSector(uint8_t sector)
{
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error = 0;  // 擦除出错时，这里会记录出错的扇区号

    // 配置擦除参数
    erase_init.TypeErase   = FLASH_TYPEERASE_SECTORS;  // 按扇区擦除
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;   // 电压范围 2.7V-3.6V（我们的板子）
    erase_init.Sector      = sector;                   // 要擦除的扇区号
    erase_init.NbSectors   = 1;                        // 擦除 1 个扇区

    // 解锁 Flash
    HAL_FLASH_Unlock();

    // 执行擦除
    // sector_error 用于接收擦除过程中出错的扇区号（如果有的话）
    if (HAL_FLASHEx_Erase(&erase_init, &sector_error) != HAL_OK) {
        HAL_FLASH_Lock();
        return -1;
    }

    // 擦除完成，锁定 Flash
    HAL_FLASH_Lock();
    return 0;
}
