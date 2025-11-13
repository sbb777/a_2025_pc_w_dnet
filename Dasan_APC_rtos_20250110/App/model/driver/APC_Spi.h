/*
 * API_Spi.h
 *
 *  Created on: 2023. 10. 30.
 *      Author: Yongseok
 */

#ifndef _APC_SPI_H_
#define _APC_SPI_H_

#include "APC_Define.h"

#include "spi.h"

#define SPI_MODE0           0
#define SPI_MODE1           1
#define SPI_MODE2           2
#define SPI_MODE3           3

#define SPI_OPTION_DEFAULT  0
#define SPI_OPTION_EXTEND   1


void initSpi(void);
void setSpiOption(uint8_t option);
uint8_t getSpiOption(void);

void setSpiDataMode(uint8_t dataMode);
void setSpiBitWidth(uint8_t bit_width);

uint8_t receiveSpi(uint8_t *pData, uint8_t len);
uint8_t transmitSpi(uint8_t *pData, uint8_t len);

uint8_t receive_SPI5_Spi(uint8_t *pData, uint8_t len);
uint8_t transmit_SPI5_Spi(uint8_t *pData, uint8_t len);

uint8_t  transfer8Spi(uint8_t data);
uint16_t transfer16Spi(uint16_t data);

bool transferSpi(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t length, uint32_t timeout);


#endif /* _APC_SPI_H_ */
