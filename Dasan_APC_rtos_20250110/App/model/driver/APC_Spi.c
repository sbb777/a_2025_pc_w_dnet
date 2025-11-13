/*
 * APC_Spi.c
 *
 *  Created on: Oct 30, 2023
 *      Author: Yongseok
 */

#include <APC_Spi.h>
#include <APC_Utils.h>

#include "APC_Timer.h"

//#include "fsmc.h"
#include <APC_Define.h>

#ifdef __REV_MTECH__
#else
extern SRAM_HandleTypeDef hsram3;
extern SRAM_HandleTypeDef hsram4;
#endif

static uint8_t _spi_option = 0;

void initSpi(void)
{

}

void setSpiOption(uint8_t option)
{
    if (_spi_option != option) {
        _spi_option = option;

        HAL_SPI_DeInit(&hspi1);
        MX_SPI1_Init();
    }
}

uint8_t getSpiOption()
{
    return _spi_option;
}

void setSpiDataMode(uint8_t dataMode)
{
    //if (hspi1.State != HAL_SPI_STATE_READY) return;

    switch (dataMode)
    {
        // CPOL=0, CPHA=0
        case SPI_MODE0:
            hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
            hspi1.Init.CLKPhase    = SPI_PHASE_1EDGE;
            HAL_SPI_Init(&hspi1);
            break;
        // CPOL=0, CPHA=1
        case SPI_MODE1:
            hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
            hspi1.Init.CLKPhase    = SPI_PHASE_2EDGE;
            HAL_SPI_Init(&hspi1);
            break;
        // CPOL=1, CPHA=0
        case SPI_MODE2:
            hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
            hspi1.Init.CLKPhase    = SPI_PHASE_1EDGE;
            HAL_SPI_Init(&hspi1);
            break;
        // CPOL=1, CPHA=1
        case SPI_MODE3:
            hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
            hspi1.Init.CLKPhase    = SPI_PHASE_2EDGE;
            HAL_SPI_Init(&hspi1);
            break;
    }
}

void set_SPI5_SpiDataMode(uint8_t dataMode)
{
    //if (hspi1.State != HAL_SPI_STATE_READY) return;

    switch (dataMode)
    {
        // CPOL=0, CPHA=0
        case SPI_MODE0:
        	hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
        	hspi5.Init.CLKPhase    = SPI_PHASE_1EDGE;
					HAL_SPI_Init(&hspi5);
					break;
        // CPOL=0, CPHA=1
        case SPI_MODE1:
        	hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
        	hspi5.Init.CLKPhase    = SPI_PHASE_2EDGE;
					HAL_SPI_Init(&hspi5);
					break;
        // CPOL=1, CPHA=0
        case SPI_MODE2:
        	hspi5.Init.CLKPolarity = SPI_POLARITY_HIGH;
        	hspi5.Init.CLKPhase    = SPI_PHASE_1EDGE;
					HAL_SPI_Init(&hspi5);
					break;
        // CPOL=1, CPHA=1
        case SPI_MODE3:
        	hspi5.Init.CLKPolarity = SPI_POLARITY_HIGH;
        	hspi5.Init.CLKPhase    = SPI_PHASE_2EDGE;
					HAL_SPI_Init(&hspi5);
					break;
    }
}

void setSpiBitWidth(uint8_t bit_width)
{
    //if (hspi1.State != HAL_SPI_STATE_READY) return;

    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;

    if (bit_width == 16)
    {
        hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    }
    HAL_SPI_Init(&hspi1);
}

uint8_t receiveSpi(uint8_t *pData, uint8_t len)
{
    if (hspi1.State != HAL_SPI_STATE_READY) return 0;

    HAL_StatusTypeDef res = HAL_SPI_Receive(&hspi1, (uint8_t *)pData, len, 100);
    //APC_Delay(1);
    return (uint8_t) res;
}

