/**
 * @file    protocol_xy.cpp
 * @brief   新域科技串口通信协议 —— 编解码实现
 *
 * 本文件是纯协议实现，不依赖任何硬件，可在任何平台编译。
 * 包含：CRC-16 查表、帧查找与去转义、帧字段解析、帧打包与转义。
 */

#include "protocol_xy.h"
#include <string.h>

/// CRC-16/XMODEM 查找表（多项式 0x1021，初始值 0x0000）
/// 查表法避免逐位计算，每个字节只需一次查表 + 异或
static const unsigned int crc_table[256] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};
/**
 * CRC-16/XMODEM 校验（查表法）
 *
 * 算法原理：每读入一个字节，取 CRC 高 8 位与该字节异或作为查表索引，
 * 将查表结果与 CRC 左移 8 位后的值异或，得到新的 CRC。
 *
 * @param ptr  待校验数据起始地址
 * @param len  数据长度（字节）
 * @return CRC16 校验值
 */
uint16_t Protocol::Get_Crc16(uint8_t *ptr, int len)
{
    uint16_t crc = 0x0000;  // CRC-16/XMODEM 初始值为 0
    while (len--) {
        // 高 8 位 XOR 当前字节 → 查表 → 与左移后的低 8 位异或
        crc = (crc << 8) ^ crc_table[(crc >> 8 ^ *ptr++) & 0xff];
    }
    return crc;
}
/**
 * 从原始字节流中查找并解码一帧协议数据
 *
 * 处理流程：
 * 1. 在缓冲区中扫描帧头 (0x10 0x02) 和帧尾 (0x10 0x03)
 * 2. 帧尾判断需排除转义序列：若 0x10 前面还是 0x10，说明是转义而非帧尾
 * 3. 找到完整帧后，去除帧内的转义字符 (0x10 0x10 → 0x10)
 * 4. 调用 DecodeRibbon 解析去转义后的帧内容
 *
 * @param src   接收缓冲区原始数据
 * @param len   缓冲区数据长度
 * @param info  解析结果输出
 * @return 本次消费掉的字节数（调用方据此移动缓冲区）
 */
uint16_t Protocol::DecodeBuffer(uint8_t *src, uint16_t len, ProtocolPacket *info)
{
    if (info == nullptr || src == nullptr || len == 0U) {
        if (info != nullptr) {
            info->state = unDataNone;
        }
        return 0U;
    }

    uint16_t start = len;
    for (uint16_t index = 1U; index < len; ++index) {
        if (src[index - 1U] == HEAD1 && src[index] == HEAD2) {
            start = static_cast<uint16_t>(index - 1U);
            break;
        }
    }
    if (start == len) {
        info->state = unHeadErr;
        return len;
    }

    uint16_t end = len;
    for (uint16_t index = static_cast<uint16_t>(start + 2U); index < len; ++index) {
        if (src[index - 1U] == END1 && src[index] == END2) {
            uint16_t repeated = 0U;
            for (uint16_t cursor = index - 1U; cursor > start && src[cursor - 1U] == END1; --cursor) {
                ++repeated;
            }
            if ((repeated & 1U) == 0U) {
                end = index;
                break;
            }
        }
    }
    if (end == len) {
        info->state = unENDErr;
        return start;
    }

    uint8_t decoded[256]{};
    uint16_t decodedLen = 0U;
    for (uint16_t index = start; index <= end; ++index) {
        if (decodedLen >= sizeof(decoded)) {
            info->state = unInfo_lenErr;
            return static_cast<uint16_t>(end + 1U);
        }
        if (index > start + 1U && index < end - 1U &&
            src[index] == HEAD1 && index + 1U < end && src[index + 1U] == HEAD1) {
            decoded[decodedLen++] = src[index++];
        } else {
            decoded[decodedLen++] = src[index];
        }
    }

    DecodeRibbon(decoded, decodedLen, info);
    return static_cast<uint16_t>(end + 1U);
}

/**
 * 协议帧字段解析
 *
 * 输入必须是已去转义的完整帧，按以下偏移提取字段：
 *   [0-1]  帧头 0x10 0x02
 *   [2-3]  info_len（信息段长度，大端）
 *   [4]    源端口
 *   [5]    源地址长度
 *   [6..]  源地址（变长）
 *   [...]  目的端口、目的地址长度、目的地址（变长）
 *   [...]  业务类型(model)、命令码(cmd)、数据段
 *   [末尾] CRC16（2字节，大端）+ 帧尾 0x10 0x03
 *
 * @param src   去转义后的帧数据
 * @param len   帧数据长度
 * @param info  解析结果输出
 */
