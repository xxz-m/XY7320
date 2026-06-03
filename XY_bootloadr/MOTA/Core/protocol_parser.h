/**
 * \file            protocol_parser.h
 * \brief           protocol parser
 */

/*
 * Copyright (c) 2022 Dino Haw
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of mOTA - The Over-The-Air technology component for MCU.
 *
 * Author:          Dino Haw <347341799@qq.com>
 * Version:         v1.0.0
 */

#ifndef __PROTOCOL_PARSER_H__
#define __PROTOCOL_PARSER_H__

#include "bsp_common.h"
#include "firmware_manage.h"

/* YModem protocol */
#define YMODEM_SOH                  0x01
#define YMODEM_STX                  0x02
#define YMODEM_EOT                  0x04
#define YMODEM_ACK                  0x06
#define YMODEM_NAK                  0x15
#define YMODEM_CAN                  0x18

#define YMODEM_FRAME_FIXED_LEN      (5)
#define YMODEM_SOH_DATA_LEN         (128)
#define YMODEM_STX_DATA_LEN         (1024)
#define YMODEM_SOH_FRAME_LEN        (YMODEM_FRAME_FIXED_LEN + YMODEM_SOH_DATA_LEN)
#define YMODEM_STX_FRAME_LEN        (YMODEM_FRAME_FIXED_LEN + YMODEM_STX_DATA_LEN)
#define YMODEM_C                    'C'

#define PP_FIRMWARE_PKG_SIZE        YMODEM_STX_DATA_LEN
#define PP_MSG_BUFF_SIZE            YMODEM_STX_FRAME_LEN

typedef enum
{
    YMODEM_FLOW_NONE = 0x00,
    YMODEM_FLOW_START,
    YMODEM_FLOW_FIRST_EOT,
    YMODEM_FLOW_SECOND_EOT,
    YMODEM_FLOW_ASK,
    YMODEM_FLOW_SUCCESS,
    YMODEM_FLOW_FAILED,
    YMODEM_FLOW_CANCEL,

} YMODEM_EXE_FLOW;

#pragma pack(1)
union HOST_MESSAGE
{
    uint8_t raw_data[ PP_MSG_BUFF_SIZE ];

    struct
    {
        uint8_t header;
        uint8_t pkt_num;
        uint8_t not_pkt_num;
        uint8_t data[];
    } pkg;
};
#pragma pack()

/* command execute error code */
typedef enum
{
    PP_ERR_OK                   = 0x00,
    PP_ERR_UNKOWN_ERR           = 0x01,
    PP_ERR_DUPLICATE_FRAME      = 0x02,
    PP_ERR_OMISSION_FRAME       = 0x03,
    PP_ERR_PKT_NUM_ERR          = 0x04,
    PP_ERR_FRAME_LENGTH_ERR     = 0x05,
    PP_ERR_FRAME_VERIFY_ERR     = 0x06,
    PP_ERR_HEADER_ERR           = 0x07,
    PP_ERR_EXE_FLOW_ERR         = 0x08,
    PP_ERR_NO_FACTORY_FIRMWARE,
    PP_ERR_NO_THIS_PART,
    PP_ERR_READ_IS_EMPTY_ERR,
    PP_ERR_FIRMWARE_OVERSIZE,
    PP_ERR_FIRMWARE_HEAD_VERIFY_ERR,
    PP_ERR_VERIFY_READ_ERR,
    PP_ERR_RAW_BODY_VERIFY_ERR,
    PP_ERR_PKG_BODY_VERIFY_ERR,
    PP_ERR_ERASE_PART_ERR,
    PP_ERR_WRITE_FIRST_ADDR_ERR,
    PP_ERR_JUMP_TO_APP_ERR,
    PP_ERR_READ_FIRMWARE_HEAD_ERR,
    PP_ERR_UPDATE_READ_ERR,
    PP_ERR_UPDATE_VER_READ_ERR,
    PP_ERR_UPDATE_VER_ERASE_ERR,
    PP_ERR_UPDATE_VER_WRITE_ERR,
    PP_ERR_WRITE_PART_ERR,
    PP_ERR_FAULT_FIRMWARE,
    PP_ERR_CAN_NOT_PLACE_IN_APP,
    PP_ERR_DOES_NOT_EXIST_DOWNLOAD,
    PP_ERR_DOES_NOT_EXIST_FACTORY,
    PP_ERR_NO_DECRYPT_COMPONENT,
    PP_ERR_DECRYPT_ERR,

} PP_CMD_ERR_CODE;

typedef enum
{
    PP_CMD_NONE                 = 0x00,
    PP_CMD_SOH                  = YMODEM_SOH,
    PP_CMD_STX                  = YMODEM_STX,
    PP_CMD_EOT                  = YMODEM_EOT,
    PP_CMD_CAN                  = YMODEM_CAN,

} PP_CMD;

typedef enum
{
    PP_RESULT_OK                = 0x00,
    PP_RESULT_PROCESS           = 0x01,
    PP_RESULT_FAILED            = 0x02,
    PP_RESULT_CANCEL            = 0x03,

} PP_CMD_EXE_RESULT;

typedef enum
{
    PP_CONFIG_NONE              = 0x00,
    PP_CONFIG_RESET             = 0x01,
    PP_CONFIG_ENABLE_RECV_CMD   = 0x02,

} PP_CONFIG_PARA;

struct PP_DEV_TX_PKG
{
    uint8_t response;
};

typedef void (*PP_Send_t)(uint8_t *data, uint16_t len, uint32_t timeout);
typedef void (*PP_HeartbeatCallback_t)(uint8_t *data, uint16_t *data_len);
typedef void (*PP_PrepareCallback_t)(PP_CMD cmd, uint8_t *data, uint16_t data_len);
typedef void (*PP_ReplyCallback_t)(PP_CMD cmd, PP_CMD_EXE_RESULT *cmd_exe_result, uint8_t *data, uint16_t *data_len);

void            PP_Init     (PP_Send_t               Send,
                             PP_HeartbeatCallback_t  HeartbeatCallback,
                             PP_PrepareCallback_t    PrepareCallback,
                             PP_ReplyCallback_t      Set_ResponseInfo);
PP_CMD_ERR_CODE PP_Handler  (uint8_t *data, uint16_t len);
void            PP_Config   (PP_CONFIG_PARA  para, void *value);

#endif
