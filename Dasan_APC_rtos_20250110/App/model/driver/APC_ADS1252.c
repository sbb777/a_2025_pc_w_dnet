/*
 * APC_ADS1252.c
 *
 *  Created on: 2023. 11. 16.
 *      Author: Yongseok
 */
#include <APC_ADS1252.h>
#include <APC_Utils.h>

#include "APC_Timer.h"

static bool _ads_initialized = false;

void initADS1252(void)
{
    _ads_initialized = true;
}

void getADS1252Config(uint8_t *pData, uint8_t len)
{

}

bool setADS1252Config(uint8_t *pData, uint8_t len)
{
    return true;
}

/**
 * ADS1252 (24bit)
 * - 19bit effective
 */
int getADS1252(uint8_t ch)
{
    uint8_t data[3] = { 0x00 };
    GPIO_TypeDef* data_port;
    uint16_t data_pin;
    int nValue, nSensor;

#ifdef __REV_MTECH__
    if (ch == _DEF_CH1) {
        data_port = MPU_DATA_AD1_GPIO_Port;
        data_pin  = MPU_DATA_AD1_Pin;
    } else {
        data_port = MPU_DATA_AD2_GPIO_Port;
        data_pin  = MPU_DATA_AD2_Pin;
    }
#else
    if (ch == _DEF_CH1) {
        data_port = DATA_AD1__GPIO_Port;
        data_pin  = DATA_AD1__Pin;
    } else {
        data_port = DATA_AD2__GPIO_Port;
        data_pin  = DATA_AD2__Pin;
    }
#endif
    //HAL_GPIO_WritePin(CLK_AD_GPIO_Port, CLK_AD_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MPU_CLK_AD_GPIO_Port, MPU_CLK_AD_Pin, GPIO_PIN_RESET);

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(data_port, data_pin);
    if (pin_state == GPIO_PIN_RESET) {
        while (HAL_GPIO_ReadPin(data_port, data_pin) == GPIO_PIN_RESET);
        APC_Delay_us(36); // t4 + t2 + t3
    } else {
        while (HAL_GPIO_ReadPin(data_port, data_pin) == GPIO_PIN_SET);
        APC_Delay_us(12);  // t2 + t3
    }
    APC_Delay_us(3);

    for (int j=0; j < 3; j++)
    {
        for (int i=0; i < 8; i++)
        {
            //HAL_GPIO_WritePin(CLK_AD_GPIO_Port, CLK_AD_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(MPU_CLK_AD_GPIO_Port, MPU_CLK_AD_Pin, GPIO_PIN_SET);
            APC_Delay_us(3);

            if (HAL_GPIO_ReadPin(data_port, data_pin) == GPIO_PIN_SET) {
                data[j] |= (1 << (7-i));
            }

            //HAL_GPIO_WritePin(CLK_AD_GPIO_Port, CLK_AD_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(MPU_CLK_AD_GPIO_Port, MPU_CLK_AD_Pin, GPIO_PIN_RESET);
            APC_Delay_us(3);
        }
    }

    if ((data[0] & 0x80) == 0x80)
    {
        data[0] = ~data[0];
        data[1] = ~data[1];
        data[2] = ~data[2];
        nValue = (data[0] << 16) | (data[1] <<8) | data[2];
        nSensor = 5230000-nValue;
    }
    else{
        nValue = (data[0] << 16) | (data[1] <<8) | data[2];
        nSensor = 5230000+nValue;
    }

    return nSensor;
}
