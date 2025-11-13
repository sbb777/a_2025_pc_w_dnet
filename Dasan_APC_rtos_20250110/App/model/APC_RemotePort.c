/*
 * APC_RemotePort.c
 *
 *  Created on: Feb 23, 2024
 *      Author: Yongseok
 */

#include "APC_RemotePort.h"

#include <math.h>
#include "APC_Utils.h"
#include "APC_SRAM.h"
#include "APC_DAC7612.h"
#include <APC_Max3100.h>
#include <APC_Flash.h>

#ifdef __REV_MTECH__
extern UART_HandleTypeDef huart4;
extern SRAM_HandleTypeDef hsram4;
#endif

/*********************************************************************************/
// Board(128) + Valve(64) + Sensor(64) + Controller(64)
#define ADDR_REMOTE_PORT_START       						(_ADDR_MO_DATA + 128 + 64 + 64 + 64)

#define ADDR_RemoteRS232_Baudrate               (ADDR_REMOTE_PORT_START +  0)
#define ADDR_RemoteRS232_ParityBit              (ADDR_REMOTE_PORT_START +  1)
#define ADDR_RemoteRS232_DataLength             (ADDR_REMOTE_PORT_START +  2)
#define ADDR_RemoteRS232_StopBits               (ADDR_REMOTE_PORT_START +  3)

#define ADDR_RemoteDigitalInputOpenValve        (ADDR_REMOTE_PORT_START +  4)
#define ADDR_RemoteDigitalInputCloseValve       (ADDR_REMOTE_PORT_START +  5)
#define ADDR_RemoteDigitalOutputOpenValve       (ADDR_REMOTE_PORT_START +  6)
#define ADDR_RemoteDigitalOutputCloseValve      (ADDR_REMOTE_PORT_START +  7)
#define ADDR_RemoteCommandSet                   (ADDR_REMOTE_PORT_START +  8)
/*********************************************************************************/

// Analog Output
#ifdef __REV_MTECH__
#define AOUT_PRESSURE   0b00000010
#define AOUT_POSITION   0b00000011
#else
#define AOUT_PRESSURE   0b00000011      // 0b00000010
#define AOUT_POSITION   0b00000010      // 0b00000011
#endif

// Read
#define PW_MD_ON        0b00010000
#define CLOSE_INPUT     0b01000000
#define OPEN_INPUT      0b10000000

// Write
#define SPI_CS_E2       0b00100000
#define RELAY_CLOSE     0b01000000
#define RELAY_OPEN      0b10000000

/*********************************************************************************/

static bool _remote_intialized = false;

EN_Baudrate             MO_RemoteRS232_Baudrate;
EN_ParityBit            MO_RemoteRS232_ParityBit;
EN_DataLength           MO_RemoteRS232_DataLength;
EN_StopBits             MO_RemoteRS232_StopBits;
EN_CommandSet           MO_RemoteRS232_CommandSet;
//uint32_t                MO_RemoteRS232_InterfaceRs232Config;

EN_DigitalInputOpenValve    MO_RemoteDIOpenValve    = 0;
EN_DigitalInputCloseValve   MO_RemoteDICloseValve   = 0;
EN_DigitalInputOpenValve    MO_RemoteDOOpenValve   = 0;
EN_DigitalInputCloseValve   MO_RemoteDOCloseValve  = 0;

/* ----------------------------------------------------------------------
    Description : MO related with "REMOTE DNET Interface"
*/
uint32_t     MO_DnetSystemInfo;
uint32_t     MO_DnetFwInfo;
uint32_t     MO_DnetSerialNo;
uint32_t     MO_DnetMacId;
uint32_t     MO_DnetBaudrate;
uint32_t     MO_DnetDatatype;
uint32_t     MO_DnetInputAssembly;
uint32_t     MO_DnetStatus;
uint32_t     MO_DnetExceptionStatus;

bool         m_bPower_md;
bool         m_bIsOpen;
bool         m_bIsClose;