void Protocol::DecodeRibbon(uint8_t *src, uint16_t len, ProtocolPacket *info)
{
    if (len > MAX_PROTOCOL_LEN) {
        info->state = unInfo_lenErr;
        return;
    }
    int i;
    int data_len = 0, info_len;
    uint16_t crcValue;

    // 填写帧头帧尾
    info->Head1 = HEAD1;
    info->Head2 = HEAD2;
    info->End1 = END1;
    info->End2 = END2;

    // 提取 info_len（偏移 2-3，大端序）
    info->info_len = src[2] << 8 | src[3];
    info_len = src[2] << 8 | src[3];
    /*
     * info_len 覆盖自身的 2 字节、地址字段、model/cmd 和 data；CRC
     * 紧随 info 段之后，因此先确认 info 段没有越过当前去转义帧。
     */
    if (info_len > (len - 4)) {
        info->state = unInfo_lenErr;
        return;
    }

    // CRC 校验遵循源协议：从 info_len 开始校验 info_len 个字节
    info->crcValue = Get_Crc16(src + 2, info_len);
    crcValue = static_cast<uint16_t>(src[info_len + 2] << 8) |
               static_cast<uint16_t>(src[info_len + 3]);
    PROTOCOL_Printf("Protocol,CRC,calc=%04X,recv=%04X,len=%u,info=%u,raw=", info->crcValue,
                    crcValue, len, info_len);
    for (uint16_t index = 0U; index < len; ++index) {
        PROTOCOL_Printf("%02X", src[index]);
    }
    PROTOCOL_Printf("\\r\\n");
    if (info->crcValue == crcValue) {
        int offset = 4;  // 当前读取偏移（跳过帧头2 + info_len 2）

        // 源端口
        info->origin_port = src[offset++];
        // 源地址长度
        info->origin_Address_len = src[offset++];
        for (i = 0; i < info->origin_Address_len; i++) {
            info->origin_IP[i] = src[offset++];
        }

        // 目的端口
        info->goal_port = src[offset++];
        // 目的地址长度
        info->goal_Address_len = src[offset++];
        for (i = 0; i < info->goal_Address_len; i++) {
            info->goal_IP[i] = src[offset++];
        }

        // 业务类型（读/写）
        info->model = src[offset++];
        // 命令码
        info->cmd = src[offset++];

        // 数据段遵循源协议：固定开销为 8 字节
        data_len = info_len - info->origin_Address_len - info->goal_Address_len - 8;
        memset(info->data, 0, MAX_PROTOCOL_LEN);
        for (i = 0; i < data_len; i++) {
            info->data[i] = src[offset++];
        }
        info->data_len = data_len;
        info->state = unNoError;
    } else {
        info->state = unCRCErr;
    }
}
/**
 * 编码协议包
 *
 * 处理流程：
 * 1. 在内部缓冲区组装帧内容：帧头 + info_len + 端口地址 + model + cmd + data + CRC + 帧尾
 * 2. 将帧体（帧头帧尾之间的内容）拷贝到 dest，遇到 0x10 字节时插入转义 (0x10 → 0x10 0x10)
 * 3. 帧头和帧尾不做转义处理
 *
 * @param Packet  待编码的协议包
 * @param dest    输出缓冲区（调用方确保容量 >= 300 字节）
 * @return 编码后的帧长度（字节），可直接通过串口发送
 */
