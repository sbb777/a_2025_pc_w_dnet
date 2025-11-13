/* USER CODE BEGIN Header */
/**
 * Louis 2025-07-22 15:54:47
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cifXToolkit.h"
#include "SerialDPMInterface.h"
#include "cifXErrors.h"
#include "App_DemoApplication.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define UART_RX_BUFFER_SIZE 256
typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi4;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
// 전역 영역에 선언
volatile char g_szLastCookie[5] = {0};   // 외부 모니터링 가능

uint8_t rx_data[10] = {0};
uint32_t delayCount =0;

uint8_t rxBuffer[64];
uint8_t rxByte;
uint8_t txMessage[] = "STM32 UART5 says hello to netX90!\r\n";
volatile uint8_t rxReady = 0;

static UART_HandleTypeDef *huart_ring;
static RingBuffer rx_ring;

static uint8_t rx_temp; // 수신용 임시 바이트 저장

uint8_t line[100];

/** DIP switch validation variables */
volatile uint8_t g_DipSwitchData[5] = {0};  // Validated DIP switch data: "1010" + null terminator
volatile uint8_t g_bDipDataValid = 0;       // Flag indicating valid DIP data received

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_SPI4_Init(void);
static void MX_UART5_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
static int32_t PreCommunicateWithSPI0(void);
static uint8_t ExtractDipSwitchData(const uint8_t *line, uint8_t *dipData);
static uint8_t ValidateDipSwitchData(const uint8_t *currentLine);
static void UART_Ring_Init(UART_HandleTypeDef *huart);
static int UART_Ring_ReadLine(uint8_t *lineBuf, uint16_t maxLen);
static int UART_Ring_Read(uint8_t *data);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static bool isCookieAvailable(PDEVICEINSTANCE ptDevInstance, uint32_t ulTimeoutInMs)
{
  bool fCookieAvailable = false;
  char szCookie[5];
  uint32_t starttime;
  uint32_t difftime = 0;

  starttime = OS_GetMilliSecCounter();

  while(false == fCookieAvailable && difftime < ulTimeoutInMs)
  {
    OS_Memset(szCookie, 0, sizeof(szCookie));

    HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

    // 전역 변수로 복사 (null termination 포함) Louis
//    szCookie[4] = '\0';  // 안전하게 null-terminated 보장
    OS_Memcpy((void*)g_szLastCookie, szCookie, sizeof(g_szLastCookie));

    /** on DPM cards we need to check the for a valid cookie */
    if((0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_BSL_STR)) || (0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_FW_STR)))
    {
      /** We have a firmware or bootloader running, so we assume it is a flash based device */
      /** NOTE: If the driver is restarted and a RAM based FW was downloaded before this
       will result in the device being handled as flash based.
       Currently there is no way to detect this */
      fCookieAvailable = true;
    }
    else
    {
      fCookieAvailable = false;
      difftime = OS_GetMilliSecCounter() - starttime;
    }
  }

  if(false == fCookieAvailable)
  {
    printf("DPM cookie not available since %u milliseconds\r\n", (unsigned int) ulTimeoutInMs);
  }

  return fCookieAvailable;
}