void initRemotePort(void)
{
		uint32_t baud_rate;

    MO_RemoteRS232_Baudrate     = readByteFromFlash(ADDR_RemoteRS232_Baudrate);
    MO_RemoteRS232_ParityBit    = readByteFromFlash(ADDR_RemoteRS232_ParityBit);
    MO_RemoteRS232_DataLength   = readByteFromFlash(ADDR_RemoteRS232_DataLength);
    MO_RemoteRS232_StopBits     = readByteFromFlash(ADDR_RemoteRS232_StopBits);
    MO_RemoteRS232_CommandSet   = readByteFromFlash(ADDR_RemoteCommandSet);

    MO_RemoteDIOpenValve   = readByteFromFlash(ADDR_RemoteDigitalInputOpenValve);
    MO_RemoteDICloseValve  = readByteFromFlash(ADDR_RemoteDigitalInputCloseValve);
    MO_RemoteDOOpenValve   = readByteFromFlash(ADDR_RemoteDigitalOutputOpenValve);
    MO_RemoteDOCloseValve  = readByteFromFlash(ADDR_RemoteDigitalOutputCloseValve);

    MO_RemoteRS232_Baudrate     = MO_RemoteRS232_Baudrate   == 0xff ? Baudrate_9600 : MO_RemoteRS232_Baudrate;
    MO_RemoteRS232_ParityBit    = MO_RemoteRS232_ParityBit  == 0xff ? ParityBit_no  : MO_RemoteRS232_ParityBit;
    MO_RemoteRS232_DataLength   = MO_RemoteRS232_DataLength == 0xff ? DataLength_8bit : MO_RemoteRS232_DataLength;
    MO_RemoteRS232_StopBits     = MO_RemoteRS232_StopBits   == 0xff ? StopBits_1    : MO_RemoteRS232_StopBits;
    MO_RemoteRS232_CommandSet   = MO_RemoteRS232_CommandSet == 0xff ? CommandSet_IC : MO_RemoteRS232_CommandSet;

    MO_RemoteDIOpenValve   = MO_RemoteDIOpenValve   == 0xff ? DigitalInputOpenValve_NotInverted : MO_RemoteDIOpenValve;
    MO_RemoteDICloseValve  = MO_RemoteDICloseValve  == 0xff ? DigitalInputCloseValve_NotInverted : MO_RemoteDICloseValve;
    MO_RemoteDOOpenValve   = MO_RemoteDOOpenValve   == 0xff ? DigitalInputOpenValve_NotInverted : MO_RemoteDOOpenValve;
    MO_RemoteDOCloseValve  = MO_RemoteDOCloseValve  == 0xff ? DigitalInputCloseValve_NotInverted : MO_RemoteDOCloseValve;

#ifdef __REV_MTECH__
    huart4.Instance = UART4;
    switch (MO_RemoteRS232_Baudrate)
    {
//      case    600:    MO_RemoteRS232_Baudrate = Baudrate_600;     break;
//      case   1200:    MO_RemoteRS232_Baudrate = Baudrate_1200;    break;
//      case   2400:    MO_RemoteRS232_Baudrate = Baudrate_2400;    break;
//      case   4800:    MO_RemoteRS232_Baudrate = Baudrate_4800;    break;
        case Baudrate_9600:		baud_rate = 9600;   break;
        case Baudrate_19200:	baud_rate = 19200;  break;
        case Baudrate_38400:  baud_rate = 38400;  break;
        case Baudrate_57600:  baud_rate = 57600;  break;
        case Baudrate_115200: baud_rate = 115200; break;
        case Baudrate_230400: baud_rate = 230400; break;
        default:        			baud_rate = 9600;  	break;
    }
    huart4.Init.BaudRate = baud_rate;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart4) != HAL_OK)
    {
      Error_Handler();
    }

#else
    initMax3100((MO_RemoteRS232_Baudrate <= Baudrate_38400 ? 600 : 450) * pow(2, MO_RemoteRS232_Baudrate),
            MO_RemoteRS232_DataLength == DataLength_8bit ? 0x00 : 0x01,
            MO_RemoteRS232_StopBits == StopBits_1 ? 0x00 : 0x01);
#endif

    initDAC7612();

    _remote_intialized = true;
}

