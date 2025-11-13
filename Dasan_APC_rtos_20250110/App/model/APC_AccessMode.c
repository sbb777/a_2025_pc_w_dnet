/*
 * APC_AccessMode.c
 *
 *  Created on: 2023. 11. 10.
 *      Author: Yongseok
 */

#include <APC_AccessMode.h>
#include "APC_Error.h"

#include "APC_Utils.h"
#include "APC_Valve.h"


static bool _mode_initialized = false;

static EN_AccessMode       _AccessMode;


typedef struct {
    uint32_t local_counter;
    uint32_t remote_counter;
    uint32_t internal_counter;
} AccessModeType;

static AccessModeType access_mode;



bool initAccessMode(void)
{
    _AccessMode  = AccessMode_Remote;

    access_mode.local_counter = 0;
    access_mode.remote_counter = 0;
    access_mode.internal_counter = 0;

    _mode_initialized = true;
    return _mode_initialized;
}

/*
 * Local Port 상태를 갱신하기 위해 주기적으로 호출 필요
 * 1) CmdListener에서 지속적으로 호출
 */
void refreshAccessMode(uint8_t ch, uint32_t counter)
{
    if (ch == APC_PORT_LOCAL)
    {
        access_mode.local_counter = counter;
    }
    else if (ch == APC_PORT_REMOTE)
    {
        access_mode.remote_counter = counter;
    }
    else
    {
        access_mode.internal_counter = counter;
        if (_AccessMode == AccessMode_Local)
        {
            uint32_t gap = 0;
            gap = access_mode.internal_counter > access_mode.local_counter
                  ? access_mode.internal_counter - access_mode.local_counter
                  : access_mode.internal_counter + (UINT32_MAX - access_mode.local_counter);

            if (gap > LOCAL_LIFE_TIME)
            {
                _AccessMode = AccessMode_Remote;
                prtLog(0, LL_INFO, __FUNCTION__, "AccessMode changed into %d", _AccessMode);
            }
        }
    }
}

/**
 * If true, return 0. Otherwise, return ErrorCode.
 */
uint8_t setAccessMode(uint8_t ch, uint8_t req_mode)
{
    if (req_mode < AccessMode_Local || req_mode > AccessMode_Locked) {
        prtLog(0, LL_WARN, __FUNCTION__, "Requested AccessMode=[%d]\n", req_mode);
        return Status_WrongAccessMode;
    }

    uint8_t curAccessMode = getAccessMode();

    if (ch == APC_PORT_LOCAL) {
        /*
         * AccessMode_Locked 해제는 Remote에서만 가능
         */
        if (curAccessMode == AccessMode_Locked) {
            prtLog(0, LL_WARN, __FUNCTION__, "AccessMode is Locked\n");
            return Status_WrongAccessModeLocked;
        }
    }
    else {
        /*
         * AccessMode_Local 상태에서는 Remote, Locked 불가
         */
        if (curAccessMode == AccessMode_Local) {
            prtLog(0, LL_WARN, __FUNCTION__, "can NOT change AccessMode=[%d]\n", curAccessMode);
            return Status_WrongAccessMode;
        }
    }

    _AccessMode = req_mode;
    return 0;
}

uint8_t getAccessMode(void)
{
    return _AccessMode;
}

