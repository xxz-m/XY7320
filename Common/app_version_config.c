#include "app_version_config.h"

#include "stm32f4xx_hal.h"
#include <string.h>

static uint32_t AppVersionConfig_ReadLe32(const uint8_t *data)
{
    return ((uint32_t)data[0]) |
           ((uint32_t)data[1] << 8U) |
           ((uint32_t)data[2] << 16U) |
           ((uint32_t)data[3] << 24U);
}

static int AppVersionConfig_ParseAsciiVersion(const uint8_t *data, uint64_t *version)
{
    uint64_t value = 0U;

    for (uint32_t i = 0; i < APP_VERSION_ASCII_LEN; i++)
    {
        if (data[i] < (uint8_t)'0' || data[i] > (uint8_t)'9')
        {
            return -1;
        }

        value = value * 10U + (uint64_t)(data[i] - (uint8_t)'0');
    }

    *version = value;
    return 0;
}

static void AppVersionConfig_MakeSlot(AppVersionSlot *slot, uint64_t version, uint8_t flag)
{
    memset(slot, 0xFF, sizeof(*slot));
    slot->magic = APP_VERSION_SLOT_MAGIC;
    slot->version = version;
    slot->flag = flag;
    memset(slot->reserved, 0xFF, sizeof(slot->reserved));
    slot->tail = APP_VERSION_SLOT_TAIL;
}

static int AppVersionConfig_WriteWord(uint32_t addr, uint32_t word)
{
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, word) != HAL_OK)
    {
        return -1;
    }

    if (*(volatile uint32_t *)addr != word)
    {
        return -1;
    }

    return 0;
}

static int AppVersionConfig_WriteSlot(uint32_t addr, const AppVersionSlot *slot)
{
    const uint32_t *words = (const uint32_t *)slot;

    for (uint32_t i = 0; i < (uint32_t)(sizeof(AppVersionSlot) / sizeof(uint32_t)); i++)
    {
        if (AppVersionConfig_WriteWord(addr + i * sizeof(uint32_t), words[i]) < 0)
        {
            return -1;
        }
    }

    return 0;
}

static int AppVersionConfig_Save(const AppVersionConfig *config)
{
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error = 0U;
    int ret = 0;

    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase_init.Sector = FLASH_SECTOR_11;
    erase_init.NbSectors = 1U;

    HAL_FLASH_Unlock();

    if (HAL_FLASHEx_Erase(&erase_init, &sector_error) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return -1;
    }

    if (AppVersionConfig_IsSlotValid(&config->a1))
    {
        ret = AppVersionConfig_WriteSlot(APP_VERSION_CONFIG_ADDR + APP_VERSION_SLOT_A1_OFFSET, &config->a1);
    }

    if (ret == 0 && AppVersionConfig_IsSlotValid(&config->a2))
    {
        ret = AppVersionConfig_WriteSlot(APP_VERSION_CONFIG_ADDR + APP_VERSION_SLOT_A2_OFFSET, &config->a2);
    }

    HAL_FLASH_Lock();
    return ret;
}

void AppVersionConfig_Read(AppVersionConfig *config)
{
    if (config == NULL)
    {
        return;
    }

    memcpy(&config->a1, (const void *)(APP_VERSION_CONFIG_ADDR + APP_VERSION_SLOT_A1_OFFSET), sizeof(AppVersionSlot));
    memcpy(&config->a2, (const void *)(APP_VERSION_CONFIG_ADDR + APP_VERSION_SLOT_A2_OFFSET), sizeof(AppVersionSlot));
}

int AppVersionConfig_IsSlotValid(const AppVersionSlot *slot)
{
    if (slot == NULL)
    {
        return 0;
    }

    return (slot->magic == APP_VERSION_SLOT_MAGIC) && (slot->tail == APP_VERSION_SLOT_TAIL);
}

int AppVersionConfig_UpdateA1(uint64_t version, uint8_t flag)
{
    AppVersionConfig config;
    AppVersionConfig_Read(&config);

    if (AppVersionConfig_IsSlotValid(&config.a1) &&
        config.a1.version == version &&
        config.a1.flag == flag)
    {
        return 0;
    }

    AppVersionConfig_MakeSlot(&config.a1, version, flag);
    return AppVersionConfig_Save(&config);
}

int AppVersionConfig_UpdateA2(uint64_t version, uint8_t flag)
{
    AppVersionConfig config;
    AppVersionConfig_Read(&config);

    if (AppVersionConfig_IsSlotValid(&config.a2) &&
        config.a2.version == version &&
        config.a2.flag == flag)
    {
        return 0;
    }

    AppVersionConfig_MakeSlot(&config.a2, version, flag);
    return AppVersionConfig_Save(&config);
}

int AppVersionConfig_ShouldEnterUpgrade(void)
{
    AppVersionConfig config;
    AppVersionConfig_Read(&config);

    if (!AppVersionConfig_IsSlotValid(&config.a1) || !AppVersionConfig_IsSlotValid(&config.a2))
    {
        return 0;
    }

    if (config.a2.flag != APP_VERSION_FLAG_NEED_DOWNLOAD)
    {
        return 0;
    }

    return config.a1.version < config.a2.version;
}

int AppVersionConfig_ParseFrame(const uint8_t *data, size_t len, uint64_t *version, uint8_t *flag)
{
    uint32_t head;
    uint32_t tail;

    if (data == NULL || version == NULL || flag == NULL || len < APP_VERSION_FRAME_SIZE)
    {
        return -1;
    }

    head = AppVersionConfig_ReadLe32(&data[0]);
    if (head != APP_VERSION_FRAME_HEAD)
    {
        return -1;
    }

    tail = AppVersionConfig_ReadLe32(&data[17]);
    if (tail != APP_VERSION_FRAME_TAIL)
    {
        return -1;
    }

    if (AppVersionConfig_ParseAsciiVersion(&data[4], version) < 0)
    {
        return -1;
    }

    *flag = data[16];
    return 0;
}