bool syncRemotePort(uint32_t addr)
{
    bool result = true;
    if (addr == _ADDR_MO_DATA) {
        result &= writeByteToFlash(ADDR_RemoteRS232_Baudrate,   MO_RemoteRS232_Baudrate);
        result &= writeByteToFlash(ADDR_RemoteRS232_ParityBit,  MO_RemoteRS232_ParityBit);
        result &= writeByteToFlash(ADDR_RemoteRS232_DataLength, MO_RemoteRS232_DataLength);
        result &= writeByteToFlash(ADDR_RemoteRS232_StopBits,   MO_RemoteRS232_StopBits);
        result &= writeByteToFlash(ADDR_RemoteCommandSet,       MO_RemoteRS232_CommandSet);

        result &= writeByteToFlash(ADDR_RemoteDigitalInputOpenValve,   MO_RemoteDIOpenValve);
        result &= writeByteToFlash(ADDR_RemoteDigitalInputCloseValve,  MO_RemoteDICloseValve);
        result &= writeByteToFlash(ADDR_RemoteDigitalOutputOpenValve,  MO_RemoteDOOpenValve);
        result &= writeByteToFlash(ADDR_RemoteDigitalOutputCloseValve, MO_RemoteDOCloseValve);
    }
    return result;
}

void procRemotePort(uint8_t ch)
{
    if (APC_PORT_REMOTE) {
        readSRAM4(ADDR_CS_IF1_);
    }
}

void PORTC_LB_is_InPut()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : PC0 PC1 PC2 PC3
													 PC4 PC5 PC6 PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
													|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void PORTC_LB_is_OutPut()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : PC0 PC1 PC2 PC3
													 PC4 PC5 PC6 PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
													|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
 *
 */
void writeAOUT_PRESSURE(int pressure)   // H,L	-> A
{
    int i_val = DAC7612_SCALE * (pressure / 1000) / 10000;
    if(i_val < 0)       						i_val = 0;
    else if(i_val > DAC7612_SCALE)  i_val = DAC7612_SCALE;

    /////////////////////////////////////////////////////////////////////
    // Test DAC7612 Scope
    // i_val = pressure;
    /////////////////////////////////////////////////////////////////////
    //uint16_t val = (uint16_t) (0x0fff & i_val) | (2l << 12);
    uint16_t val = (uint16_t) (0x0fff & i_val) | (AOUT_PRESSURE << 12);
    setDAC7612Value(ADDR_CS_IF1_, val);
}

/**
 *
 */
void writeAOUT_POSITION(int position)  // H,H	-> B
{
    int i_val = 0;
    switch(getPositionCommRange()){
        case 0:
            i_val = DAC7612_SCALE * (position / 100) / 1000;
            break;
        case 1:
            i_val = DAC7612_SCALE * (position / 1000) / 1000;
            break;
        case 2:
            i_val = DAC7612_SCALE * (position / 10000) / 1000;
            break;
    }
    if(i_val < 0)       i_val = 0;
    else if(i_val > DAC7612_SCALE)  i_val = DAC7612_SCALE;

    /////////////////////////////////////////////////////////////////////
    // Test DAC7612 Scope
    // i_val = position;
    /////////////////////////////////////////////////////////////////////
    //uint16_t val = (uint16_t) (0x0fff & i_val) | (3l << 12);
    uint16_t val = (uint16_t) (0x0fff & i_val) | (AOUT_POSITION << 12);
    setDAC7612Value(ADDR_CS_IF1_, val);
}

void writeRELAY_CLOSE(bool option)
{
	HAL_StatusTypeDef HAL_Status;
	uint8_t data = option == APC_HIGH ? RELAY_CLOSE : 0x00;
	if (MO_RemoteDOCloseValve == DigitalInputCloseValve_Inverted)
		data = option == APC_HIGH ? 0x00 : RELAY_CLOSE;

#ifdef __REV_MTECH__
	PORTC_LB_is_OutPut();
	if (MO_RemoteDOCloseValve != DigitalInputCloseValve_Inverted)
	{
		if(option == APC_HIGH) 	HAL_GPIO_WritePin(GPIOC, RELAY_CLOSE, GPIO_PIN_SET);
		else 										HAL_GPIO_WritePin(GPIOC, RELAY_CLOSE, GPIO_PIN_RESET);
	}
	else
	{
		if(option == APC_HIGH) 	HAL_GPIO_WritePin(GPIOC, RELAY_CLOSE, GPIO_PIN_RESET);
		else 										HAL_GPIO_WritePin(GPIOC, RELAY_CLOSE, GPIO_PIN_SET);
	}
	HAL_Status = HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + ADDR_CS_IF1_), &data, 1) == HAL_OK ? true : false;
	PORTC_LB_is_InPut();
