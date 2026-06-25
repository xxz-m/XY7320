/**
 * @file    version_store.h
 * @brief   版本配置存储
 *          管理 Flash 中的 A1（当前版本）/ A2（目标版本）槽位
 */

#ifndef XY7320_VERSION_STORE_H
#define XY7320_VERSION_STORE_H

#include <cstdint>

/** 升级标志位定义（供 APP 与 BootLoader 共用） */
#define VERSION_FRAME_FLAG_NEED_DOWNLOAD  0x00U
#define VERSION_FRAME_FLAG_DOWNLOADED     0x01U

/**
 * 版本配置存储
 *
 * Flash 布局（Sector 11，地址 0x080E0000）：
 *   偏移 0x00: A1 槽位（32 字节）— 当前运行版本
 *   偏移 0x40: A2 槽位（32 字节）— 目标升级版本
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class VersionStore
{
public:
    /** 版本槽位结构（与 Flash 中布局一致，32 字节） */
    struct Slot {
        uint32_t magic;       ///< 魔数，标识槽位有效
        uint64_t version;     ///< 版本号
        uint8_t  flag;        ///< 标志位
        uint8_t  reserved[7]; ///< 保留
        uint32_t tail;        ///< 尾标记
    };

    /** 版本配置（A1 + A2） */
    struct Config {
        Slot a1;
        Slot a2;
    };

    /** 获取单例 */
    static VersionStore& Instance();

    /**
     * @brief  读取完整配置（A1 + A2）
     * @param  config  输出参数，读取到的配置
     * @return true 读取成功, false Flash 数据损坏
     */
    bool Read(Config &config);

    /**
     * @brief  更新 A1 槽位（当前运行版本）
     * @param  version  版本号
     * @param  flag     标志位
     * @return true 写入成功, false 写入失败
     */
    bool WriteA1(uint64_t version, uint8_t flag);

    /**
     * @brief  更新 A2 槽位（目标升级版本）
     * @param  version  版本号
     * @param  flag     标志位
     * @return true 写入成功, false 写入失败
     */
    bool WriteA2(uint64_t version, uint8_t flag);

    /**
     * @brief  判断是否需要进入升级模式
     * @return true 需要升级（A2 有效且版本与 A1 不同）, false 不需要
     */
    bool ShouldEnterUpgrade();

    /**
     * @brief  判断槽位数据是否有效
     * @param  slot  待检查的槽位
     * @return true 有效（magic 和 tail 匹配）, false 无效
     */
    bool IsSlotValid(const Slot &slot);

private:
    VersionStore() = default;

    /** 构造一个有效槽位 */
    void MakeSlot(Slot &slot, uint64_t version, uint8_t flag);

    /**
     * 保存配置到 Flash
     * NOTE: 每次调用会擦除整个 Sector 11（128KB），然后写回 A1+A2
     */
    bool Save(const Config &config);
};

#endif /* XY7320_VERSION_STORE_H */
