/*
 * APC_Display.c
 *
 * - HCMS-2903
 *
 *  Created on: 2023. 10. 23.
 *      Author: Yongseok
 */

#include <APC_Display.h>
#include <APC_AccessMode.h>
#include <APC_ControlMode.h>
#include <APC_Utils.h>
#include <APC_Font.h>

#include <APC_ChipSelect.h>
#include <APC_Spi.h>
#ifdef _USE_THREAD
    #include "FreeRTOS.h"
    #include "task.h"
#endif

#include <APC_Define.h>

static char _display_char[DISPLAY_SIZE] = { 0 };

static uint8_t _cur_mode = 0x00;
static int     _val_pos  = 0;


void setControlRegister(uint8_t data);


/**
 * init Display
 */
void initDisplay(void)
{
    setControlRegister(0x7F);
    setControlRegister(0x80);
}

char* getDisplayData(void)
{
//    for(int i=0; i < DISPLAY_SIZE; i++) {
//        data[i] = _display_char[i];
//    }
    return _display_char;
}

bool setDisplayData(char* data)
{
#ifdef _USE_THREAD
    taskENTER_CRITICAL();
#endif

    bool status = false;
    uint8_t dotRegister[DISPLAY_SIZE*5] = { 0 };

    for(int i=0; i<DISPLAY_SIZE; i++)
    {
        _display_char[i] = *(data+i);
        for (int j=0; j < 5; j++) {
            dotRegister[(i * 5) + j] = Font5x7[((*(data+i) - 0x20) * 5) + j];
        }
    }

#ifdef __REV_MTECH__
    HAL_GPIO_WritePin(DSP_RS_GPIO_Port, DSP_RS_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(MPU_SPI5_DSPCS__GPIO_Port, MPU_SPI5_DSPCS__Pin, GPIO_PIN_RESET);
    for (int i=0; i < DISPLAY_SIZE*5; i++)
    {
        uint8_t rxByte = 0xFF;
        status = transferDspSpi(&dotRegister[i], &rxByte, 1, 100);
    }
    HAL_GPIO_WritePin(MPU_SPI5_DSPCS__GPIO_Port, MPU_SPI5_DSPCS__Pin, GPIO_PIN_SET);
#else
    setSpiOption(SPI_OPTION_EXTEND);

    //HAL_GPIO_WritePin(SPI_I_GPIO_Port, SPI_I_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MPU_SPI1_MISO_GPIO_Port, MPU_SPI1_MISO_Pin, GPIO_PIN_RESET);
    setChipSelect(SPI_CS_DP_, APC_LOW);
    for (int i=0; i < DISPLAY_SIZE*5; i++)
    {
        uint8_t rxByte = 0xFF;
        status = transferSpi(&dotRegister[i], &rxByte, 1, 100);
    }
    setChipSelect(SPI_CS_DP_, APC_HIGH);
#endif
#ifdef _USE_THREAD
    taskEXIT_CRITICAL();
#endif

	return status;
}

void setControlRegister(uint8_t data)
{
#ifdef _USE_THREAD
    taskENTER_CRITICAL();
#endif

#ifdef __REV_MTECH__
    HAL_GPIO_WritePin(DSP_RS_GPIO_Port, DSP_RS_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(MPU_SPI5_DSPCS__GPIO_Port, MPU_SPI5_DSPCS__Pin, GPIO_PIN_RESET);
    uint8_t rxByte = 0xFF;
    transferDspSpi(&data, &rxByte, 1, 100);
    HAL_GPIO_WritePin(MPU_SPI5_DSPCS__GPIO_Port, MPU_SPI5_DSPCS__Pin, GPIO_PIN_SET);
#else
    setSpiOption(SPI_OPTION_EXTEND);

    //HAL_GPIO_WritePin(SPI_I_GPIO_Port, SPI_I_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MPU_SPI1_MISO_GPIO_Port, MPU_SPI1_MISO_Pin, GPIO_PIN_SET);

    setChipSelect(SPI_CS_DP_, APC_LOW);
    uint8_t rxByte = 0xFF;
    transferSpi(&data, &rxByte, 1, 100);
    setChipSelect(SPI_CS_DP_, APC_HIGH);
    //APC_Delay(10);
#endif

#ifdef _USE_THREAD
    taskEXIT_CRITICAL();
#endif
}

void setLed(bool option)
{
    setChipSelect(LED_CS_, option);
}

void setDisplayCodeNumber(uint8_t mode, int number)
{
    if (_cur_mode == mode && _val_pos == number)    return;

    char code = 'V';
    switch (mode) {
        case ControlMode_POSITION:  code = 'V'; break;
        case ControlMode_CLOSED:    code = 'C'; break;
        case ControlMode_OPEN:      code = 'O'; break;
        case ControlMode_PRESSURE:  code = 'P'; break;
        case ControlMode_HOLD:      code = 'H'; break;
        case ControlMode_LEARN:     code = 'L'; break;
        case ControlMode_INTERLOCK_OPEN:    code = 'I'; break;
        case ControlMode_INTERLOCK_CLOSED:  code = 'I'; break;
        //case ControlMode_MaintenanceOpen:     code = 'L'; break;
        //case ControlMode_MaintenanceClose:     code = 'L'; break;
        case ControlMode_PowerFailure:      code = 'F'; break;
        case ControlMode_SafetyMode:        code = 'D'; break;
        case ControlMode_FatalError:        code = 'E'; break;
        //case ControlMode_Service:   code = 'S'; break;
        default:    break;
    }

    if (_cur_mode != mode)  _cur_mode = mode;
    if (_val_pos != number) _val_pos = number;

    uint8_t disp[4] = { 0x00 };
    int offset = 0;
    offset = sprintf((char*) (disp + offset), "%c",  code);
    offset = sprintf((char*) (disp + offset), "%3d", number);
    //snprintf((char*) disp, 5, "%c%3d", code, number);
    setDisplayData((char*) disp);
}