#else
	writeSRAM4(ADDR_CS_IF1_, data);
#endif
}

void writeRELAY_OPEN(bool option)
{
	HAL_StatusTypeDef HAL_Status;
	uint8_t data = option == APC_HIGH ? RELAY_OPEN : 0x00;
	if (MO_RemoteDOOpenValve == DigitalInputOpenValve_Inverted)
		data = option == APC_HIGH ? 0x00 : RELAY_OPEN;

#ifdef __REV_MTECH__
	PORTC_LB_is_OutPut();
	if (MO_RemoteDOOpenValve != DigitalInputOpenValve_Inverted)
	{
		if(option == APC_HIGH) 	HAL_GPIO_WritePin(GPIOC, RELAY_OPEN, GPIO_PIN_SET);
		else 										HAL_GPIO_WritePin(GPIOC, RELAY_OPEN, GPIO_PIN_RESET);
	}
	else
	{
		if(option == APC_HIGH) 	HAL_GPIO_WritePin(GPIOC, RELAY_OPEN, GPIO_PIN_RESET);
		else 										HAL_GPIO_WritePin(GPIOC, RELAY_OPEN, GPIO_PIN_SET);
	}
	HAL_Status = HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + ADDR_CS_IF1_), &data, 1) == HAL_OK ? true : false;
	PORTC_LB_is_InPut();
#else
	writeSRAM4(ADDR_CS_IF1_, data);
#endif
}

void writeSPI_CS_E2(bool option)
{
    writeSRAM4(ADDR_CS_IF1_, option == APC_HIGH ? SPI_CS_E2 : 0x00);
}

uint8_t readRemotePort(void)
{
    return readSRAM4(ADDR_CS_IF1_);
}

uint8_t readCLOSE_INPUT(uint8_t input)
{
    //uint8_t data = readSRAM4(ADDR_CS_IF1_);
    uint8_t data = (input & CLOSE_INPUT) > 0 ? 0x01 : 0x00;
    return MO_RemoteDICloseValve == DigitalInputCloseValve_Inverted ? (~data & 0x01) : data;
}

uint8_t readOPEN_INPUT(uint8_t input)
{
    //uint8_t data = readSRAM4(ADDR_CS_IF1_);
    uint8_t data = (input & OPEN_INPUT) > 0 ? 0x01 : 0x00;
    return MO_RemoteDIOpenValve == DigitalInputOpenValve_Inverted ? (~data & 0x01) : data;
}

uint8_t readPW_MD_ON(uint8_t input)
{
    //uint8_t data = readSRAM4(ADDR_CS_IF1_);
    return (input & PW_MD_ON) > 0 ? 0x01 : 0x00;
}

void CheckRemote(void)
{
    uint8_t data = readSRAM4(ADDR_CS_IF1_);

    m_bPower_md = (data & PW_MD_ON) > 0 ? true : false;
    m_bIsOpen   = (data & OPEN_INPUT) > 0 ? true : false;
    m_bIsClose  = (data & CLOSE_INPUT) > 0 ? true : false;
}

EN_DigitalInputOpenValve getRemoteDigitalInputOpenValve()
{
    return MO_RemoteDIOpenValve;
}

void setRemoteDigitalInputOpenValve(EN_DigitalInputOpenValve openValve)
{
    MO_RemoteDIOpenValve = openValve;
}

EN_DigitalInputCloseValve getRemoteDigitalInputCloseValve()
{
    return MO_RemoteDICloseValve;
}

void setRemoteDigitalInputCloseValve(EN_DigitalInputCloseValve closeValve)
{
    MO_RemoteDICloseValve = closeValve;
}

EN_DigitalInputOpenValve getRemoteDigitalOutputOpenValve()
{
    return MO_RemoteDOOpenValve;
}

void setRemoteDigitalOutputOpenValve(EN_DigitalInputOpenValve openValve)
{
    MO_RemoteDOOpenValve = openValve;
}

EN_DigitalInputCloseValve getRemoteDigitalOutputCloseValve()
{
    return MO_RemoteDOCloseValve;
}

void setRemoteDigitalOutputCloseValve(EN_DigitalInputCloseValve closeValve)
{
    MO_RemoteDOCloseValve = closeValve;
}

