/**
 * 版本帧协议解析实现
 * 
 * 本文件是纯协议解析，不依赖任何硬件，可在任何平台编译。
 */

#include "version_frame.h"
#include <stddef.h>

/** 帧头 "XYVH" 的小端表示（X=0x58, Y=0x59, V=0x56, H=0x48） */
#define FRAME_HEAD  0x48565958U

/** 帧尾 "XYVT" 的小端表示（X=0x58, Y=0x59, V=0x56, T=0x54） */
#define FRAME_TAIL  0x54565958U

/** 版本帧总长度（4 + 12 + 1 + 4 = 21 字节） */
#define FRAME_SIZE  21U

/** ASCII 版本号长度（yyyyMMddHHmm = 12 位） */
#define ASCII_VERSION_LEN  12U

/**
 * 从字节流读取 32 位小端整数
 * 
 * 小端格式：低字节在前，高字节在后
 * 例如：data = {0x58, 0x59, 0x56, 0x48} → 返回 0x48565958
 * 
 * @param data  字节流起始地址
 * @return 32 位整数
 */
static uint32_t ReadLe32(const uint8_t *data)
{
    return ((uint32_t)data[0])       |   // 第 0 字节（最低位）
           ((uint32_t)data[1] << 8)  |   // 第 1 字节
           ((uint32_t)data[2] << 16) |   // 第 2 字节
           ((uint32_t)data[3] << 24);    // 第 3 字节（最高位）
}

/**
 * 解析 ASCII 版本号
 * 
 * 将 12 位 ASCII 数字字符串转换为 uint64 整数
 * 例如："202606082257" → 202606082257
 * 
 * @param data     12 字节 ASCII 数字
 * @param version  输出解析结果
 * @return 0 成功, -1 格式错误（包含非数字字符）
 */
static int ParseAsciiVersion(const uint8_t *data, uint64_t *version)
{
    uint64_t value = 0;

    for (uint32_t i = 0; i < ASCII_VERSION_LEN; i++) {
        // 检查是否是数字字符 '0'-'9'
        if (data[i] < '0' || data[i] > '9') {
            return -1;
        }
        // 累加：value = value * 10 + 当前数字
        value = value * 10U + (uint64_t)(data[i] - '0');
    }

    *version = value;
    return 0;
}

/**
 * 解析版本帧
 * 
 * 帧格式：XYVH(4) + yyyyMMddHHmm(12) + flag(1) + XYVT(4) = 21 字节
 * 偏移：   0-3        4-15                16      17-20
 * 
 * @param data  原始字节流
 * @param len   数据长度
 * @param out   解析结果输出
 * @return 0 成功, -1 失败
 */
int VersionFrame_Parse(const uint8_t *data, size_t len, VersionFrame *out)
{
    // 参数检查：指针非空，长度至少 21 字节
    if (data == NULL || out == NULL || len < FRAME_SIZE) {
        return -1;
    }

    // 检查帧头：偏移 0-3 字节必须是 "XYVH"
    uint32_t head = ReadLe32(&data[0]);
    if (head != FRAME_HEAD) {
        return -1;
    }

    // 检查帧尾：偏移 17-20 字节必须是 "XYVT"
    uint32_t tail = ReadLe32(&data[17]);
    if (tail != FRAME_TAIL) {
        return -1;
    }

    // 解析版本号：偏移 4-15 是 12 位 ASCII 数字
    if (ParseAsciiVersion(&data[4], &out->version) < 0) {
        return -1;
    }

    // 读取 flag：偏移 16 是 1 字节标志位
    out->flag = data[16];
    return 0;
}

