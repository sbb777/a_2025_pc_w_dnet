/*
 * APC_Power.c
 *
 *  Created on: Oct 24, 2023
 *      Author: Yongseok
 */

#include <APC_Power.h>
#include <APC_Utils.h>

#include <APC_Board.h>
#include <APC_Max1116.h>


static bool _power_intialized = false;
//static int _cur_volt;

bool initPower(void)
{
    if (getPFOEquip() == Dev_Equipped){
        setPFOStatus(Status_Enabled);
    }

    _power_intialized = true;
    return _power_intialized;
}

int getPower(uint8_t ch)
{
    if (ch == _DEF_CH1) {
    }
	return _cur_volt;
}

bool setPower(uint8_t status)
{
    if (getPFOEquip() != Dev_Equipped)
        return false;

    if (status == Status_Disabled) {
        HAL_GPIO_WritePin(PFO_ON_GPIO_Port, PFO_ON_Pin, GPIO_PIN_RESET);
    }
    else {
        HAL_GPIO_WritePin(PFO_ON_GPIO_Port, PFO_ON_Pin, GPIO_PIN_SET);
    }
	return true;
}

/**
 * input: ~ 40Volt (MC34063A)
 *
 * The full-scale analog input range is determined by the internal reference of +4.096V.
 */
void refreshPower()
{
    uint8_t data = 0x00;
    data = getMax1116Value();
//    _cur_volt = (int)(data/* & 0xFF*/) * 16 * 10;        // 16 = (4096 / 255)
    _cur_volt = (int)data * 98;       // 30000/255 = 118,    25000)/255 = 98;
}