uint32_t availableRemoteRS232(uint32_t ch)
{
    uint32_t ret = 0;

    ret = availableMax3100Receive();

    return ret;
}
/*
uint32_t getRemoteRS232Config(void)
{
    return MO_RemoteRS232_InterfaceRs232Config;
}

void setRemoteRS232Config(char* data)
{
    MO_RemoteRS232_InterfaceRs232Config = (uint32_t) (*data);
}*/

EN_Baudrate getRemoteRS232Baudrate()
{
    uint32_t baud_rate = getMax3100Baudrate();
    switch (baud_rate)
    {
//        case    600:    MO_RemoteRS232_Baudrate = Baudrate_600;     break;
//        case   1200:    MO_RemoteRS232_Baudrate = Baudrate_1200;    break;
//        case   2400:    MO_RemoteRS232_Baudrate = Baudrate_2400;    break;
//        case   4800:    MO_RemoteRS232_Baudrate = Baudrate_4800;    break;
        case   9600:    MO_RemoteRS232_Baudrate = Baudrate_9600;    break;
        case  19200:    MO_RemoteRS232_Baudrate = Baudrate_19200;   break;
        case  38400:    MO_RemoteRS232_Baudrate = Baudrate_38400;   break;
        case  57600:    MO_RemoteRS232_Baudrate = Baudrate_57600;   break;
        case 115200:    MO_RemoteRS232_Baudrate = Baudrate_115200;  break;
        case 230400:    MO_RemoteRS232_Baudrate = Baudrate_230400;  break;
        default:        MO_RemoteRS232_Baudrate = Baudrate_9600;  break;
    }
    return MO_RemoteRS232_Baudrate;
}

void setRemoteRS232Baudrate(EN_Baudrate baudRate)
{
    MO_RemoteRS232_Baudrate = baudRate;

    uint32_t baud_rate = (baudRate <= Baudrate_38400 ? 600 : 450) * pow(2, baudRate);
    setMax3100Baudrate(baud_rate);
}

EN_ParityBit getRemoteRS232ParityBit()
{
    return MO_RemoteRS232_ParityBit;
}

void setRemoteRS232ParityBit(EN_ParityBit parityBit)
{
    MO_RemoteRS232_ParityBit = parityBit;
}

EN_DataLength getRemoteRS232DataLength()
{
    return MO_RemoteRS232_DataLength;
}
void setRemoteRS232DataLength(EN_DataLength length)
{
    MO_RemoteRS232_DataLength = length;
}

EN_StopBits getRemoteRS232StopBits()
{
    return MO_RemoteRS232_StopBits;
}
void setRemoteRS232StopBits(EN_StopBits stopBits)
{
    MO_RemoteRS232_StopBits = stopBits;
}

EN_CommandSet getRemoteRS232CommandSet()
{
    return MO_RemoteRS232_CommandSet;
}
void setRemoteRS232CommandSet(EN_CommandSet commandSet)
{
    MO_RemoteRS232_CommandSet = commandSet;
}

uint8_t readRemoteRS232Data(void)
{
#ifdef __REV_MTECH__
		uint8_t rData;
		HAL_UART_Receive(&huart4, &rData, 1, 100);
		return rData;
#else
    int r = readMax3100Data();
    return (uint8_t) (r < 0 ? 0x00 : r & 0xFF);
#endif
}

void writeRemoteRS232Data(uint8_t *p_data, uint32_t length)
{
#ifdef __REV_MTECH__
  uint32_t ret = 0;
  HAL_StatusTypeDef HAL_Status;

	while(__HAL_UART_GET_FLAG(&huart4, UART_FLAG_TXE) == RESET);
  HAL_Status = HAL_UART_Transmit(&huart4, p_data, length, 100);	// 50
  //hal_ret = HAL_UART_Transmit(&huart1, p_data, length, 10);
  if (HAL_Status == HAL_OK)
  {
      ret = length;
  }
#else
    for (int i = 0; i < length; i++)
        writeMax3100Data( *(p_data+i));
#endif
}

void printfRemoteRS232(const char *fmt, ...)
{
    va_list arg;
    char print_buf[256];

    va_start(arg, fmt);
    int len = vsnprintf(print_buf, 256, fmt, arg);
    va_end(arg);

    if (len > 0)
    {
        writeRemoteRS232Data((uint8_t *)print_buf, len);
    }
}

void refreshRemoteRS232()
{

}