uint8_t transmitSpi(uint8_t *pData, uint8_t len)
{
    if (hspi1.State != HAL_SPI_STATE_READY) return 0;

    HAL_StatusTypeDef res = HAL_SPI_Transmit(&hspi1, (uint8_t *)pData, len, 100);
    //APC_Delay(1);
    return (uint8_t) res;
}

/**
 * read configuration
 ********************************************************
 * spi1 -> spi5 GPIOB.14
 ********************************************************
 * 25c080 in MASTER spi5
 * #define MPU_SPI5_CFGCS__Pin GPIO_PIN_14
 * #define MPU_SPI5_CFGCS__GPIO_Port GPIOB
 ********************************************************
 */
uint8_t receive_SPI5_Spi(uint8_t *pData, uint8_t len)
{
    if (hspi5.State != HAL_SPI_STATE_READY) return 0;

    HAL_StatusTypeDef res = HAL_SPI_Receive(&hspi5, (uint8_t *)pData, len, 100);
    APC_Delay(1);
    return (uint8_t) res;
}

uint8_t transmit_SPI5_Spi(uint8_t *pData, uint8_t len)
{
    if (hspi5.State != HAL_SPI_STATE_READY) return 0;

    HAL_StatusTypeDef res = HAL_SPI_Transmit(&hspi5, (uint8_t *)pData, len, 100);
    APC_Delay(1);
    return (uint8_t) res;
}


uint8_t transfer8Spi(uint8_t data)
{
    if (hspi1.State != HAL_SPI_STATE_READY) return 0;

    uint8_t ret;
    HAL_SPI_TransmitReceive(&hspi1, &data, &ret, 1, 100);
    //APC_Delay(1);
    return ret;
}

uint16_t transfer16Spi(uint16_t data)
{
    if (hspi1.State != HAL_SPI_STATE_READY) return 0;

    uint16_t ret;
    if (hspi1.Init.DataSize == SPI_DATASIZE_8BIT)
    {
        uint8_t tBuf[2];
        uint8_t rBuf[2];

        tBuf[0] = (uint8_t)(data>>8);
        tBuf[1] = (uint8_t) data;
        HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)tBuf, (uint8_t *)rBuf, 2, 100);

        ret = rBuf[0];
        ret <<= 8;
        ret += rBuf[1];
    }
    else
    {
        HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&data, (uint8_t *)&ret, 1, 100);
    }
    //APC_Delay(1);

    return ret;
}

bool transferSpi(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t length, uint32_t timeout)
{
    HAL_StatusTypeDef status;

    if (rx_buf == NULL)
    {
        status =  HAL_SPI_Transmit(&hspi1, tx_buf, length, timeout);
    }
    else if (tx_buf == NULL)
    {
        status =  HAL_SPI_Receive(&hspi1, rx_buf, length, timeout);
    }
    else
    {
        status =  HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, length, timeout);
    }
    //APC_Delay(1);

    return (status == HAL_OK) ? true : false;
}

bool transfer_VDRUCK_Spi(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t length, uint32_t timeout)
{
    HAL_StatusTypeDef status;

    if (rx_buf == NULL)
    {
        status =  HAL_SPI_Transmit(&hspi5, tx_buf, length, timeout);
    }
    else if (tx_buf == NULL)
    {
        status =  HAL_SPI_Receive(&hspi5, rx_buf, length, timeout);
    }
    else
    {
        status =  HAL_SPI_TransmitReceive(&hspi5, tx_buf, rx_buf, length, timeout);
    }
    //APC_Delay(1);

    return (status == HAL_OK) ? true : false;
}


bool transferDspSpi(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t length, uint32_t timeout)
{
    HAL_StatusTypeDef status;

    if (rx_buf == NULL)
    {
        status =  HAL_SPI_Transmit(&hspi5, tx_buf, length, timeout);
    }
    else if (tx_buf == NULL)
    {
        status =  HAL_SPI_Receive(&hspi5, rx_buf, length, timeout);
    }
    else
    {
        status =  HAL_SPI_TransmitReceive(&hspi5, tx_buf, rx_buf, length, timeout);
    }
    //APC_Delay(1);

    return (status == HAL_OK) ? true : false;
}

