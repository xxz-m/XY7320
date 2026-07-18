#include "main.h"
#include "string.h"
/*
 * STM32F407 分区：Sector 0-3 为 Bootloader，Sector 4-10 为 APP，
 * Sector 11 从 0x080E0000 开始保留给版本配置，因此升级操作不得越界。
 */
#define FLASH_BASE_ADDR      0x08000000U
#define APP_START_ADDR       0x08010000U
#define FLASH_END_ADDR       0x080E0000U

/**
 * @brief 从 APP Flash 分区读取数据。
 *
 * @param offset 绝对 Flash 地址，不是相对 APP 分区偏移。
 * @param buf    输出缓冲区。
 * @param size   读取字节数。
 * @return 成功返回读取字节数，地址或参数越界返回 -1。
 */
int read(long offset, uint8_t *buf, size_t size)
{
    if (buf == NULL || size == 0) {
        return -1;
    }

    if ((uint32_t)offset < APP_START_ADDR ||
        ((uint32_t)offset + size) > FLASH_END_ADDR) {
        return -1;
        }

    memcpy(buf, (const void *)offset, size);

    return (int)size;
}

/**
 * @brief 向 APP Flash 分区写入数据并回读校验。
 *
 * Flash 控制器按 4 字节写入；调用方须提供 4 字节对齐地址，尾部不足
 * 4 字节时由本实现使用 0xFF 补齐后写入。
 *
 * @param offset 绝对 Flash 地址。
 * @param buf    输入数据。
 * @param size   逻辑数据长度，单位为字节。
 * @return 成功返回逻辑数据长度，写入或校验失败返回 -1。
 */
int write(long offset, const uint8_t *buf, size_t size)
{
    uint32_t addr = (uint32_t)offset;
    uint32_t i = 0;
    uint32_t word;

    if (buf == NULL || size == 0) {
        return -1;
    }

    if (addr < APP_START_ADDR || (addr + size) > FLASH_END_ADDR) {
        return -1;
    }

    if ((addr % 4U) != 0U) {
        return -1;
    }

    HAL_FLASH_Unlock();

    while (i < size) {
        word = 0xFFFFFFFFU;

        for (uint32_t j = 0; j < 4U; j++) {
            if ((i + j) < size) {
                word &= ~((uint32_t)0xFFU << (8U * j));
                word |= ((uint32_t)buf[i + j] << (8U * j));
            }
        }

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, word) != HAL_OK) {
            HAL_FLASH_Lock();
            return -1;
        }

        if (*(volatile uint32_t *)(addr + i) != word) {
            HAL_FLASH_Lock();
            return -1;
        }

        i += 4U;
    }

    HAL_FLASH_Lock();

    return (int)size;
}

/**
 * @brief 根据绝对 Flash 地址映射 STM32F407 Sector。
 *
 * 这些边界对应 F407 的 Sector 0-11；升级擦除仅允许 Sector 4-10，
 * 以保护 Bootloader 和版本配置区。
 */
uint32_t get_sector(uint32_t addr)
{
    if (addr < 0x08004000U) return FLASH_SECTOR_0;
    if (addr < 0x08008000U) return FLASH_SECTOR_1;
    if (addr < 0x0800C000U) return FLASH_SECTOR_2;
    if (addr < 0x08010000U) return FLASH_SECTOR_3;
    if (addr < 0x08020000U) return FLASH_SECTOR_4;
    if (addr < 0x08040000U) return FLASH_SECTOR_5;
    if (addr < 0x08060000U) return FLASH_SECTOR_6;
    if (addr < 0x08080000U) return FLASH_SECTOR_7;
    if (addr < 0x080A0000U) return FLASH_SECTOR_8;
    if (addr < 0x080C0000U) return FLASH_SECTOR_9;
    if (addr < 0x080E0000U) return FLASH_SECTOR_10;

    return FLASH_SECTOR_11;
}

int erase(long offset, size_t size)
{
    uint32_t addr = (uint32_t)offset;
    uint32_t first_sector;
    uint32_t last_sector;
    uint32_t sector_error = 0;
    FLASH_EraseInitTypeDef erase_init;

    if (size == 0) {
        return -1;
    }

    if (addr < APP_START_ADDR || (addr + size) > FLASH_END_ADDR) {
        return -1;
    }

    first_sector = get_sector(addr);
    last_sector = get_sector(addr + size - 1);

    if (first_sector < FLASH_SECTOR_4 || last_sector > FLASH_SECTOR_10) {
        return -1;
    }

    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase_init.Sector = first_sector;
    erase_init.NbSectors = last_sector - first_sector + 1;

    HAL_FLASH_Unlock();

    if (HAL_FLASHEx_Erase(&erase_init, &sector_error) != HAL_OK) {
        HAL_FLASH_Lock();
        return -1;
    }

    HAL_FLASH_Lock();

    return (int)size;
}
