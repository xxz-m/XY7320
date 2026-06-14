$ErrorActionPreference = 'Stop'

$headerPath = 'E:\XY7320\XY7320\XY_bootloadr\Core\Inc\app_version_config.h'
$sourcePath = 'E:\XY7320\XY7320\XY_bootloadr\Core\Src\app_version_config.c'
$updatePath = 'E:\XY7320\XY7320\XY_bootloadr\MOTA\Core\simple_update.c'

$header = Get-Content -Raw -Path $headerPath -Encoding UTF8
$header = $header -replace '#include <stdint.h>', "#include <stdint.h>`r`n#include <stddef.h>"
$header = $header -replace 'int AppVersionConfig_IsSlotValid\(const AppVersionSlot \*slot\);\r?\nint AppVersionConfig_ShouldEnterUpgrade\(void\);', "int AppVersionConfig_IsSlotValid(const AppVersionSlot *slot);`r`nint AppVersionConfig_UpdateA2(uint64_t version, uint8_t flag);`r`nint AppVersionConfig_ShouldEnterUpgrade(void);"
Set-Content -Path $headerPath -Value $header -Encoding UTF8

$source = Get-Content -Raw -Path $sourcePath -Encoding UTF8
$insert = @"
static uint32_t AppVersionConfig_ReadLe32(const uint8_t *data)
{
    return ((uint32_t)data[0]) |
           ((uint32_t)data[1] << 8U) |
           ((uint32_t)data[2] << 16U) |
           ((uint32_t)data[3] << 24U);
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
    FLASH_EraseInitTypeDef erase_init = {0};
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

"@
$source = $source -replace '#include <string.h>?
', "#include <string.h>`r`n`r`n$insert"
$source = $source -replace 'int AppVersionConfig_IsSlotValid\(const AppVersionSlot \*slot\)\r?\n\{[\s\S]*?return \(slot->magic == APP_VERSION_SLOT_MAGIC\) && \(slot->tail == APP_VERSION_SLOT_TAIL\);\r?\n\}', @"
int AppVersionConfig_IsSlotValid(const AppVersionSlot *slot)
{
    if (slot == NULL)
    {
        return 0;
    }

    return (slot->magic == APP_VERSION_SLOT_MAGIC) && (slot->tail == APP_VERSION_SLOT_TAIL);
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
"@
Set-Content -Path $sourcePath -Value $source -Encoding UTF8

$update = Get-Content -Raw -Path $updatePath -Encoding UTF8
$update = $update -replace '#include "bsp_flash.h"', "#include `"bsp_flash.h`"`r`n#include `"app_version_config.h`""
$hook = @"
                if (Simple_Update_CheckAppValid())
                {
                    BSP_Printf("[simple update] APP valid\r\n");

                    {
                        AppVersionConfig version_config;

                        AppVersionConfig_Read(&version_config);
                        if (AppVersionConfig_IsSlotValid(&version_config.a2))
                        {
                            if (AppVersionConfig_UpdateA2(version_config.a2.version, APP_VERSION_FLAG_DOWNLOADED) == 0)
                            {
                                BSP_Printf("[simple update] version flag cleared\r\n");
                            }
                            else
                            {
                                BSP_Printf("[simple update] version flag clear fail\r\n");
                            }
                        }
                    }

                    BSP_Printf("[simple update] update ok, jump app\r\n");
"@
$update = $update -replace 'if \(Simple_Update_CheckAppValid\(\)\)\r?\n\s*\{\r?\n\s*BSP_Printf\("\[simple update\] APP valid\\r\\n"\);\r?\n\s*BSP_Printf\("\[simple update\] update ok, jump app\\r\\n"\);', $hook
Set-Content -Path $updatePath -Value $update -Encoding UTF8
