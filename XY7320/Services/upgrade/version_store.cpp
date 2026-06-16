/**
 * @file    version_store.cpp
 * @brief   版本配置存储实现
 *
 * 本模块是 Services 层，负责：
 * - 管理 Flash 中的版本配置（A1/A2 槽位）
 * - 提供"读/写/判断"等业务接口
 *
 * 本模块不直接操作 Flash 硬件，而是通过 BspFlash_* 接口：
 * - BspFlash_Read   → 读取 Flash 数据
 * - BspFlash_Write  → 写入 Flash 数据
 * - BspFlash_EraseSector → 擦除 Flash 扇区
 *
 * 设计原则：
 * - Services 层知道"存什么、怎么组织"（业务逻辑）
 * - BSP 层知道"怎么操作 Flash"（硬件细节）
 */

#include "version_store.h"
#include "bsp_flash.h"
#include <cstring>

/** 魔数：用于标识槽位是否有效 */
static constexpr uint32_t SLOT_MAGIC = 0x41564346U;  /* "AVCF" */
static constexpr uint32_t SLOT_TAIL  = 0x46435641U;  /* "FCVA" */

/** Flash 布局常量 */
static constexpr uint32_t CONFIG_ADDR   = 0x080E0000U;  /* 版本配置存储地址 */
static constexpr uint8_t  CONFIG_SECTOR = 11;            /* 对应 Flash 扇区号 */
static constexpr uint32_t A1_OFFSET     = 0x00000000U;  /* A1 槽位偏移 */
static constexpr uint32_t A2_OFFSET     = 0x00000040U;  /* A2 槽位偏移（64字节） */

VersionStore& VersionStore::Instance()
{
    static VersionStore instance;
    return instance;
}

/**
 * 构造一个有效的版本槽位
 *
 * 槽位结构（32 字节）：
 * ┌────────┬─────────┬──────┬───────────┬────────┐
 * │ magic  │ version │ flag │ reserved  │  tail  │
 * │ (4)    │  (8)    │ (1)  │   (7)     │  (4)   │
 * └────────┴─────────┴──────┴───────────┴────────┘
 *
 * magic 和 tail 用于判断槽位是否有效（防止读到未初始化的 Flash 区域）
 */
void VersionStore::MakeSlot(Slot &slot, uint64_t version, uint8_t flag)
{
    memset(&slot, 0, sizeof(slot));
    slot.magic = SLOT_MAGIC;
    slot.version = version;
    slot.flag = flag;
    memset(slot.reserved, 0xFF, sizeof(slot.reserved));
    slot.tail = SLOT_TAIL;
}

/**
 * 判断槽位是否有效
 * 通过检查 magic 和 tail 是否匹配来判断
 * 如果 Flash 从未写入过（全 0xFF），则无效
 */
bool VersionStore::IsSlotValid(const Slot &slot)
{
    return (slot.magic == SLOT_MAGIC) && (slot.tail == SLOT_TAIL);
}

/**
 * 从 Flash 读取完整配置（A1 + A2）
 * Flash 地址：0x080E0000，读取长度：64 字节（32 + 32）
 */
bool VersionStore::Read(Config &config)
{
    return BspFlash_Read(CONFIG_ADDR,
                         reinterpret_cast<uint8_t *>(&config),
                         sizeof(Config)) == 0;
}

/**
 * 保存配置到 Flash
 *
 * 流程：
 * 1. 擦除整个扇区（Flash 只能把 1 写成 0，要写新数据必须先擦除）
 * 2. 写回 A1（如果有效）
 * 3. 写回 A2（如果有效）
 *
 * NOTE: 每次写入都会擦除整个扇区，所以 A1 和 A2 必须一起写回
 */
bool VersionStore::Save(const Config &config)
{
    /* 先擦除整个扇区（128KB） */
    if (BspFlash_EraseSector(CONFIG_SECTOR) < 0) {
        return false;
    }

    /* 写回 A1（如果有效） */
    if (IsSlotValid(config.a1)) {
        if (BspFlash_Write(CONFIG_ADDR + A1_OFFSET,
                           reinterpret_cast<const uint8_t *>(&config.a1),
                           sizeof(Slot)) < 0) {
            return false;
        }
    }

    /* 写回 A2（如果有效） */
    if (IsSlotValid(config.a2)) {
        if (BspFlash_Write(CONFIG_ADDR + A2_OFFSET,
                           reinterpret_cast<const uint8_t *>(&config.a2),
                           sizeof(Slot)) < 0) {
            return false;
        }
    }

    return true;
}

/**
 * 更新 A1 槽位（当前运行版本）
 *
 * A1 在 APP 启动时写入，表示"当前正在运行的版本"
 * Bootloader 会比较 A1 和 A2，决定是否跳转 APP
 *
 * 优化：如果 A1 没变，跳过写入（减少 Flash 擦写寿命消耗）
 */
bool VersionStore::WriteA1(uint64_t version, uint8_t flag)
{
    Config config;
    Read(config);

    /* 版本未变则跳过写入，保护 Flash 寿命（约 10000 次擦写） */
    if (IsSlotValid(config.a1) &&
        config.a1.version == version &&
        config.a1.flag == flag) {
        return true;
    }

    MakeSlot(config.a1, version, flag);
    return Save(config);
}

/**
 * 更新 A2 槽位（目标升级版本）
 *
 * A2 在收到上位机版本帧后写入，表示"要升级到的目标版本"
 * APP 写入 A2 后复位，Bootloader 检测到 A2 有效且版本更新，进入升级模式
 *
 * 优化：如果 A2 没变，跳过写入
 */
bool VersionStore::WriteA2(uint64_t version, uint8_t flag)
{
    Config config;
    Read(config);

    if (IsSlotValid(config.a2) &&
        config.a2.version == version &&
        config.a2.flag == flag) {
        return true;
    }

    MakeSlot(config.a2, version, flag);
    return Save(config);
}

/**
 * 判断是否需要进入升级模式
 *
 * 条件：
 * 1. A1 和 A2 都有效
 * 2. A2.flag == NEED_DOWNLOAD（表示需要下载）
 * 3. A2.version > A1.version（目标版本比当前版本新）
 */
bool VersionStore::ShouldEnterUpgrade()
{
    Config config;
    Read(config);

    if (!IsSlotValid(config.a1) || !IsSlotValid(config.a2)) {
        return false;
    }

    if (config.a2.flag != VERSION_FRAME_FLAG_NEED_DOWNLOAD) {
        return false;
    }

    return config.a1.version < config.a2.version;
}
