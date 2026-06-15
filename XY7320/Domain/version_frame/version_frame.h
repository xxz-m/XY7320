//
// Created by XYKJ on 2026/6/15.
//

#ifndef XY7320_VERSION_FRAME_H
#define XY7320_VERSION_FRAME_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * 版本帧协议定义
 * 
 * 协议格式（21 字节）：
 * ┌─────────┬──────────────┬──────┬─────────┐
 * │ XYVH(4) │ yyyyMMddHHmm │ flag │ XYVT(4) │
 * │  帧头   │  (12字节)    │ (1)  │  帧尾   │
 * └─────────┴──────────────┴──────┴─────────┘
 * 
 * 示例：XYVH20260608225700XYVT
 *       帧头  版本号(12位)  flag 帧尾
 * 
 * 版本号格式：yyyyMMddHHmm（12位 ASCII 数字）
 *   如 202606082257 表示 2026-06-08 22:57
 * 
 * flag 含义：
 *   0x00 = NEED_DOWNLOAD（需要下载升级）
 *   0x01 = DOWNLOADED（已下载完成）
 */

/** 版本帧标志位定义 */
#define VERSION_FRAME_FLAG_NEED_DOWNLOAD  0x00U  // 需要下载升级
#define VERSION_FRAME_FLAG_DOWNLOADED     0x01U  // 已下载完成

/** 版本帧 ACK 应答码 */
#define VERSION_FRAME_ACK  "XYA1"  // APP 收到版本帧后返回此应答，然后复位进入 Bootloader

/** 版本帧解析结果 */
typedef struct {
    uint64_t version;  // 版本号（如 202606082257）
    uint8_t  flag;     // 标志位（0x00 需升级，0x01 已下载）
} VersionFrame;

/**
 * 解析版本帧
 * 
 * 将 21 字节的原始数据解析为 VersionFrame 结构体
 * 
 * @param data  原始字节流（至少 21 字节）
 * @param len   数据长度
 * @param out   解析结果输出
 * @return 0 成功, -1 失败（帧头/帧尾/格式错误）
 */
int VersionFrame_Parse(const uint8_t *data, size_t len, VersionFrame *out);

#ifdef __cplusplus
}
#endif

#endif //XY7320_VERSION_FRAME_H