static int32_t InitializeToolkit()
{
  int32_t lRet = CIFX_NO_ERROR;

  lRet = cifXTKitInit();

  if(CIFX_NO_ERROR == lRet)
  {
    PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE) OS_Memalloc(sizeof(*ptDevInstance));
    OS_Memset(ptDevInstance, 0, sizeof(*ptDevInstance));

    /** Set trace level of toolkit */
    g_ulTraceLevel = TRACE_LEVEL_ERROR |
    TRACE_LEVEL_WARNING |
    TRACE_LEVEL_INFO |
    TRACE_LEVEL_DEBUG;

    /** Insert the basic device information into the DeviceInstance structure
     for the toolkit.
     NOTE: The irq number are for information use only, so we skip them here.
     Interrupt is currently not supported and ignored, so we don't need to set it */
    ptDevInstance->fPCICard = 0;
    ptDevInstance->pvOSDependent = ptDevInstance;
    ptDevInstance->ulDPMSize = 0x8000;           // 32K
    OS_Strncpy(ptDevInstance->szName, "cifX0", sizeof(ptDevInstance->szName));

    /** we know here that netX firmware is flash based, therefore we check if it starts up
     ** by comparing cookie at DPM address 0x00 is valid.*/

    do
    {
      SerialDPM_Init(ptDevInstance);
    } while(false == isCookieAvailable(ptDevInstance, 100));

    /* Add the device to the toolkits handled device list */
    lRet = cifXTKitAddDevice(ptDevInstance);

    /** If it succeeded do device tests */
    if(CIFX_NO_ERROR != lRet)
    {
      /** Uninitialize Toolkit, this will remove all handled boards from the toolkit and
       deallocate the device instance */
      cifXTKitDeinit();
    }
  }

  return lRet;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	 int32_t lRet=CIFX_NO_ERROR;    /* Return value for common error codes      */
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
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USB_HOST_Init();
  MX_TIM3_Init();
  MX_SPI4_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */


  UART_Ring_Init(&huart5);  // 인터럽트 수신 초기화

  // D switch validation loop - simple single reception
  HAL_UART_Transmit(&huart5, (uint8_t *)"Starting D validation loop (waiting for D:[xxxx] + 0x0D)...\r\n", 62, HAL_MAX_DELAY);
  
  // 수정된 부분: 복잡한 누적 로직을 제거하고, 라인 단위로 수신
  while (!g_bDipDataValid)
  {
      int len = UART_Ring_ReadLine(line, sizeof(line));

      if (len > 0)
      {
          // A full line was received. Echo it back.
          HAL_UART_Transmit(&huart5, (uint8_t *)"RX: ", 4, HAL_MAX_DELAY);
          HAL_UART_Transmit(&huart5, line, len, HAL_MAX_DELAY);
          HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

          // Now validate the received line
          if (ValidateDipSwitchData(line) == 1)
          {
              HAL_UART_Transmit(&huart5, (uint8_t *)"D data validated!\r\n", 19, HAL_MAX_DELAY);
              break; // Exit validation loop
          }
          else
          {
              // Validation failed, continue waiting for a new line
              HAL_UART_Transmit(&huart5, (uint8_t *)"Validation FAILED, waiting for new data.\r\n", 42, HAL_MAX_DELAY);
          }
      }
      
      HAL_Delay(100); // Short delay to prevent busy-waiting
  }
  
  lRet = InitializeToolkit();

  if(CIFX_NO_ERROR == lRet)
  {
//    lRet = App_CifXApplicatiomnDemo("cifX0");
    lRet = App_CifXApplicationDemo("cifX0");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI4_Init(void)
{

  /* USER CODE BEGIN SPI4_Init 0 */

  /* USER CODE END SPI4_Init 0 */

  /* USER CODE BEGIN SPI4_Init 1 */

  /* USER CODE END SPI4_Init 1 */
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_MASTER;
  hspi4.Init.Direction = SPI_DIRECTION_2LINES;
  hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi4.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi4.Init.NSS = SPI_NSS_SOFT;
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 200-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SPI5_CS_Pin|GPIO_PIN_6|ACP_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, RDX_Pin|WRX_DCX_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, LD3_Pin|LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : A0_Pin A1_Pin A2_Pin A3_Pin
                           A4_Pin A5_Pin SDNRAS_Pin A6_Pin
                           A7_Pin A8_Pin A9_Pin */
  GPIO_InitStruct.Pin = A0_Pin|A1_Pin|A2_Pin|A3_Pin
                          |A4_Pin|A5_Pin|SDNRAS_Pin|A6_Pin
                          |A7_Pin|A8_Pin|A9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI5_SCK_Pin SPI5_MISO_Pin SPI5_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI5_SCK_Pin|SPI5_MISO_Pin|SPI5_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : ENABLE_Pin */
  GPIO_InitStruct.Pin = ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(ENABLE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SDNWE_Pin */
  GPIO_InitStruct.Pin = SDNWE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(SDNWE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NCS_MEMS_SPI_Pin CSX_Pin OTG_FS_PSO_Pin */
  GPIO_InitStruct.Pin = NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : B1_Pin MEMS_INT1_Pin MEMS_INT2_Pin TP_INT1_Pin */
  GPIO_InitStruct.Pin = B1_Pin|MEMS_INT1_Pin|MEMS_INT2_Pin|TP_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : B5_Pin VSYNC_Pin R4_Pin R5_Pin */
  GPIO_InitStruct.Pin = B5_Pin|VSYNC_Pin|R4_Pin|R5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI5_CS_Pin PA6 ACP_RST_Pin */
  GPIO_InitStruct.Pin = SPI5_CS_Pin|GPIO_PIN_6|ACP_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OC_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : R3_Pin R6_Pin */
  GPIO_InitStruct.Pin = R3_Pin|R6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : A10_Pin A11_Pin BA0_Pin BA1_Pin
                           SDCLK_Pin SDNCAS_Pin */
  GPIO_InitStruct.Pin = A10_Pin|A11_Pin|BA0_Pin|BA1_Pin
                          |SDCLK_Pin|SDNCAS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : D4_Pin D5_Pin D6_Pin D7_Pin
                           D8_Pin D9_Pin D10_Pin D11_Pin
                           D12_Pin NBL0_Pin NBL1_Pin */
  GPIO_InitStruct.Pin = D4_Pin|D5_Pin|D6_Pin|D7_Pin
                          |D8_Pin|D9_Pin|D10_Pin|D11_Pin
                          |D12_Pin|NBL0_Pin|NBL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : G4_Pin G5_Pin B6_Pin B7_Pin */
  GPIO_InitStruct.Pin = G4_Pin|G5_Pin|B6_Pin|B7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : D13_Pin D14_Pin D15_Pin D0_Pin
                           D1_Pin D2_Pin D3_Pin */
  GPIO_InitStruct.Pin = D13_Pin|D14_Pin|D15_Pin|D0_Pin
                          |D1_Pin|D2_Pin|D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : TE_Pin */
  GPIO_InitStruct.Pin = TE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RDX_Pin WRX_DCX_Pin */
  GPIO_InitStruct.Pin = RDX_Pin|WRX_DCX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : R7_Pin DOTCLK_Pin B3_Pin */
  GPIO_InitStruct.Pin = R7_Pin|DOTCLK_Pin|B3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : HSYNC_Pin G6_Pin R2_Pin */
  GPIO_InitStruct.Pin = HSYNC_Pin|G6_Pin|R2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : I2C3_SDA_Pin */
  GPIO_InitStruct.Pin = I2C3_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
  HAL_GPIO_Init(I2C3_SDA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : I2C3_SCL_Pin */
  GPIO_InitStruct.Pin = I2C3_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
  HAL_GPIO_Init(I2C3_SCL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : G7_Pin B2_Pin */
  GPIO_InitStruct.Pin = G7_Pin|B2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : G3_Pin B4_Pin */
  GPIO_InitStruct.Pin = G3_Pin|B4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD4_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : SDCKE1_Pin SDNE1_Pin */
  GPIO_InitStruct.Pin = SDCKE1_Pin|SDNE1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//
//static int32_t PreCommunicateWithSPI0()
//
//{
//
//int32_t lRet = CIFX_NO_ERROR;
//
//uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
//
//uint8_t rx_data[10] = {0};
//
//printf("Starting SPI0 pre-communication with netX90...\r\n");
//
//// SPI0 통신 수행 (예시)
//
//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); //
//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // CS Low
//
//if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000) != HAL_OK)
//
//{
//
//lRet = CIFX_FUNCTION_FAILED;
//
//printf("SPI0 communication failed\r\n");
//
//}
//
//else
//
//{
//
//printf("SPI0 communication successful\r\n");
//
//// 수신된 데이터 처리
//
//for(int i = 0; i < 10; i++)
//
//{
//
//printf("RX[%d]: 0x%02X\r\n", i, rx_data[i]);
//
//}
//
//}
//
//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // CS High
//
//return lRet;
//
//}



// 인터럽트 콜백
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//  if (huart->Instance == UART5)
//  {
//    rxReady = 1;
//  }
//}

// 에러 콜백
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == UART5)
  {
    // 에러 종류 확인 및 처리
    uint32_t error = HAL_UART_GetError(huart);
    
    HAL_UART_Transmit(&huart5, (uint8_t*)"UART Error Code: 0x", 19, HAL_MAX_DELAY);
    
    // 에러 코드를 16진수로 출력
    char errorStr[9];
    sprintf(errorStr, "%08X", (unsigned int)error);
    HAL_UART_Transmit(&huart5, (uint8_t*)errorStr, 8, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
    
    // UART 상태 재설정 및 수신 재시작
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)) {
        __HAL_UART_CLEAR_OREFLAG(huart);
    }
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_NE)) {
        __HAL_UART_CLEAR_NEFLAG(huart);
    }
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_FE)) {
        __HAL_UART_CLEAR_FEFLAG(huart);
    }
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_PE)) {
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
    
    // UART 수신 재시작
    HAL_UART_Receive_IT(huart, &rx_temp, 1);
  }
}


