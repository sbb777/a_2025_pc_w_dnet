/*
 * APC_CompAir.c
 *
 *  Created on: Oct 24, 2023
 *      Author: Yongseok
 */

#include <APC_CompAir.h>
#include <APC_Utils.h>

#include <APC_AccessMode.h>
#include <APC_ControlMode.h>
#include "APC_Error.h"
#include "APC_Warning.h"
#include "APC_Display.h"
#include "APC_Valve.h"
#include "APC_TMC.h"
#include <APC_Spi.h>
#include <APC_ChipSelect.h>

static bool    _air_initialized = false;
//static uint8_t _comp_air_val;
static uint8_t _air_status = 2;     // 0(OK), 1(AIRf), 2(AIRx), 3(Closed)


bool initCompAir(void)
{
    _air_initialized = true;
    _comp_air_val = 0;
    // SPI 채널 확인
    return true;
}

/**
 * SPI_CS_MD/, SPI_CLK, SPI_I 통신을 통해 VDRUCK 수신
 * - option: true (synchronize 과정), false (other case)
 * - return value : 0(OK), 1(AIRf), 2(AIRx)
 */
uint8_t checkCompAirStatus(uint8_t ch, bool option)
{
    return 0;

    uint8_t value = 0, status = 0;
    if (getValveType() == ValveType_Butterfly){
        _air_status = 0;
        return _air_status;
    }
    if (getExtIsolationValveFunc() == ExtIsolationValveFunc_No){
        _air_status = 0;
        return _air_status;
    }

    int count = option == true ? 3 : 1;
    for (int i=0; i < count; i++) {
        value = getCompAir(ch);
        if(value < 0x80) { // Change the spec. value by JWY.  4Bar = 128 = 0x80
            setWarningCode(Warning_CompressedAir);
            if(value < 0x2B)        // Below 1 bar
                status = 2;
            else
                status = 1;

            if (count > 1)  APC_Delay(500);
        }
        else {
            break;
        }
    // 1Bar = 43 = 0x2B,  2Bar = 72 = 0x48, 3Bar = 92 = 0x5C, 4Bar = 128 = 0x80, 5Bar = 156 = 0x9C
    }

    _air_status = status;
    return status;
}

uint8_t getCompAirStatus()
{
    return _air_status;
}

uint8_t getCompAir(uint8_t ch)
{
    uint8_t data = 0;

    setSpiOption(SPI_OPTION_DEFAULT);

    do{
        setChipSelect(SPI_CS_MD_, APC_LOW);
        transferSpi(NULL, &data, 1, 100);
        //data = transfer8Spi(0x00);
        setChipSelect(SPI_CS_MD_, APC_HIGH);
    }while(data == 0xFF);
    _comp_air_val = data & 0xFF;    // normal value (0xc7 ~ 0xea)

    return _comp_air_val;
}

uint32_t getCompAirValue(void)
{
    if(_comp_air_val > 255)
        return 8200;
    else if(_comp_air_val < 12)
        return 0;
    else
        return ((5 * (uint32_t)_comp_air_val - 60) * 6);       // 6 = 1000 / 144
}
