/**
 * @file    protocol_xy.h
 * @brief   新域科技串口通信协议 —— 帧格式定义与编解码接口
 *
 * 帧格式（大端）：
 * ┌──────┬──────┬────────┬──────────┬──────────┬──────────┬──────────┬──────┬─────┬────────┬──────┬──────┐
 * │ HEAD │ HEAD │info_len│origin_port│origin_addr│goal_port │goal_addr │model │ cmd │  data  │ CRC  │ END  │
 * │0x10  │0x02  │ (2B)   │  (1B)    │ (变长)    │  (1B)    │ (变长)   │ (1B) │(1B) │ (变长) │ (2B) │0x1003│
 * └──────┴──────┴────────┴──────────┴──────────┴──────────┴──────────┴──────┴─────┴────────┴──────┴──────┘
 *
 * 转义规则：帧体内 0x10 → 0x10 0x10（收发两端自动处理）
 * CRC 算法：CRC-16/XMODEM（多项式 X^16+X^12+X^5+1，查表法）
 *
 * 本文件是纯协议定义，不依赖任何硬件，可在任何平台编译。
 */

#ifndef XY7320_PROTOCOL_XY_H
#define XY7320_PROTOCOL_XY_H

#include <stdint.h>
#include "log_service.h"

/// 协议层日志输出，重定向到 LogService
#define PROTOCOL_Printf(...)  LogService::Instance().Printf(__VA_ARGS__)