//
//static int32_t PreCommunicateWithSPI0()
//{
//    int32_t lRet = CIFX_NO_ERROR;
//    uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
////    uint8_t rx_data[10] = {0};
//    uint32_t success_count = 0;
//
//    printf("Starting SPI0 pre-communication with netX90 (20 attempts)...\r\n");
//
//    // 20회 반복
//    for(int attempt = 1; attempt <= 20; attempt++)
//    {
//        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // CS Low
////        HAL_Delay(1);
//        for (volatile int i = 0; i < 1000; i++) __NOP();  // 약 100~150μs
//
//        if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000) != HAL_OK)
//        {
//            printf("Attempt %d: FAILED\r\n", attempt);
//        }
//        else
//        {
//            printf("Attempt %d: SUCCESS\r\n", attempt);
//            success_count++;
//        }
//
//        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // CS High
//        HAL_Delay(100); // 100ms 대기
//    }
//
//    printf("Success: %lu/20 attempts\r\n", success_count);
//
//    if(success_count > 0)
//        lRet = CIFX_NO_ERROR;
//    else
//        lRet = CIFX_FUNCTION_FAILED;
//
//    return lRet;
//}




void UART_Ring_Init(UART_HandleTypeDef *huart)
{
    huart_ring = huart;
    rx_ring.head = 0;
    rx_ring.tail = 0;

    // 첫 1바이트 수신 시작
    HAL_UART_Receive_IT(huart_ring, &rx_temp, 1);
}

