//
// Created by XYKJ on 2026/6/4.
//

#ifndef XY_BOOTLOADR_SIMPLE_UPDATE_H
#define XY_BOOTLOADR_SIMPLE_UPDATE_H
#include "main.h"

/**
 * @brief 初始化 APP 分区和升级状态机。
 *
 * 初始化完成后处于等待 12 字节升级头的状态；该函数应在
 * Bootloader 主循环进入数据处理前调用一次。
 */
void Simple_Update_Init(void);

/**
 * @brief 在主循环上下文处理收到的升级数据。
 *
 * 数据顺序为 12 字节头、固件数据包；合法头会擦除 APP 分区并发送
 * XYB2，固件校验通过后发送 XYB3 并跳转 APP。不得在中断服务函数中调用。
 *
 * @param data 收到的数据缓冲区。
 * @param len  数据长度，单位为字节。
 */
void Simple_Update_Process(uint8_t *data, uint16_t len);
#endif //XY_BOOTLOADR_SIMPLE_UPDATE_H