namespace Protocol {

/* ====== 帧格式常量 ====== */

#define MAX_PROTOCOL_LEN 128       ///< 单帧数据段最大长度（字节）
#define Address_len 6              ///< 通信地址最大长度（字节）

#define HEAD1 0x10                 ///< 帧头第一字节
#define HEAD2 0x02                 ///< 帧头第二字节（帧头 = 0x10 0x02）
#define END1  0x10                 ///< 帧尾第一字节
#define END2  0x03                 ///< 帧尾第二字节（帧尾 = 0x10 0x03）

/* ====== 协议解析状态码 ====== */

/// DecodeBuffer / DecodeRibbon 返回的解析结果
enum PacketState {
    unNoError = 0,     ///< 解析成功
    unHeadErr = 1,     ///< 未找到帧头
    unENDErr = 2,      ///< 未找到帧尾
    unCRCErr = 3,      ///< CRC 校验失败
    unPortErr = 4,     ///< 端口号不匹配
    unAddressErr = 5,  ///< 地址错误
    unDataNone = 6,    ///< 无数据
    unInfo_lenErr = 7, ///< info_len 字段异常
    unknown = -1,      ///< 未知错误
};

/* ====== 协议包结构体 ====== */

/// 一帧协议数据包的完整描述
typedef struct tagProtocolPacket {
    uint8_t Head1;                     ///< 帧头第一字节 (0x10)
    uint8_t Head2;                     ///< 帧头第二字节 (0x02)
    uint16_t info_len;                 ///< 信息段长度（从 origin_port 到 CRC 之前的字节数）

    uint8_t origin_port;               ///< 源端口号（见 USERPORT 枚举）
    uint8_t origin_Address_len;        ///< 源地址长度
    uint8_t origin_IP[Address_len];    ///< 源通信地址

    uint8_t goal_port;                 ///< 目的端口号
    uint8_t goal_Address_len;          ///< 目的地址长度
    uint8_t goal_IP[Address_len];      ///< 目的通信地址

    uint8_t model;                     ///< 业务类型（0x01=读, 0x02=写）
    uint8_t cmd;                       ///< 命令码
    uint8_t data[MAX_PROTOCOL_LEN];    ///< 数据段（最大 128 字节）
    uint16_t crcValue;                 ///< CRC16 校验值
    uint8_t End1;                      ///< 帧尾第一字节 (0x10)
    uint8_t End2;                      ///< 帧尾第二字节 (0x03)
    uint8_t data_len;                  ///< 数据段实际长度
    PacketState state;                 ///< 解析状态
} ProtocolPacket;

/* ====== 读命令码（上位机读取设备信息） ====== */

enum PacketReadCMD {
    DeviceButton  = 0x30,  ///< 按键事件上报
    DeviceNumber  = 0x31,  ///< 读设备序列号
    DeviceVersion = 0x32,  ///< 读固件版本号
    DeviceBat     = 0x33,  ///< 读电池电压
    DeviceTemp    = 0x34,  ///< 读温度
    DeviceMohm    = 0x35,  ///< 读兆欧值
};

/* ====== 写命令码（上位机向设备写入控制命令） ====== */

enum PacketWriteCMD {
    RelayCH = 0x01,        ///< 继电器通道控制，通道号在 data 中标识
};

/// 继电器通道编码（各产品型号的继电器控制码）
typedef struct tagRelayCode{
    uint16_t BT200HVNA = 0x0300;
    uint16_t BT200HTX = 0x0000;
    uint16_t BT200HRX = 0x1000;

    uint16_t BT300HVNA = 0x0300;
    uint16_t BT300HTX = 0x0000;
    uint16_t BT300HRX = 0x1000;

    uint16_t BT300SVNATX = 0x0350;
    uint16_t BT300SVNARX = 0x0352;
    uint16_t BT300STX = 0x0040;
    uint16_t BT300SRX = 0x1040;

    uint16_t BT300TVNATX = 0x07FC;
    uint16_t BT300TVNARX = 0x07FE;
    uint16_t BT300TVNAE = 0x07FD;
    uint16_t BT300TVNAF = 0x07FF;
    uint16_t BT300TTX = 0x04E8;
    uint16_t BT300TRX = 0x14E8;
} RelayCode;

/* ====== 业务类型 ====== */

enum PacketTYPE {
    unRead  = 0x01,    ///< 读操作：上位机请求读取设备数据
    unWrite = 0x02     ///< 写操作：上位机向设备发送控制命令
};

/* ====== 设备端口号 ====== */
/// 公司各产品端口号定义，新增产品在后面递增
enum USERPORT {
    XY_all        = 0x00,  ///< 广播地址
    XY_PC         = 0x01,  ///< 上位机
    XY_403D       = 0x02,
    XY_890H       = 0x03,
    XY_1600A      = 0x04,
    XY_7666A      = 0x05,
    XY_HL8000A    = 0x06,
    XY_ZC9000A    = 0x07,
    XY_CQ1000A    = 0x08,
    XY_9200       = 0x09,
    XY_BT6000SW   = 0x20,
    XY_7000XMAIN  = 0x21,
    XY_7320       = 0x22,
};

/* ====== 编解码接口 ====== */

/**
 * CRC-16/XMODEM 校验（查表法）
 *
 * @param ptr  待校验数据起始地址
 * @param len  数据长度（字节）
 * @return CRC16 校验值
 */
uint16_t Get_Crc16(uint8_t *ptr, int len);

/**
 * 协议帧解析（解析已去转义的完整帧）
 *
 * 将一帧原始字节解析为 ProtocolPacket 结构体，
 * 包含帧头/帧尾校验、CRC 校验、字段提取。
 * 调用前必须确保数据已去除转义字符。
 *
 * @param src   去转义后的帧数据
 * @param len   帧数据长度
 * @param info  解析结果输出
 */
void DecodeRibbon(uint8_t *src, uint16_t len, ProtocolPacket *info);

/**
 * 从原始字节流中查找并解码一帧协议数据
 *
 * 在接收缓冲区中定位帧头(0x10 0x02)和帧尾(0x10 0x03)，
 * 去除帧内转义字符(0x10 0x10 → 0x10)，然后调用 DecodeRibbon 解析。
 *
 * @param src   接收缓冲区原始数据
 * @param len   缓冲区数据长度
 * @param info  解析结果输出
 * @return 本次消费掉的字节数（调用方据此移动缓冲区）
 */
uint16_t DecodeBuffer(uint8_t *src, uint16_t len, ProtocolPacket *info);

/**
 * 编码协议包
 *
 * 将 ProtocolPacket 结构体打包为可发送的字节流，
 * 自动添加帧头/帧尾、计算 CRC、处理转义字符。
 *
 * @param Packet  待编码的协议包
 * @param dest    输出缓冲区（调用方确保容量足够，建议 >= 300 字节）
 * @return 编码后的帧长度（字节），可直接发送
 */
uint16_t EncodePacket(const ProtocolPacket *Packet, uint8_t *dest);

/**
 * 初始化协议包默认参数
 *
 * 设置帧头/帧尾、源端口（本机）、目的端口（上位机）等默认值，
 * 发送前调用此函数初始化，再按需修改 cmd 和 data 字段。
 *
 * @param info  待初始化的协议包
 */
void initProtocol(ProtocolPacket *info);

} // namespace Protocol

#endif // XY7320_PROTOCOL_XY_H
