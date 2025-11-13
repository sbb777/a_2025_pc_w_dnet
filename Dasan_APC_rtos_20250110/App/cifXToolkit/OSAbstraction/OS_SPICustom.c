/**
 * @file OS_SPICustom.c
 * @brief STM32 HAL SPI5 Abstraction Layer for cifXToolkit SerialDPM
 * @date 2025-01-10
 *
 * This file provides STM32 HAL SPI5-specific implementations for accessing
 * the netX 90 chip's DPM (Dual Port Memory) via SPI interface.
 *
 * Hardware Configuration:
 * - SPI5 connected to netX 90
 * - CS (Chip Select) GPIO pin
 * - SRDY (Service Ready) GPIO input pin
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* TODO: Define GPIO pins for SPI5 handshaking */
#define NETX_CS_PIN         GPIO_PIN_6    /* Example: SPI5_CS on PF6 */
#define NETX_CS_PORT        GPIOF

#define NETX_SRDY_PIN       GPIO_PIN_7    /* Example: SRDY on PF7 */
#define NETX_SRDY_PORT      GPIOF

/* SPI timeout in milliseconds */
#define SPI_TIMEOUT_MS      1000

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi5;  /* SPI5 handle - defined in main.c or spi.c */

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize SPI interface
 * @return 0 on success, negative on error
 */
int32_t SPI_Init(void)
{
    /* GPIO initialization for handshaking */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOF_CLK_ENABLE();  /* Adjust based on actual GPIO port */

    /* Configure CS pin as output */
    GPIO_InitStruct.Pin = NETX_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(NETX_CS_PORT, &GPIO_InitStruct);

    /* Set CS high (inactive) */
    HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_SET);

    /* Configure SRDY pin as input */
    GPIO_InitStruct.Pin = NETX_SRDY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(NETX_SRDY_PORT, &GPIO_InitStruct);

    /* SPI5 should already be initialized by STM32CubeMX */
    /* Verify SPI5 is configured:
     * - Mode: Master
     * - Direction: Full Duplex
     * - Data Size: 8 bits
     * - Clock Polarity: Low (CPOL = 0)
     * - Clock Phase: 1 Edge (CPHA = 0)
     * - NSS: Software
     * - BaudRate: Up to 20 MHz (adjust based on APB clock)
     * - FirstBit: MSB
     */

    return 0;
}

/**
 * @brief Deinitialize SPI interface
 * @return 0 on success, negative on error
 */
int32_t SPI_Deinit(void)
{
    /* Set CS high (inactive) */
    HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_SET);

    return 0;
}

/**
 * @brief Perform SPI transfer (send and receive simultaneously)
 * @param pbSend Pointer to send buffer
 * @param pbRecv Pointer to receive buffer
 * @param ulLen Length of transfer in bytes
 * @return 0 on success, negative on error
 */
int32_t SPI_Transfer(uint8_t* pbSend, uint8_t* pbRecv, uint32_t ulLen)
{
    HAL_StatusTypeDef status;

    if (pbSend == NULL || pbRecv == NULL || ulLen == 0)
        return -1;

    /* Perform full-duplex transfer */
    status = HAL_SPI_TransmitReceive(&hspi5, pbSend, pbRecv, ulLen, SPI_TIMEOUT_MS);

    if (status != HAL_OK)
        return -1;

    return 0;
}

/**
 * @brief Set Chip Select signal
 * @param bActive 1 to activate (low), 0 to deactivate (high)
 * @return 0 on success, negative on error
 */
int32_t SPI_SetCS(uint8_t bActive)
{
    if (bActive)
    {
        /* CS active - set low */
        HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_RESET);
    }
    else
    {
        /* CS inactive - set high */
        HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_SET);
    }

    return 0;
}

/**
 * @brief Get SRDY (Service Ready) signal state
 * @return 1 if SRDY is active (high), 0 if inactive (low), negative on error
 */
int32_t SPI_GetSRDY(void)
{
    GPIO_PinState state;

    state = HAL_GPIO_ReadPin(NETX_SRDY_PORT, NETX_SRDY_PIN);

    /* SRDY is active high */
    return (state == GPIO_PIN_SET) ? 1 : 0;
}

/**
 * @brief Wait for SRDY signal with timeout
 * @param ulTimeoutMs Timeout in milliseconds
 * @return 0 on success (SRDY active), -1 on timeout
 */
int32_t SPI_WaitSRDY(uint32_t ulTimeoutMs)
{
    uint32_t startTime = HAL_GetTick();

    while ((HAL_GetTick() - startTime) < ulTimeoutMs)
    {
        if (SPI_GetSRDY() == 1)
            return 0;  /* SRDY is active */
    }

    return -1;  /* Timeout */
}
