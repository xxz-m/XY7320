#ifndef APP_VERSION_CONFIG_H
#define APP_VERSION_CONFIG_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_VERSION_CONFIG_ADDR          0x080E0000U
#define APP_VERSION_CONFIG_SIZE          0x00020000U
#define APP_VERSION_CONFIG_END_ADDR      (APP_VERSION_CONFIG_ADDR + APP_VERSION_CONFIG_SIZE)

#define APP_VERSION_SLOT_A1_OFFSET       0x00000000U
#define APP_VERSION_SLOT_A2_OFFSET       0x00000040U

#define APP_VERSION_SLOT_MAGIC           0x41564346U
#define APP_VERSION_SLOT_TAIL            0x46435641U

#define APP_VERSION_FRAME_HEAD           0x48565958U
#define APP_VERSION_FRAME_TAIL           0x54565958U
#define APP_VERSION_FRAME_SIZE           21U
#define APP_VERSION_ASCII_LEN            12U

#define APP_VERSION_FLAG_NEED_DOWNLOAD   0x00U
#define APP_VERSION_FLAG_DOWNLOADED      0x01U

typedef struct
{
    uint32_t magic;
    uint64_t version;
    uint8_t flag;
    uint8_t reserved[7];
    uint32_t tail;
} AppVersionSlot;

typedef struct
{
    AppVersionSlot a1;
    AppVersionSlot a2;
} AppVersionConfig;

void AppVersionConfig_Read(AppVersionConfig *config);
int AppVersionConfig_IsSlotValid(const AppVersionSlot *slot);
int AppVersionConfig_UpdateA1(uint64_t version, uint8_t flag);
int AppVersionConfig_UpdateA2(uint64_t version, uint8_t flag);
int AppVersionConfig_ShouldEnterUpgrade(void);
int AppVersionConfig_ParseFrame(const uint8_t *data, size_t len, uint64_t *version, uint8_t *flag);

#ifdef __cplusplus
}
#endif

#endif
