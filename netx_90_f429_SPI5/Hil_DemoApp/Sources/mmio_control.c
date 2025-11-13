#include "mmio_control.h"
#include "cifXErrors.h"
#include <stdbool.h> // For bool type
#include <string.h> // For memset
#include "OS_Dependent.h" // For OS_GetMilliSecCounter and OS_Sleep

/**
 * @brief Waits for a response packet from the netX90 and checks its status.
 *
 * @param hChannel The channel handle.
 * @return CIFX_NO_ERROR on success, otherwise an error code from the response packet or timeout.
 */
static int32_t WaitForResponsePacket(CIFXHANDLE hChannel)
{
    CIFX_PACKET tRsp;
    int32_t lRet;
    uint32_t ulRecvPktCount = 0;
    uint32_t ulSendPktCount = 0;
    uint32_t ulTimeout = 1000; // 1 second timeout
    uint32_t ulStartTime = OS_GetMilliSecCounter();

    do
    {
        lRet = xChannelGetMBXState(hChannel, &ulRecvPktCount, &ulSendPktCount);
        if (lRet != CIFX_NO_ERROR)
        {
            return lRet; // Error getting mailbox state
        }

        if (ulRecvPktCount > 0)
        {
            lRet = xChannelGetPacket(hChannel, sizeof(tRsp), &tRsp, ulTimeout);
            if (lRet == CIFX_NO_ERROR)
            {
                return tRsp.tHeader.ulState; // Return status from response packet
            }
            else
            {
                return lRet; // Error getting packet
            }
        }
        OS_Sleep(10); // Wait a bit before checking again
    } while ((OS_GetMilliSecCounter() - ulStartTime) < ulTimeout);

    return CIFX_DEV_GET_TIMEOUT; // Timeout if no response received
}

/**
 * @brief Configures MMIO7 as an output.
 *
 * @param hChannel The channel handle.
 * @return CIFX_NO_ERROR on success, otherwise an error code.
 */
int32_t SetMMIO7AsOutput(CIFXHANDLE hChannel)
{
    DIO_SET_LINE_CONFIG_REQ tCfg;
    memset(&tCfg, 0, sizeof(tCfg));

    tCfg.tHeader.ulDest = 0x20;
    tCfg.tHeader.ulCmd = 0xB1; // DIO_SET_LINE_CONFIG
    tCfg.tHeader.ulLen = sizeof(tCfg) - sizeof(tCfg.tHeader);
    tCfg.ulLine = 7; // MMIO7
    tCfg.ulType = DRV_DIO_LINE_MMIO;
    tCfg.ulFlags = 0; // Output

    int32_t lRet = xChannelPutPacket(hChannel, (CIFX_PACKET*)&tCfg, 1000);
    if (lRet == CIFX_NO_ERROR)
    {
        lRet = WaitForResponsePacket(hChannel);
    }

    return lRet;
}

/**
 * @brief Sets the state of MMIO7.
 *
 * @param hChannel The channel handle.
 * @param fHigh true to set the line high, false to set it low.
 * @return CIFX_NO_ERROR on success, otherwise an error code.
 */
int32_t SetMMIO7State(CIFXHANDLE hChannel, bool fHigh)
{
    DIO_SET_LINE_STATE_REQ tReq;
    memset(&tReq, 0, sizeof(tReq));

    tReq.tHeader.ulDest = 0x20;
    tReq.tHeader.ulCmd = 0xB2; // DIO_SET_LINE_STATE
    tReq.tHeader.ulLen = sizeof(tReq) - sizeof(tReq.tHeader);
    tReq.ulLine = 7; // MMIO7
    tReq.ulType = DRV_DIO_LINE_MMIO;
    tReq.ulState = fHigh ? 1 : 0;

    int32_t lRet = xChannelPutPacket(hChannel, (CIFX_PACKET*)&tReq, 1000);
    if (lRet == CIFX_NO_ERROR)
    {
        lRet = WaitForResponsePacket(hChannel);
    }

    return lRet;
}