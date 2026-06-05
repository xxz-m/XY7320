/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx_it.h"
#include "string.h"
#include "bsp_flash.h"
#include "firmware_manage.h"
#include "bootloader_define.h"
#include "simple_update.h"
#include "boot_jump.h"
extern uint8_t uart2_rx_buf[UART2_RX_BUF_SIZE];
extern uint8_t uart2_proc_buf[UART2_RX_BUF_SIZE];
extern volatile uint16_t uart2_rx_len;
extern volatile uint8_t uart2_rx_done;
extern volatile uint8_t uart2_rx_overflow;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// FLASH_TEST_ABS_ADDR   测试绝对地址，Sector 11 起始地址
// FLASH_TEST_OFFSET     APP 分区内偏移
// FLASH_TEST_ERASE_SIZE 擦除 128KB，也就是 Sector 11
// FLASH_TEST_MAX_SIZE   最大测试 1024 字节
#define FLASH_TEST_ABS_ADDR      0x080E0000U
#define FLASH_TEST_OFFSET        (FLASH_TEST_ABS_ADDR - APP_ADDRESS)
#define FLASH_TEST_ERASE_SIZE    0x20000U
#define FLASH_TEST_MAX_SIZE      1024U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void MOTA_Flash_Test_By_Uart_Data(uint8_t *data, uint16_t len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  BSP_Printf("bootloader start\r\n");
  Simple_Update_Init();

  BSP_Printf("waiting upgrade...\r\n");

  HAL_UART_Receive_DMA(&huart2, uart2_rx_buf, UART2_RX_BUF_SIZE);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (uart2_rx_done)
    {
      uart2_rx_done = 0;
      BSP_Printf("USART2 RX len = %d\r\n", uart2_rx_len);
      Simple_Update_Process(uart2_proc_buf, uart2_rx_len);
    }

    if (uart2_rx_overflow)
    {
      uart2_rx_overflow = 0;
      BSP_Printf("USART2 RX overflow, slow down sender\r\n");
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void MOTA_Flash_Test_By_Uart_Data(uint8_t *data, uint16_t len)
{
  struct BSP_FLASH *app_part = NULL;
  uint8_t read_buf[FLASH_TEST_MAX_SIZE];
  uint16_t test_len = len;

  if (data == NULL || len == 0)
  {
    BSP_Printf("[flash test] invalid data\r\n");
    return;
  }

  if (test_len > FLASH_TEST_MAX_SIZE)
  {
    test_len = FLASH_TEST_MAX_SIZE;
    BSP_Printf("[flash test] data too long, cut to %d\r\n", test_len);
  }

  /*
   * 当前 flash_port_stm32f4.c 是按 32bit word 写入。
   * 测试阶段建议长度按 4 字节对齐。
   */
  if ((test_len % 4U) != 0U)
  {
    test_len = (test_len + 3U) & ~3U;
    BSP_Printf("[flash test] align len to %d\r\n", test_len);
  }

  memset(read_buf, 0, sizeof(read_buf));

  app_part = BSP_Flash_GetHandle(APP_PART_NAME);
  if (app_part == NULL)
  {
    BSP_Printf("[flash test] get app part fail\r\n");
    return;
  }

  BSP_Printf("[flash test] app addr: 0x%08lX, len: 0x%08lX\r\n",
         app_part->addr,
         app_part->len);

  BSP_Printf("[flash test] erase offset: 0x%08lX, size: 0x%08lX\r\n",
         (uint32_t)FLASH_TEST_OFFSET,
         (uint32_t)FLASH_TEST_ERASE_SIZE);

  if (BSP_Flash_Erase(app_part, FLASH_TEST_OFFSET, FLASH_TEST_ERASE_SIZE) < 0)
  {
    BSP_Printf("[flash test] erase fail\r\n");
    return;
  }

  BSP_Printf("[flash test] erase ok\r\n");

  if (BSP_Flash_Write(app_part, FLASH_TEST_OFFSET, data, test_len) < 0)
  {
    BSP_Printf("[flash test] write fail\r\n");
    return;
  }

  BSP_Printf("[flash test] write ok\r\n");

  if (BSP_Flash_Read(app_part, FLASH_TEST_OFFSET, read_buf, test_len) < 0)
  {
    BSP_Printf("[flash test] read fail\r\n");
    return;
  }

  BSP_Printf("[flash test] read ok\r\n");

  if (memcmp(read_buf, data, len) == 0)
  {
    BSP_Printf("[flash test] compare ok, raw len = %d, write len = %d\r\n",
           len,
           test_len);
  }
  else
  {
    BSP_Printf("[flash test] compare fail\r\n");

    BSP_Printf("[flash test] send data first 16 bytes:\r\n");
    for (uint16_t i = 0; i < len && i < 16; i++)
    {
      BSP_Printf("%02X ", data[i]);
    }
    BSP_Printf("\r\n");

    BSP_Printf("[flash test] read data first 16 bytes:\r\n");
    for (uint16_t i = 0; i < len && i < 16; i++)
    {
      BSP_Printf("%02X ", read_buf[i]);
    }
    BSP_Printf("\r\n");
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: BSP_Printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
