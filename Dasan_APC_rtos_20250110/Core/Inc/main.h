/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PFO_ON_Pin GPIO_PIN_4
#define PFO_ON_GPIO_Port GPIOE
#define MPU_D23VPFO_ON_Pin GPIO_PIN_6
#define MPU_D23VPFO_ON_GPIO_Port GPIOF
#define DSP_RS_Pin GPIO_PIN_8
#define DSP_RS_GPIO_Port GPIOF
#define MPU_OPEN_Pin GPIO_PIN_10
#define MPU_OPEN_GPIO_Port GPIOF
#define XTAL1_Pin GPIO_PIN_0
#define XTAL1_GPIO_Port GPIOH
#define XTAL2_Pin GPIO_PIN_1
#define XTAL2_GPIO_Port GPIOH
#define MPU_USART4_TX_Pin GPIO_PIN_0
#define MPU_USART4_TX_GPIO_Port GPIOA
#define MPU_USART4_RX_Pin GPIO_PIN_1
#define MPU_USART4_RX_GPIO_Port GPIOA
#define MPU_USART2_TX_Pin GPIO_PIN_2
#define MPU_USART2_TX_GPIO_Port GPIOA
#define MPU_USART2_RX_Pin GPIO_PIN_3
#define MPU_USART2_RX_GPIO_Port GPIOA
#define MPU_USART2_ENB_Pin GPIO_PIN_4
#define MPU_USART2_ENB_GPIO_Port GPIOA
#define MPU_SPI1_CLK_Pin GPIO_PIN_5
#define MPU_SPI1_CLK_GPIO_Port GPIOA
#define MPU_SPI1_MISO_Pin GPIO_PIN_6
#define MPU_SPI1_MISO_GPIO_Port GPIOA
#define MPU_SPI1_MOSI_Pin GPIO_PIN_7
#define MPU_SPI1_MOSI_GPIO_Port GPIOA
#define MPU_CLK_AD_Pin GPIO_PIN_0
#define MPU_CLK_AD_GPIO_Port GPIOB
#define MPU_SW_RST__Pin GPIO_PIN_1
#define MPU_SW_RST__GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define MPU_CLOSE_Pin GPIO_PIN_11
#define MPU_CLOSE_GPIO_Port GPIOF
#define MPU_DATA_AD1_Pin GPIO_PIN_10
#define MPU_DATA_AD1_GPIO_Port GPIOB
#define MPU_DATA_AD2_Pin GPIO_PIN_11
#define MPU_DATA_AD2_GPIO_Port GPIOB
#define MPU_SPI5_MDVDCS__Pin GPIO_PIN_12
#define MPU_SPI5_MDVDCS__GPIO_Port GPIOB
#define MPU_SPI5_DSPCS__Pin GPIO_PIN_13
#define MPU_SPI5_DSPCS__GPIO_Port GPIOB
#define MPU_SPI5_CFGCS__Pin GPIO_PIN_14
#define MPU_SPI5_CFGCS__GPIO_Port GPIOB
#define IO_4_Pin GPIO_PIN_15
#define IO_4_GPIO_Port GPIOB
#define MPU_ID_0_Pin GPIO_PIN_6
#define MPU_ID_0_GPIO_Port GPIOG
#define MPU_ID_1_Pin GPIO_PIN_7
#define MPU_ID_1_GPIO_Port GPIOG
#define MPU_ID_2_Pin GPIO_PIN_8
#define MPU_ID_2_GPIO_Port GPIOG
#define USART1_TXD_Pin GPIO_PIN_9
#define USART1_TXD_GPIO_Port GPIOA
#define USART1_RXD_Pin GPIO_PIN_10
#define USART1_RXD_GPIO_Port GPIOA
#define DB_SWDIO_Pin GPIO_PIN_13
#define DB_SWDIO_GPIO_Port GPIOA
#define DB_SWCLK_Pin GPIO_PIN_14
#define DB_SWCLK_GPIO_Port GPIOA
#define LD1_LED_Pin GPIO_PIN_2
#define LD1_LED_GPIO_Port GPIOD
#define LD2_LED_Pin GPIO_PIN_3
#define LD2_LED_GPIO_Port GPIOD
#define MPU_RD__Pin GPIO_PIN_4
#define MPU_RD__GPIO_Port GPIOD
#define MPU_WR__Pin GPIO_PIN_5
#define MPU_WR__GPIO_Port GPIOD
#define MPU_NOR_CS__Pin GPIO_PIN_7
#define MPU_NOR_CS__GPIO_Port GPIOD
#define MPU_TMC_RST__Pin GPIO_PIN_3
#define MPU_TMC_RST__GPIO_Port GPIOB
#define MPU_SPI1_PMONCS__Pin GPIO_PIN_4
#define MPU_SPI1_PMONCS__GPIO_Port GPIOB
#define MPU_SPI1_PCFGCS__Pin GPIO_PIN_5
#define MPU_SPI1_PCFGCS__GPIO_Port GPIOB
#define ID_0_Pin GPIO_PIN_8
#define ID_0_GPIO_Port GPIOB
#define MPU_IRQ_D24PFO_Pin GPIO_PIN_9
#define MPU_IRQ_D24PFO_GPIO_Port GPIOB
#define NBL0__Pin GPIO_PIN_0
#define NBL0__GPIO_Port GPIOE
#define NBL1__Pin GPIO_PIN_1
#define NBL1__GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
