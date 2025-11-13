/*
 * APC_ChipSelect.c
 *
 *  Created on: 2023. 11. 27.
 *      Author: Yongseok
 *
 *
 */
#include <APC_ChipSelect.h>
#include <APC_Utils.h>

#include <APC_Define.h>

#ifdef __REV_MTECH__
#else
extern SRAM_HandleTypeDef hsram4;
#endif
static  uint8_t cs_data = 0x00;


/*
 * When initializing, all CS pins are in default condition.
 */
void initChipSelect(void)
{
    setChipSelect(0, APC_LOW);
}

void setChipSelect(uint8_t order, bool on_off)
{
#ifdef __REV_MTECH__
	switch (order)
	{
		case 0:     // reset
			break;
		case LED_CS_:       // 1
			if (on_off == APC_HIGH)		HAL_GPIO_WritePin(GPIOD, LD2_LED_Pin, GPIO_PIN_RESET);
			else											HAL_GPIO_WritePin(GPIOD, LD2_LED_Pin, GPIO_PIN_SET);
			break;
	}
#else
    uint32_t cs_addr = (0 << 19) | (0 << 18) | (0 << 17);

    switch (order)
    {
        case 0:     // reset
            cs_data = 0xBF; // 0b10111111;
            break;
        case LED_CS_:       // 1
        case SENSEMODE:     // 2
        case SPI_CS_IF_:    // 3
        case SPI_CS_DP_:    // 4
        case SPI_CS_MD_:    // 5
        case SPI_CS_E2_:    // 6
        case SPI_CS_GL_:    // 7
        case SPI_CS_PS_:    // 8
            if (order >= SPI_CS_IF_ && order <= SPI_CS_PS_)
                cs_data |= 0x3F;    // 0b00111111;
            if (on_off == APC_HIGH)
                cs_data |=  (1 << (8 - order));
            else
                cs_data &= ~(1 << (8 - order));
            break;
        case CS_IF1_:   // 17
            if (on_off == APC_HIGH)
                cs_addr = (1 << 19) | (1 << 18) | (1 << 17);
            else
                cs_addr = (0 << 19) | (0 << 18) | (1 << 17);
            break;
        case CS_IF2_:   // 18
            if (on_off == APC_HIGH)
                cs_addr = (1 << 19) | (1 << 18) | (1 << 17);
            else
                cs_addr = (0 << 19) | (1 << 18) | (0 << 17);
            break;
        case CS_IF3_:   // 19
            if (on_off == APC_HIGH)
                cs_addr = (1 << 19) | (1 << 18) | (1 << 17);
            else
                cs_addr = (0 << 19) | (1 << 18) | (1 << 17);
            break;
        case CS_PS_:   // 20
            if (on_off == APC_HIGH)
                cs_addr = (1 << 19) | (1 << 18) | (1 << 17);
            else
                cs_addr = (1 << 19) | (0 << 18) | (0 << 17);
            break;
        default:
            break;
    }

    HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + cs_addr), &cs_data, 1);
    APC_Delay(1);
#endif
}

