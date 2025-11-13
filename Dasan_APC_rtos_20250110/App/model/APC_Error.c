/*
 * APC_Error.c
 *
 *  Created on: 2023. 11. 12.
 *      Author: Yongseok
 */

#include "APC_Error.h"

#include <APC_AsyncHandler.h>

static bool _error_initialized = false;

static uint8_t     _ErrorCode;
static uint8_t     _FatalErrorCode;


void initError(void)
{
    _error_initialized = true;
}

uint8_t getErrorCode(void)
{
    return _ErrorCode;
}

bool setErrorCode(uint8_t err_num)
{
    if (err_num < 0 || err_num > RSinterface_BufferOverrun) {
        return false;
    }
    _ErrorCode = err_num;
    return true;
}

uint8_t getFatalErrorCode(void)
{
    return _FatalErrorCode;
}

bool setFatalErrorCode(uint8_t err_num)
{
    if (err_num < E1_NoValueDefined || err_num > E40_MotorDriverFailure) {
        return false;
    }
    _FatalErrorCode = err_num;
    return true;
}

void resetError(uint8_t option)
{
    if (option == 0) {
        // reset service request WARNINGS
        ;
    }
    else if (option == 1) {
        // TODO restart Control Unit
        setRestartFw(DATA_SYNC_FULL);
    }

}