uint16_t Protocol::EncodePacket(const ProtocolPacket *Packet, uint8_t *dest)
{
#ifdef PROTOCOL_LOG
    PROTOCOL_Printf("info.Head1:%02x\n", Packet->Head1);
    PROTOCOL_Printf("info.Head2:%02x\n", Packet->Head2);
    PROTOCOL_Printf("info.origin_port:%02x\n", Packet->origin_port);
    PROTOCOL_Printf("info.goal_port:%02x\n", Packet->goal_port);
    PROTOCOL_Printf("info.model:%02x\n", Packet->model);
    PROTOCOL_Printf("info.cmd:%02x\n", Packet->cmd);
    PROTOCOL_Printf("info.origin_Address_len:%d\n", Packet->origin_Address_len);
    PROTOCOL_Printf("info.goal_Address_len:%d\n", Packet->goal_Address_len);
    PROTOCOL_Printf("info.data_len:%02x\n", Packet->data_len);
#endif

    /* 第一步：在 buff 中组装未转义的完整帧 */
    uint8_t buff[300];  // 内部组装缓冲（300 字节足够覆盖最大帧）
    int len = 0;

    // 帧头
    buff[len++] = Packet->Head1;
    buff[len++] = Packet->Head2;

    // info_len 遵循源协议，固定开销为 8 字节
    uint16_t info_len = 8 + Packet->origin_Address_len + Packet->goal_Address_len + Packet->data_len;
    buff[len++] = (info_len >> 8) & 0xff;  // 大端序高字节
    buff[len++] = info_len & 0xff;          // 大端序低字节

    // 源端口 + 源地址
    buff[len++] = Packet->origin_port;
    buff[len++] = Packet->origin_Address_len;
    for (int i = 0; i < Packet->origin_Address_len; i++) {
        buff[len++] = Packet->origin_IP[i];
    }

    // 目的端口 + 目的地址
    buff[len++] = Packet->goal_port;
    buff[len++] = Packet->goal_Address_len;
    for (int i = 0; i < Packet->goal_Address_len; i++) {
        buff[len++] = Packet->goal_IP[i];
    }

    // 业务类型 + 命令码 + 数据段
    buff[len++] = Packet->model;
    buff[len++] = Packet->cmd;
    for (int i = 0; i < Packet->data_len; i++) {
        buff[len++] = Packet->data[i];
    }

    // CRC16 校验遵循源协议：从 info_len 开始校验 info_len 个字节
    uint16_t crcValue = Get_Crc16(buff + 2, info_len);
    buff[len++] = (crcValue >> 8) & 0xff;
    buff[len++] = crcValue & 0xff;

    // 帧尾
    buff[len++] = Packet->End1;
    buff[len++] = Packet->End2;

    /* 第二步：拷贝到 dest，帧体内 0x10 字节插入转义
 *
 * 帧头/帧尾是定界符，转义后无法定位边界，因此**不转义**。
 * 仅帧体（偏移 2 到 len-3）中的 0x10 重复一次表示字面值。 */
    uint16_t DLE_len = 2;
    dest[0] = buff[0];  // 帧头原样拷贝，不转义
    dest[1] = buff[1];

    // 帧体（从偏移 2 到 len-3）逐字节拷贝，遇 0x10 则重复一次
    for (int i = 2; i < len - 2; i++) {
        dest[DLE_len] = buff[i];
        DLE_len++;
        if (buff[i] == END1) {
            dest[DLE_len] = buff[i];  // 转义：0x10 → 0x10 0x10
            DLE_len++;
        }
    }

    // 帧尾原样拷贝，不转义
    dest[DLE_len++] = buff[len - 2];
    dest[DLE_len++] = buff[len - 1];

    return DLE_len;
}
/**
 * 初始化协议包默认参数
 *
 * 设置帧头/帧尾常量，源端口默认为本机（XY_7000XMAIN），
 * 目的端口默认为上位机（XY_PC），地址长度清零。
 * 调用后按需修改 model、cmd、data 等字段即可发送。
 *
 * @param info  待初始化的协议包
 */
void Protocol::initProtocol(ProtocolPacket *info)
{
    // 帧头帧尾固定值
    info->Head1 = HEAD1;
    info->Head2 = HEAD2;
    info->End1 = END1;
    info->End2 = END2;

    // 源端口：本机设备（TODO: XY7320 应替换为自己的端口号）
    info->origin_port = XY_7320;
    info->origin_Address_len = 0;
    memset(info->origin_IP, 0, Address_len);

    // 目的端口：默认上位机
    info->goal_port = XY_PC;
    info->goal_Address_len = 0;
    memset(info->goal_IP, 0, Address_len);

    // 默认写操作，命令和数据清零
    info->model = unWrite;
    info->cmd = 0x00;
    info->data_len = 0;
    memset(info->data, 0, MAX_PROTOCOL_LEN);

    info->state = unNoError;
}