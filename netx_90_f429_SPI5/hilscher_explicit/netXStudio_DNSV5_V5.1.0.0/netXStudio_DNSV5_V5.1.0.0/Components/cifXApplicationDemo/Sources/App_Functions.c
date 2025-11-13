/**************************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

Exclusion of Liability for this demo software:
  The following software is intended for and must only be used for reference and in an
  evaluation laboratory environment. It is provided without charge and is subject to
  alterations. There is no warranty for the software, to the extent permitted by
  applicable law. Except when otherwise stated in writing the copyright holders and/or
  other parties provide the software "as is" without warranty of any kind, either
  expressed or implied.
  Please refer to the Agreement in README_DISCLAIMER.txt, provided together with this file!
  By installing or otherwise using the software, you accept the terms of this Agreement.
  If you do not agree to the terms of this Agreement, then do not install or use the
  Software!

**************************************************************************************************/

#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"
#include "Hil_DualPortMemory.h"

#include <string.h>

/*! Print boards
 * This helper function print all available boards
 *
 * \param hDriver     [in]   Handle if cifX driver
 */
void App_PrintBoards(const CIFXHANDLE hDriver)
{
  int32_t               lRet         = CIFX_NO_ERROR;
  DRIVER_INFORMATION    tDrvInfo     = {{0}};
  BOARD_INFORMATION     tBoardInfo   = {0};
  unsigned long         ulBoardIndex = 0;

  if (CIFX_NO_ERROR != (lRet = xDriverGetInformation(hDriver, sizeof(tDrvInfo), &tDrvInfo)))
  {
    PRINTF("ERROR: xDriverGetInformation failed: 0x%08x" NEWLINE, (unsigned int)lRet);
    return;
  }

  /* iterate through all boards. */
  for (ulBoardIndex = 0; ulBoardIndex < tDrvInfo.ulBoardCnt; ulBoardIndex++)
  {
    lRet = xDriverEnumBoards(hDriver, ulBoardIndex, sizeof(tBoardInfo), &tBoardInfo);
    if (CIFX_NO_ERROR == lRet)
    {
      PRINTF("System Channel Info Block of \"%s\" Device:" NEWLINE, tBoardInfo.abBoardName);
      PRINTF("DPM Size         : %u" NEWLINE, (unsigned int)tBoardInfo.tSystemInfo.ulDpmTotalSize);
      PRINTF("Device Number    : %u" NEWLINE, (unsigned int)tBoardInfo.tSystemInfo.ulDeviceNumber);
      PRINTF("Serial Number    : %u" NEWLINE, (unsigned int)tBoardInfo.tSystemInfo.ulSerialNumber);
      PRINTF("Manufacturer     : %u" NEWLINE, (unsigned int)tBoardInfo.tSystemInfo.usManufacturer);
      PRINTF("Production Date  : %u" NEWLINE, (unsigned int)tBoardInfo.tSystemInfo.usProductionDate);
      PRINTF("Device Class     : %u" NEWLINE, (unsigned int)tBoardInfo.tSystemInfo.usDeviceClass);
      PRINTF("HW Revision      : %u" NEWLINE, (unsigned int)tBoardInfo.tSystemInfo.bHwRevision);
      PRINTF("HW Compatibility : %u" NEWLINE, (unsigned int)tBoardInfo.tSystemInfo.bHwCompatibility);
    } else
    {
      PRINTF("ERROR: xDriverEnumBoards failed: 0x%08x" NEWLINE, (unsigned int)lRet);
    }
  }
}

/*! Call the start configuration function for all channels
*
*   \param ptAppData      [in]  Pointer to application data
*
*   \return 0 on success.
*/
int32_t App_ConfigureChannels(APP_DATA_T* ptAppData)
{
  int32_t lRet = CIFX_NO_ERROR;

  for (int i = 0; i < USED_COMMUNICATION_CHANNELS; i++)
  {
    APP_COMM_CHANNEL_T *ptCommChannel =  &ptAppData->atCommChannels[i];

    if (ptCommChannel->tCommChannelHandler.pfnSetup != NULL){
      lRet = ptCommChannel->tCommChannelHandler.pfnSetup(ptCommChannel->hCommChannelRsc);
      if (CIFX_NO_ERROR != lRet){
        PRINTF("ERROR: Channel setup failed: 0x%08X" NEWLINE, (unsigned int)lRet);
        return lRet;
      }

    }
  }
  return CIFX_NO_ERROR;
}

/*! Call the packet handler function for all channels
*
*   \param ptAppData      [in]  Pointer to application data
*
*   \return 0 on success.
*/
void App_CallRoutineTasks( APP_DATA_T* ptAppData )
{
  for (int i = 0; i < USED_COMMUNICATION_CHANNELS; i++)
  {
    APP_COMM_CHANNEL_T *ptCommChannel =  &ptAppData->atCommChannels[i];

    if (ptCommChannel->tCommChannelHandler.pfnRoutineTask != NULL){
      ptCommChannel->tCommChannelHandler.pfnRoutineTask(ptCommChannel->hCommChannelRsc);
    }
  }
}

