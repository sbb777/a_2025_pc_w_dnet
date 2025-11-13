/*
 * APC_E2prom.c
 *
 *  Created on: 2023. 10. 23.
 *      Author: Yongseok
 */

#include "APC_E2prom.h"

#include "APC_Utils.h"
#include <APC_ChipSelect.h>
#include <APC_Spi.h>


#define  E2PROM_READ    0x03    // Read data from memory array beginning at selected address
#define  E2PROM_WRITE   0x02    // Write data to memory array beginning at selected address
#define  E2PROM_WREN    0x06    // Set the write enable latch (enable write operations)
#define  E2PROM_RDSR    0x05    // Read Status register
#define  E2PROM_WRSR    0x01    // Write Status register


static bool _eeprom_intialized = false;

void initE2prom(void)
{
    _eeprom_intialized = true;
}

/**
 * read configuration
 ********************************************************
 * spi1 -> spi5
 ********************************************************
 * 25c080 in MASTER spi5
 * #define MPU_SPI5_CFGCS__Pin GPIO_PIN_14
 * #define MPU_SPI5_CFGCS__GPIO_Port GPIOB
 ********************************************************
 */
bool getE2promData(uint16_t addr, uint8_t data)
{
#ifdef __REV_MTECH__
    //setSpiOption(SPI_OPTION_DEFAULT);
		set_SPI5_SpiDataMode(SPI_MODE0);
    //setChipSelect(SPI_CS_E2_, APC_LOW);
    HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_RESET);
    uint8_t send[3] = { 0 };
    send[0] = E2PROM_READ;
    send[1] = addr >> 8;
    send[2] = addr & 0xff;
#if 0
    HAL_SPI_TransmitReceive(&hspi5, send, &data, 3, 100);
#else
    transmit_SPI5_Spi(send, 3);

    receive_SPI5_Spi(&data, 1);
#endif
    //transferSpi(send, &data, 3, 100);
    //setChipSelect(SPI_CS_E2_, APC_HIGH);
    HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_SET);
		set_SPI5_SpiDataMode(SPI_MODE0);
#else
    setSpiOption(SPI_OPTION_DEFAULT);
    setChipSelect(SPI_CS_E2_, APC_LOW);
    uint8_t send[3] = { 0 };
    send[0] = E2PROM_READ;
    send[1] = addr >> 8;
    send[2] = addr & 0xff;
    transmitSpi(send, 3);
    receiveSpi(&data, 1);
    //transferSpi(send, &data, 3, 100);
    setChipSelect(SPI_CS_E2_, APC_HIGH);
#endif
	return true;
}

bool setE2promData(uint16_t addr, uint8_t data)
{
#ifdef __REV_MTECH__
  //setSpiOption(SPI_OPTION_DEFAULT);
  //setChipSelect(SPI_CS_E2_, APC_LOW);
	set_SPI5_SpiDataMode(SPI_MODE0);
  HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_RESET);
  uint8_t send[4] = { 0 };
  send[0] = E2PROM_WREN;
  transmit_SPI5_Spi(send, 1);
  //HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_SET);

  //HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_RESET);
  send[0] = E2PROM_WRITE;
  send[1] = addr >> 8;
  send[2] = addr & 0xff;
  send[3] = data;
  transmit_SPI5_Spi(send, 4);
  //transferSpi(send, NULL, 4, 100);
  //setChipSelect(SPI_CS_E2_, APC_HIGH);
  HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_SET);
	set_SPI5_SpiDataMode(SPI_MODE0);
#else
    setSpiOption(SPI_OPTION_DEFAULT);
    setChipSelect(SPI_CS_E2_, APC_LOW);
    uint8_t send[4] = { 0 };
    send[0] = E2PROM_WREN;
    transmitSpi(send, 1);

    send[0] = E2PROM_WRITE;
    send[1] = addr >> 8;
    send[2] = addr & 0xff;
    send[3] = data;
    transmitSpi(send, 4);
    //transferSpi(send, NULL, 4, 100);
    setChipSelect(SPI_CS_E2_, APC_HIGH);
#endif
	return true;
}
