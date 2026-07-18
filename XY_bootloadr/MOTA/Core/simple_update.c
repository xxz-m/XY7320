//
// Created by XYKJ on 2026/6/4.
//

#include "simple_update.h"
#include "bsp_flash.h"
#include "app_version_config.h"
#include "bootloader_config.h"
#include "bootloader_define.h"
#include "usart.h"
#include "string.h"

/*
 * 升级头和 ACK 必须与上位机 FirmwareUploader 保持字节级一致：
 * 头为小端 magic/app_size/app_crc，XYB2 表示可发送固件，XYB3
 * 表示校验通过且 Bootloader 即将跳转到 APP。
 */
#define SIMPLE_APP_MAGIC        0x41505055U
#define SIMPLE_UPDATE_MAX_FRAME 1024U
#define SIMPLE_UPDATE_ACK_HEADER_READY "XYB2"
#define SIMPLE_UPDATE_ACK_FINISH       "XYB3"

typedef struct
{
    uint32_t magic;
    uint32_t app_size;
    uint32_t app_crc;
} simple_app_head_t;

typedef enum
{
    SIMPLE_UPGRADE_WAIT_HEAD = 0,
    SIMPLE_UPGRADE_RECV_APP,
    SIMPLE_UPGRADE_DONE,
    SIMPLE_UPGRADE_ERROR
} simple_upgrade_state_t;

static simple_upgrade_state_t upgrade_state = SIMPLE_UPGRADE_WAIT_HEAD;
static simple_app_head_t upgrade_head;

static struct BSP_FLASH app_flash_part;
static struct BSP_FLASH *app_part = NULL;

static uint32_t upgrade_received_size = 0;
static uint32_t upgrade_write_offset = 0;

static uint8_t Simple_Update_CheckAppValid(void);
static void Simple_Update_JumpToApp(void);
static void Simple_Update_SendAck(const char *ack);

/**
 * 升级状态从 WAIT_HEAD 开始，避免复位后的残留缓冲被误当作固件数据。
 * APP 分区地址和大小由 bootloader_config.h 提供，必须与链接脚本一致。
 */
void Simple_Update_Init(void)
{
    BSP_Flash_Init(&app_flash_part, APP_PART_NAME, APP_ADDRESS, APP_PART_SIZE);
    app_part = BSP_Flash_GetHandle(APP_PART_NAME);

    upgrade_state = SIMPLE_UPGRADE_WAIT_HEAD;
    upgrade_received_size = 0;
    upgrade_write_offset = 0;

    BSP_Printf("[simple update] init ok\r\n");
    BSP_Printf("[simple update] APP addr: 0x%08lX, size: 0x%08lX\r\n",
           (uint32_t)APP_ADDRESS,
           (uint32_t)APP_PART_SIZE);
}
void Simple_Update_Process(uint8_t *data, uint16_t len)
{
    uint32_t write_len;

    if (data == NULL || len == 0)
    {
        return;
    }

    if (app_part == NULL)
    {
        BSP_Printf("[simple update] app part null\r\n");
        return;
    }

    switch (upgrade_state)
    {
        case SIMPLE_UPGRADE_WAIT_HEAD:
        {
            if (len < sizeof(simple_app_head_t))
            {
                BSP_Printf("[simple update] head too short, len=%d\r\n", len);
                return;
            }

            memcpy(&upgrade_head, data, sizeof(simple_app_head_t));

            BSP_Printf("[simple update] head magic: 0x%08lX\r\n", upgrade_head.magic);
            BSP_Printf("[simple update] app size: %lu\r\n", upgrade_head.app_size);
            BSP_Printf("[simple update] app crc : 0x%08lX\r\n", upgrade_head.app_crc);

            if (upgrade_head.magic != SIMPLE_APP_MAGIC)
            {
                BSP_Printf("[simple update] magic error\r\n");
                upgrade_state = SIMPLE_UPGRADE_ERROR;
                return;
            }

            if (upgrade_head.app_size == 0 || upgrade_head.app_size > APP_PART_SIZE)
            {
                BSP_Printf("[simple update] app size error\r\n");
                upgrade_state = SIMPLE_UPGRADE_ERROR;
                return;
            }

            BSP_Printf("[simple update] erase app part...\r\n");

            if (BSP_Flash_Erase(app_part, 0, APP_PART_SIZE) < 0)
            {
                BSP_Printf("[simple update] erase app fail\r\n");
                upgrade_state = SIMPLE_UPGRADE_ERROR;
                return;
            }

            BSP_Printf("[simple update] erase app ok\r\n");

            /* 只有完整头校验和 APP 分区擦除都成功，才允许接收固件数据。 */
            upgrade_received_size = 0;
            upgrade_write_offset = 0;
            upgrade_state = SIMPLE_UPGRADE_RECV_APP;

            BSP_Printf("OK\r\n");
            BSP_Printf("[simple update] waiting app data...\r\n");
            Simple_Update_SendAck(SIMPLE_UPDATE_ACK_HEADER_READY);
            break;
        }

        case SIMPLE_UPGRADE_RECV_APP:
        {
            if (upgrade_received_size >= upgrade_head.app_size)
            {
                BSP_Printf("[simple update] already received done\r\n");
                return;
            }

            write_len = len;

            if ((upgrade_received_size + write_len) > upgrade_head.app_size)
            {
                write_len = upgrade_head.app_size - upgrade_received_size;
            }

            /*
             * 当前 flash_port_stm32f4.c 按 4 字节写。
             * 最后一包如果不是 4 字节对齐，需要补齐。
             */
            uint8_t write_buf[SIMPLE_UPDATE_MAX_FRAME + 4];
            uint32_t aligned_len = write_len;

            if (write_len > SIMPLE_UPDATE_MAX_FRAME)
            {
                BSP_Printf("[simple update] frame too large\r\n");
                upgrade_state = SIMPLE_UPGRADE_ERROR;
                return;
            }

            memset(write_buf, 0xFF, sizeof(write_buf));
            memcpy(write_buf, data, write_len);

            /* Flash 按 32 位写入；最后一包的逻辑长度和实际写入长度可能不同，
             * 补齐区保持 0xFF，避免把无效数据写入 APP。 */
            if ((aligned_len % 4U) != 0U)
            {
                aligned_len = (aligned_len + 3U) & ~3U;
            }

            if (BSP_Flash_Write(app_part, upgrade_write_offset, write_buf, aligned_len) < 0)
            {
                BSP_Printf("[simple update] write fail, offset=0x%08lX\r\n", upgrade_write_offset);
                upgrade_state = SIMPLE_UPGRADE_ERROR;
                return;
            }

            upgrade_received_size += write_len;
            upgrade_write_offset += aligned_len;

            BSP_Printf("[simple update] recv %lu / %lu\r\n",
                   upgrade_received_size,
                   upgrade_head.app_size);

            if (upgrade_received_size >= upgrade_head.app_size)
            {
                BSP_Printf("[simple update] receive done\r\n");

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

                    upgrade_state = SIMPLE_UPGRADE_DONE;

                    /* XYB3 必须先发出并留出串口发送时间，再跳转，避免上位机收不到完成 ACK。 */
                    Simple_Update_SendAck(SIMPLE_UPDATE_ACK_FINISH);
                    HAL_Delay(100);
                    Simple_Update_JumpToApp();
                }
                else
                {
                    BSP_Printf("[simple update] APP invalid\r\n");
                    upgrade_state = SIMPLE_UPGRADE_ERROR;
                }
            }

            break;
        }

        case SIMPLE_UPGRADE_DONE:
        {
            BSP_Printf("[simple update] already done\r\n");
            break;
        }

        case SIMPLE_UPGRADE_ERROR:
        default:
        {
            BSP_Printf("[simple update] error state, reset board to retry\r\n");
            break;
        }
    }
}

