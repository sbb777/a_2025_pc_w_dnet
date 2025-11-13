/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f7xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */
	  /*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	SPIx_SCK_GPIO_CLK_ENABLE();
	SPIx_MISO_GPIO_CLK_ENABLE();
	SPIx_MOSI_GPIO_CLK_ENABLE();
	/* Enable SPI clock */
	SPIx_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* SPI SCK GPIO pin configuration  */
	GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = SPIx_SCK_AF;
	HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

	/* SPI MISO GPIO pin configuration  */
	GPIO_InitStruct.Pin = SPIx_MISO_PIN;
	GPIO_InitStruct.Alternate = SPIx_MISO_AF;
	HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

	/* SPI MOSI GPIO pin configuration  */
	GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
	GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
	HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);


	/*##-3- Configure the NVIC for SPI #########################################*/
	/* NVIC for SPI */
	HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(SPIx_IRQn);
  /* USER CODE END SPI1_MspInit 0 */
#if 0
    /* Peripheral clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif
  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }

}

/**
* @brief SPI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */
	  /*##-1- Reset peripherals ##################################################*/
   SPIx_FORCE_RESET();
   SPIx_RELEASE_RESET();

   /*##-2- Disable peripherals and GPIO Clocks ################################*/
   /* Configure SPI SCK as alternate function  */
   HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
   /* Configure SPI MISO as alternate function  */
   HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
   /* Configure SPI MOSI as alternate function  */
   HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);

   /*##-3- Disable the NVIC for SPI ###########################################*/
   HAL_NVIC_DisableIRQ(SPIx_IRQn);
  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
#if 0
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
#endif
  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
