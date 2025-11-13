/*
 * APC_DAC7612.c
 *
 *  Created on: Mar 24, 2024
 *      Author: Yongseok
 */
#include "APC_DAC7612.h"

#include "APC_Utils.h"
#include "APC_Timer.h"
#include "APC_SRAM.h"

#include <APC_Define.h>

#define LOADDACS_LOW    0b00000000
#define LOADDACS_HIGH   0b00000100

#define LOADDACS_PIN		0b00000100
#define LOAD_CLK_PIN		0b00000010
#define LOAD_SDI_PIN		0b00000001

static bool _dac7612_intialized = false;

extern SRAM_HandleTypeDef hsram4;

void initDAC7612(void)
{
    _dac7612_intialized = true;
}

void getDAC7612Config(uint8_t *pData, uint8_t len)
{

}

bool setDAC7612Config(uint8_t *pData, uint8_t len)
{
    return true;
}

/***********************************************
      (MSB)                                (LSB)
 A1 A0 D11 D10 D9 D8 D7 D6 D5 D4 D3  D2  D1  D0
************************************************
*/
void setDAC7612Value(uint32_t addr, uint16_t value)
{
	HAL_StatusTypeDef HAL_Status;
	// DACS_LOW
	uint8_t data    = LOADDACS_LOW;
#ifdef __REV_MTECH__
	PORTC_LB_is_OutPut();
	APC_Delay_us(1);
	HAL_GPIO_WritePin(GPIOC, LOADDACS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, LOAD_CLK_PIN, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(GPIOC, LOAD_SDI_PIN, GPIO_PIN_RESET);
	HAL_Status = HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + ADDR_CS_IF1_), &data, 1) == HAL_OK ? true : false;
	//PORTC_LB_is_InPut();
#else
	writeSRAM4(addr, data);
#endif
	APC_Delay_us(3);
	// 25.01.09
	APC_Delay_us(3);

	/*************************************************
	 A1 A0 CLK CS_ LOADDACS_      DAC         DAC
														Register A   Register B
	**************************************************
	 H  L   X   H     L       Load Data    No change
	 H  H   X   H     L       No change    Load Data
	**************************************************/
	uint8_t cur_val = 0x00;
	for (int i=13; i >= 0; i--)
	{
		//cur_val = (uint8_t) ((value >> i) & 0x01);      // 0x01
		//writeSRAM4(addr, cur_val);
		//APC_Delay_us(1);
		//data = cur_val | 0x02;
		//writeSRAM4(addr, data);
		//APC_Delay_us(1);

		// CLK_DOWN
#ifdef __REV_MTECH__
		//PORTC_LB_is_OutPut();
		HAL_GPIO_WritePin(GPIOC, LOAD_CLK_PIN, GPIO_PIN_RESET);
		//if(cur_val & 1)	HAL_GPIO_WritePin(GPIOC, LOAD_SDI_PIN, GPIO_PIN_SET);
		//else						HAL_GPIO_WritePin(GPIOC, LOAD_SDI_PIN, GPIO_PIN_RESET);
		HAL_Status = HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + ADDR_CS_IF1_), &data, 1) == HAL_OK ? true : false;
		//PORTC_LB_is_InPut();
#else
		data = (uint8_t) (LOADDACS_LOW | (0x00 << 1) | cur_val);   // CLK (down)
		writeSRAM4(addr, data);
#endif
		APC_Delay_us(3);
		// 25.01.09
		APC_Delay_us(3);

		// SDI
		cur_val = (uint8_t) ((value >> i) & 0x01);
#ifdef __REV_MTECH__
		//PORTC_LB_is_OutPut();
		//HAL_GPIO_WritePin(GPIOC, LOAD_CLK_PIN, GPIO_PIN_RESET);
		if(cur_val & 1)	HAL_GPIO_WritePin(GPIOC, LOAD_SDI_PIN, GPIO_PIN_SET);
		else						HAL_GPIO_WritePin(GPIOC, LOAD_SDI_PIN, GPIO_PIN_RESET);
		HAL_Status = HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + ADDR_CS_IF1_), &data, 1) == HAL_OK ? true : false;
		//PORTC_LB_is_InPut();
#else
		data = (uint8_t) (LOADDACS_LOW | (0x00 << 1) | cur_val);   // SDI
		writeSRAM4(addr, data);
#endif
		APC_Delay_us(3);
		// 25.01.09
		APC_Delay_us(3);

		// CLK_UP
#ifdef __REV_MTECH__
		//PORTC_LB_is_OutPut();
		HAL_GPIO_WritePin(GPIOC, LOAD_CLK_PIN, GPIO_PIN_SET);
		//if(cur_val & 1)	HAL_GPIO_WritePin(GPIOC, LOAD_SDI_PIN, GPIO_PIN_SET);
		//else						HAL_GPIO_WritePin(GPIOC, LOAD_SDI_PIN, GPIO_PIN_RESET);
		HAL_Status = HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + ADDR_CS_IF1_), &data, 1) == HAL_OK ? true : false;
		//PORTC_LB_is_InPut();
#else
		data = (uint8_t) (LOADDACS_LOW | (0x01 << 1) | cur_val);   // CLK (up)
		writeSRAM4(addr, data);
#endif
		APC_Delay_us(3);
		// 25.01.09
		APC_Delay_us(3);
	}

	// DACS_HIGH
	data = LOADDACS_HIGH;       // 0x06
#ifdef __REV_MTECH__
	//PORTC_LB_is_OutPut();
	HAL_GPIO_WritePin(GPIOC, LOADDACS_PIN, GPIO_PIN_RESET);
	HAL_Status = HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + ADDR_CS_IF1_), &data, 1) == HAL_OK ? true : false;
	PORTC_LB_is_InPut();
#else
	writeSRAM4(addr, data);
#endif
}
