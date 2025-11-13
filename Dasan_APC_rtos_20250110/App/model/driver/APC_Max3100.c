/*
 * APC_Max3100.c
 *
 * MAX3100 is used for Remote RS232 Interface
 *
 *  Created on: 2023. 11. 30.
 *      Author: Yongseok
 */

#include <APC_Max3100.h>

#include <math.h>
#include "APC_Spi.h"
#include "APC_ChipSelect.h"

// Commands.
#define MAX3100_CMD_WRITE_CONF      0b1100000000000000
#define MAX3100_CMD_READ_CONF       0b0100000000000000
#define MAX3100_CMD_WRITE_DATA      0b1000000000000000
#define MAX3100_CMD_READ_DATA       0b0000000000000000

// Configuration.
#define MAX3100_CONF_R              0b1000000000000000
#define MAX3100_CONF_T              0b0100000000000000
#define MAX3100_CONF_RM             0b0000010000000000
#define MAX3100_CONF_L              0b0000000000010000
#define MAX3100_CONF_ST             0b0000000001000000

void     setMax3100(uint16_t data);
uint16_t getMax3100(uint16_t data);
uint8_t  calculateBaudrateConfig(uint32_t rate);

bool initMax3100(uint32_t b_rate, uint8_t d_length, uint8_t stop_bit) {
    uint16_t config = 0x0000;
    config |= calculateBaudrateConfig(b_rate);
    config |= (d_length == 0x00 ? 0x0000 : MAX3100_CONF_L);
    config |= (stop_bit == 0x00 ? 0x0000 : MAX3100_CONF_ST);
    setMax3100Config(config);

    //setMax3100Baudrate(baud_rate);
    //setMax3100Config(MAX3100_CONF_R | MAX3100_CONF_RM);
    return true;
}

uint8_t calculateBaudrateConfig(uint32_t rate)
{
    uint8_t b0b3 = 0x0B;
    switch (rate)
    {
//        case    600:    b0b3 = 0x0F; break;
//        case   1200:    b0b3 = 0x0E; break;
//        case   2400:    b0b3 = 0x0D; break;
//        case   4800:    b0b3 = 0x0C; break;
        case   9600:    b0b3 = 0x0B; break; //4
        case  19200:    b0b3 = 0x0A; break; //5
        case  38400:    b0b3 = 0x09; break; //6
        case  57600:    b0b3 = 0x02; break; //7
        case 115200:    b0b3 = 0x01; break; //8
        case 230400:    b0b3 = 0x00; break; //9
        default:        b0b3 = 0x0B; break;
    }
    return b0b3;
}

void setMax3100Baudrate(uint32_t rate) {
    uint8_t config = calculateBaudrateConfig(rate);
    setMax3100Config(config);
}

uint32_t getMax3100Baudrate(void) {
    uint16_t div_ratio = 0x01;
    uint16_t result = getMax3100Config();
    uint8_t b0b3 = result & 0x0f;

    if (b0b3 < 0x08)
        div_ratio = pow(2, b0b3);
    else
        div_ratio = 3 * pow(2, (b0b3 & 0x07));
    return 230400 / div_ratio;  // fOSC = 3.6864MHz
}


int readMax3100Data(void)
{
    int character = -1;
    uint16_t data = getMax3100Data();
    if (data & MAX3100_CONF_R)
        character = data & 0xff;
    return character;
}

bool availableMax3100Transmit(void)
{
    uint16_t config = getMax3100Config();
    //uint16_t data = getMax3100Data();
    return (config & MAX3100_CONF_T) > 0 ? true : false;
}

void writeMax3100Data(uint8_t data)
{
    while ( !availableMax3100Transmit());
    setMax3100Data(data);
}

bool availableMax3100Receive(void)
{
    uint16_t config = getMax3100Config();
    //uint16_t data = getMax3100Data();
    return (config & MAX3100_CONF_R) > 0 ? true : false;
}


void setMax3100Config(uint16_t config)
{
    setMax3100(MAX3100_CMD_WRITE_CONF | config); 	// Write Configuration (D15, D14 = 1, 1)
}

uint16_t getMax3100Config(void)
{
    return getMax3100(MAX3100_CMD_READ_CONF); 		// Read Configuration (D15, D14 = 0, 1)
}

void setMax3100Data(uint16_t data)
{
    setMax3100(MAX3100_CMD_WRITE_DATA | data); 		// Write Data (D15, D14 = 1, 0)
}

uint16_t getMax3100Data(void)
{
    return getMax3100(MAX3100_CMD_READ_DATA); 		// Read Data (D15, D14 = 0, 0)
}

void setMax3100(uint16_t data)
{
    setSpiOption(SPI_OPTION_DEFAULT);
    setChipSelect(SPI_CS_IF_, APC_LOW);
    //uint16_t result = transfer16Spi( data);
    transfer16Spi( data);
    setChipSelect(SPI_CS_IF_, APC_HIGH);
}

uint16_t getMax3100(uint16_t data)
{
    setSpiOption(SPI_OPTION_DEFAULT);
    setChipSelect(SPI_CS_IF_, APC_LOW);
    uint16_t result = transfer16Spi( data);
    setChipSelect(SPI_CS_IF_, APC_HIGH);
    return result;
}