// 인터럽트 콜백: 수신 완료 시 호출됨
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	  if (huart->Instance == UART5)
	  {
	    rxReady = 1;
	  }


    if (huart == huart_ring)
    {
        uint16_t next_head = (rx_ring.head + 1) % UART_RX_BUFFER_SIZE;

        // 버퍼가 가득 차면 덮어쓰지 않음 (오버플로우 방지)
        if (next_head != rx_ring.tail)
        {
            rx_ring.buffer[rx_ring.head] = rx_temp;
            rx_ring.head = next_head;
        }

        // 다음 수신 요청
        HAL_UART_Receive_IT(huart_ring, &rx_temp, 1);
    }
}

// Ring buffer에서 1바이트 읽기
int UART_Ring_Read(uint8_t *data)
{
    if (rx_ring.head == rx_ring.tail)
    {
        return 0; // 데이터 없음
    }

    *data = rx_ring.buffer[rx_ring.tail];
    rx_ring.tail = (rx_ring.tail + 1) % UART_RX_BUFFER_SIZE;
    return 1;
}


// 수정된 부분: 라인 종결 문자를 '\r' (0x0D)로 변경
// 줄 단위 수신 (0x0D로 종료되는 문자열)
int UART_Ring_ReadLine(uint8_t *lineBuf, uint16_t maxLen)
{
    uint16_t i = 0;
    uint8_t ch;

    // Read from the ring buffer until a line is complete or buffer is empty
    while (i < maxLen - 1)
    {
        if (!UART_Ring_Read(&ch))
        {
            // No new character available yet, return 0 to signal no complete line
            return 0;
        }

        if (ch == '\r')
        {
            // Found the carriage return terminator
            lineBuf[i] = '\0'; // Replace \r with null terminator
            return i;
        }
        else
        {
            lineBuf[i++] = ch;
        }
    }
    
    // Buffer overflow or no terminator found within maxLen.
    lineBuf[i] = '\0';
    return i;
}



