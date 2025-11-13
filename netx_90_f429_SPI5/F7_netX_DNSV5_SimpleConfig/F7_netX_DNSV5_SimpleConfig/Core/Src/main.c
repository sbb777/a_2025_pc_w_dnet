/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OS_Includes.h"
#include "OS_Spi.h"
#include "cifXEndianess.h"
#include "cifXErrors.h"
#include "cifXToolkit.h"
#include "SerialDPMInterface.h"
#include <stdbool.h>
#include "App_DemoApplication.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

extern SPI_HandleTypeDef SpiHandle;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
/*****************************************************************************/
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

		/** on DPM cards we need to check the for a valid cookie */
		if( (0 == OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_BSL_STR)) ||
				(0 == OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_FW_STR)) )
		{
			/** We have a firmware or bootloader running, so we assume it is a flash based device */
			/** NOTE: If the driver is restarted and a RAM based FW was downloaded before this
           will result in the device being handled as flash based.
           Currently there is no way to detect this */
			fCookieAvailable = true;
		}else
		{
			fCookieAvailable = false;
			difftime = OS_GetMilliSecCounter() - starttime;
		}
	}
	if(false == fCookieAvailable)
	{
		;//printf("DPM cookie not available since %u milliseconds\r\n", (unsigned int)ulTimeoutInMs);
	}
	return fCookieAvailable;
}
/*****************************************************************************/
static int32_t InitializeToolkit(){
	int32_t      lRet              = 0;                                                  /* Return value for common error codes      */

	/* First of all initialize toolkit */
	lRet = cifXTKitInit();


	if(CIFX_NO_ERROR == lRet)
	{
		DEVICEINSTANCE *ptDevInstance = (DEVICEINSTANCE*)OS_Memalloc(sizeof(*ptDevInstance));
		OS_Memset(ptDevInstance, 0, sizeof(*ptDevInstance));

		/* Set trace level of toolkit */
		g_ulTraceLevel = TRACE_LEVEL_ERROR   |
				TRACE_LEVEL_WARNING |
				TRACE_LEVEL_INFO    |
				TRACE_LEVEL_DEBUG;

		/* Insert the basic device information , into the DeviceInstance structure
       for the toolkit. The DPM address must be zero, as we only transfer address
       offsets via the SPI interface.
       NOTE: The physical address and irq number are for information use
             only, so we skip them here. Interrupt is currently not supported
             and ignored, so we dont need to set it */
		ptDevInstance->fPCICard          = 0;
		ptDevInstance->pvOSDependent     = ptDevInstance;
		ptDevInstance->ulDPMSize         = 0x8000;//0x10000;
		OS_Strncpy(ptDevInstance->szName, "cifX0", sizeof(ptDevInstance->szName));

		/* netX needs some time until SPM is ready for netX type autodetection */
		OS_Sleep(500);

		/* netX type corresponding SPM initialization */
		printf("netX type detection and SPM initialisation ...\n\r");
		lRet=SerialDPM_Init(ptDevInstance);
		printf("netX type 0x%02x\n\r",(char)lRet);


		/** we know that netX firmware is flash based in this application, therefore we check if it starts up
		 ** by comparing cookie at DPM address 0x00 is valid.*/
		while( false == isCookieAvailable(ptDevInstance, 100) );

		/* Add the device to the toolkits handled device list */
		lRet = cifXTKitAddDevice(ptDevInstance);
	}
	return lRet;
}
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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  lRet = InitializeToolkit();

  if(CIFX_NO_ERROR == lRet)
  {
    lRet = App_CifXApplicationDemo("cifX0");
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */
	/*##-1- Configure the SPI peripheral #######################################*/
	/* Set the SPI parameters */
	SpiHandle.Instance               = SPIx;
	SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
	SpiHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
	SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
	SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
	SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
	SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	SpiHandle.Init.CRCPolynomial     = 7;
	SpiHandle.Init.NSS               = SPI_NSS_SOFT; //SPI_NSS_HARD_OUTPUT;  // original: SPI_NSS_SOFT
	SpiHandle.Init.Mode              = SPI_MODE_MASTER;

	if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/* Configure the GPIO pin D14 as SPI CS  */
	GPIO_InitTypeDef  GPIO_InitStruct;
	GPIO_InitStruct.Pin = SPM_CS_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	__HAL_RCC_GPIOD_CLK_ENABLE();
	HAL_GPIO_Init(SPM_CS_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOD, SPM_CS_PIN, GPIO_PIN_SET);
  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
#if 0
  SpiHandle.Instance = SPIx;
  SpiHandle.Init.Mode = SPI_MODE_MASTER;
  SpiHandle.Init.Direction = SPI_DIRECTION_2LINES;
  SpiHandle.Init.DataSize = SPI_DATASIZE_8BIT;//SPI_DATASIZE_4BIT;
  SpiHandle.Init.CLKPolarity = SPI_POLARITY_HIGH;//SPI_POLARITY_LOW;
  SpiHandle.Init.CLKPhase = SPI_PHASE_2EDGE;//SPI_PHASE_1EDGE;
  SpiHandle.Init.NSS = SPI_NSS_SOFT;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  SpiHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.TIMode = SPI_TIMODE_DISABLE;
  SpiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  SpiHandle.Init.CRCPolynomial = 7;
  SpiHandle.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  SpiHandle.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&SpiHandle) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */
  /* Configure the GPIO pin D14 as SPI CS  */
  	GPIO_InitTypeDef  GPIO_InitStruct;
  	GPIO_InitStruct.Pin = GPIO_PIN_14;
  	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  	GPIO_InitStruct.Pull = GPIO_NOPULL;
  	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  	__HAL_RCC_GPIOD_CLK_ENABLE();
  	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  /* USER CODE END SPI1_Init 2 */
#endif
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