/* Bootloader 尚未接入 APP 的 UartTxService，只能在主循环中阻塞发送升级 ACK。 */
static void Simple_Update_SendAck(const char *ack)
{
    if (ack == NULL)
    {
        return;
    }

    (void)HAL_UART_Transmit(&huart2, (uint8_t *)ack, (uint16_t)strlen(ack), 100U);
}

static uint8_t Simple_Update_CheckAppValid(void)
{
    uint32_t app_stack = *(volatile uint32_t *)APP_ADDRESS;
    uint32_t app_reset = *(volatile uint32_t *)(APP_ADDRESS + 4U);

    BSP_Printf("[simple update] app stack: 0x%08lX\r\n", app_stack);
    BSP_Printf("[simple update] app reset: 0x%08lX\r\n", app_reset);

    if ((app_stack & 0x2FF00000U) != 0x20000000U)
    {
        return 0;
    }

    if (app_reset < APP_ADDRESS || app_reset > ONCHIP_FLASH_END_ADDRESS)
    {
        return 0;
    }

    return 1;
}

static void Simple_Update_JumpToApp(void)
{
    uint32_t stack_addr;
    uint32_t reset_handler;

    stack_addr = *(volatile uint32_t *)APP_ADDRESS;
    reset_handler = *(volatile uint32_t *)(APP_ADDRESS + 4U);

    __disable_irq();

    HAL_UART_DMAStop(&huart2);
    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart2);

    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /* 清除 SysTick/PendSV 挂起，避免跳转瞬间触发旧中断 */
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;

    /* 清除 Fault 状态，避免 APP 调试时看到 Bootloader 遗留状态 */
    SCB->CFSR = 0xFFFFFFFFU;
    SCB->HFSR = 0xFFFFFFFFU;
    SCB->DFSR = 0xFFFFFFFFU;

    for (uint8_t i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    SCB->VTOR = APP_ADDRESS;
    __set_MSP(stack_addr);
    __set_CONTROL(0);

    __DSB();
    __ISB();

    /* 不在 Bootloader 中重新打开中断，由 APP main() 自己开启 */
    ((void (*)(void))reset_handler)();

    while (1)
    {
    }
}

