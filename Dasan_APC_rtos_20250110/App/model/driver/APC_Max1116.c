/*
 * APC_Max1116.c
 *
 *  Created on: Apr 9, 2024
 *      Author: Yongseok
 */

#include <APC_Max1116.h>
#include <APC_Utils.h>

#include <APC_ChipSelect.h>
#include <APC_Spi.h>

static bool _max1116_initialized = false;

void initMax1116(void)
{
    _max1116_initialized = true;
}

void getMax1116Config(uint8_t *pData, uint8_t len)
{

}

bool setMax1116Config(uint8_t *pData, uint8_t len)
{
    return true;
}

/**
 *
 * - 8bit(MAX1116)
 * in CONNECTER spi1
 */
uint8_t getMax1116Value(void)
{
  uint8_t data;
#ifdef __REV_MTECH__
  setSpiOption(SPI_OPTION_DEFAULT);
  HAL_GPIO_WritePin(MPU_SPI1_PMONCS__GPIO_Port, MPU_SPI1_PMONCS__Pin, GPIO_PIN_RESET);
  //data = transfer8Spi(0x00);
  transferSpi(NULL, &data, 1, 10);
  HAL_GPIO_WritePin(MPU_SPI1_PMONCS__GPIO_Port, MPU_SPI1_PMONCS__Pin, GPIO_PIN_SET);

  return data;
#else
	setSpiOption(SPI_OPTION_DEFAULT);
	setChipSelect(SPI_CS_PS_, APC_LOW);
	//data = transfer8Spi(0x00);
	transferSpi(NULL, &data, 1, 10);
	setChipSelect(SPI_CS_PS_, APC_HIGH);

	return data;
#endif
}

/**
 *
 * - 8bit(MAX1116)
 **********************************************************
 * VDRUCK
 **********************************************************
 * #define MPU_SPI5_MDVDCS__Pin GPIO_PIN_12
 * #define MPU_SPI5_MDVDCS__GPIO_Port GPIOB
 * in MST spi5
 */
uint8_t getMax1116_VDRUCK_Value(void)
{
  uint8_t data;
#ifdef __REV_MTECH__
  setSpiOption(SPI_OPTION_DEFAULT);
  HAL_GPIO_WritePin(MPU_SPI5_MDVDCS__GPIO_Port, MPU_SPI5_MDVDCS__Pin, GPIO_PIN_RESET);
  //data = transfer8Spi(0x00);
  transfer_VDRUCK_Spi(NULL, &data, 1, 10);
  HAL_GPIO_WritePin(MPU_SPI5_MDVDCS__GPIO_Port, MPU_SPI5_MDVDCS__Pin, GPIO_PIN_SET);

  return data;
#else
	setSpiOption(SPI_OPTION_DEFAULT);
	setChipSelect(SPI_CS_PS_, APC_LOW);
	//data = transfer8Spi(0x00);
	transferSpi(NULL, &data, 1, 10);
	setChipSelect(SPI_CS_PS_, APC_HIGH);

	return data;
#endif
}