/**************************************************************************************/
/*! Call the terminal handler function for all channels
*
*   \param ptAppData        [in]  Pointer to application data
*   \param szTerminalBuffer [in]  Command line input
*
*   \return 0 on success.
*/
/**************************************************************************************/
void App_AllChannels_TerminalHandler( APP_DATA_T* ptAppData,
                                      char*       szTerminalBuffer )
{

}

/**************************************************************************************/
/*! Call the event handler function for all channels
*
*   \param ptAppData    [in]  Pointer to application data
*   \param szBoardName  [in]  Name of the board that shall be accessed
*
*   \return 0 on success.
*/
/**************************************************************************************/
int32_t App_InitializeChannels(APP_DATA_T* ptAppData,
                               char*       szBoardName)
{
  int     i    = 0;
  int32_t lRet = CIFX_NO_ERROR;

  for (i = 0; i < USED_COMMUNICATION_CHANNELS; i++)
  {
    APP_COMM_CHANNEL_T  *ptCommChannel = &ptAppData->atCommChannels[i];
    CHANNEL_INFORMATION *ptChannelInfo = &ptCommChannel->tCifXChannelInfo;
    uint32_t            ulState        = 0;

    if (ptCommChannel->tCommChannelHandler.pfnInitialize != NULL)
    {
      /* Open the driver handle of the channel */
      lRet = xChannelOpen(ptAppData->hCifXDriver, szBoardName, i, &ptCommChannel->hCifXChannel);
      if (CIFX_NO_ERROR != lRet){
        PRINTF("ERROR: xChannelOpen for %s, channel %d failed: 0x%08X" NEWLINE, szBoardName, i, (unsigned int)lRet);
        return lRet;
      }

      /* Wait until the channel is ready (If the channel is not ready we can't send packets) */
      do{
        memset(ptChannelInfo, 0, sizeof(*ptChannelInfo));

        /** Retrieve the global communication channel information */
        if(CIFX_NO_ERROR != (lRet = xChannelInfo(ptCommChannel->hCifXChannel, sizeof(CHANNEL_INFORMATION), ptChannelInfo)))
        {
          PRINTF("ERROR: Querying communication channel information block: 0x%08X !" NEWLINE, (unsigned int) lRet);
          return lRet;
        }
        else
        {
          PRINTF("Communication Channel Info:" NEWLINE);
          PRINTF("Device Number   : %u" NEWLINE, (unsigned int) ptChannelInfo->ulDeviceNumber);
          PRINTF("Serial Number   : %u" NEWLINE, (unsigned int) ptChannelInfo->ulSerialNumber);
          PRINTF("Firmware        : %s" NEWLINE, ptChannelInfo->abFWName);
          PRINTF("FW Version      : %u.%u.%u.%u" NEWLINE,
          (unsigned int) ptChannelInfo->usFWMajor,
          (unsigned int) ptChannelInfo->usFWMinor,
          (unsigned int) ptChannelInfo->usFWBuild,
          (unsigned int) ptChannelInfo->usFWRevision);

          PRINTF("FW Date         : %02u/%02u/%04u" NEWLINE,
          (unsigned int) ptChannelInfo->bFWMonth,
          (unsigned int) ptChannelInfo->bFWDay,
          (unsigned int) ptChannelInfo->usFWYear);
          PRINTF("Mailbox Size    : %u" NEWLINE,
          (unsigned int) ptChannelInfo->ulMailboxSize);
        }
      }while(  !(ptChannelInfo->ulDeviceCOSFlags & HIL_COMM_COS_READY)
             || (ptChannelInfo->ulDeviceCOSFlags == CIFX_DPM_NO_MEMORY_ASSIGNED));

      /** Set the Application state flag in the application COS flags */
      if (CIFX_NO_ERROR != (lRet = xChannelHostState(ptCommChannel->hCifXChannel, CIFX_HOST_STATE_READY, &ulState, 5)))
      {
        PRINTF("ERROR: xChannelHostState failed: 0x%08X" NEWLINE, (unsigned int)lRet);
        return lRet;
      }

      /** Initialize the Stack hander */
      if (CIFX_NO_ERROR != (lRet = ptCommChannel->tCommChannelHandler.pfnInitialize(&ptCommChannel->hCommChannelRsc, ptCommChannel->hCifXChannel, ptChannelInfo)))
      {
        PRINTF("ERROR: Channel initialize failed: 0x%08X" NEWLINE, (unsigned int)lRet);
        return lRet;
      }
    }
  }

  return CIFX_NO_ERROR;
}


/**************************************************************************************/
/*! Close all opened channels
*
*   \param ptAppData    [in]  Pointer to application data
*
*   \return 0 on success.
*/
/**************************************************************************************/
void App_AllChannels_Close( APP_DATA_T* ptAppData )
{
  int i;
  for (i = 0; i < USED_COMMUNICATION_CHANNELS; i++)
  {
    APP_COMM_CHANNEL_T *ptCommChannel =  &ptAppData->atCommChannels[i];
    if (ptCommChannel->hCifXChannel != NULL)
    {
      xChannelClose(ptCommChannel->hCifXChannel);
      ptCommChannel->hCifXChannel = NULL;
    }
  }
}