/*!
 * \brief Extract DIP switch data from received UART line
 * \param[in] line - Received UART line buffer
 * \param[out] dipData - Extracted 4-bit DIP switch data (as string)
 * \return 1 if valid DIP data found, 0 otherwise
 */
static uint8_t ExtractDipSwitchData(const uint8_t *line, uint8_t *dipData)
{
    // Look for pattern "D:[XXXX]" where XXXX is 4 binary digits
    const char *dipStart = strstr((const char*)line, "D:[");
    
    if (dipStart == NULL) {
        HAL_UART_Transmit(&huart5, (uint8_t *)"No D pattern found\r\n", 20, HAL_MAX_DELAY);
        return 0;
    }
    
    dipStart += 3; // Move to start of binary digits
    
    // Validate 4 binary digits
    for (int i = 0; i < 4; i++)
    {
        if (dipStart[i] != '0' && dipStart[i] != '1') {
            HAL_UART_Transmit(&huart5, (uint8_t *)"Invalid D digit at pos ", 23, HAL_MAX_DELAY);
            char posStr[2] = {'0' + i, '\0'};
            HAL_UART_Transmit(&huart5, (uint8_t *)posStr, 1, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
            return 0;
        }
        dipData[i] = dipStart[i];
    }
    
    // Check for closing bracket
    if (dipStart[4] != ']') {
        HAL_UART_Transmit(&huart5, (uint8_t *)"Missing closing bracket\r\n", 25, HAL_MAX_DELAY);
        return 0;
    }
    
    dipData[4] = '\0'; // Null terminate
    
    HAL_UART_Transmit(&huart5, (uint8_t *)"D extracted: ", 13, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, dipData, 4, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    
    return 1;
}

/*!
 * \brief Validate D switch data with single reception
 * \param[in] currentLine - Current received UART line
 * \return 1 if validation successful, 0 if extraction failed
 */
static uint8_t ValidateDipSwitchData(const uint8_t *currentLine)
{
    uint8_t currentDipData[5] = {0};
    
    HAL_UART_Transmit(&huart5, (uint8_t *)"Validating line: ", 17, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)currentLine, strlen((const char*)currentLine), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    
    // Extract D data from current line
    if (ExtractDipSwitchData(currentLine, currentDipData) == 0) {
        HAL_UART_Transmit(&huart5, (uint8_t *)"D extraction failed\r\n", 21, HAL_MAX_DELAY);
        return 0; // No valid D data in current line
    }
    
    // Single reception validation - no comparison needed
    for(int i = 0; i < 5; i++) g_DipSwitchData[i] = currentDipData[i];
    g_bDipDataValid = 1;
    
    HAL_UART_Transmit(&huart5, (uint8_t *)"D validation SUCCESS: ", 22, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t*)g_DipSwitchData, 4, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    
    return 1; // Validation successful immediately
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  * where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */