/**************************************************************************************
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
**************************************************************************************/

/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************/

/**************************************************************************************/
/*! General Inclusion Area                                                            */
/**************************************************************************************/

#include <stdbool.h>
#include <malloc.h>

#include "App_DemoApplication.h"
#include "AppDNS_DemoApplication.h"
#include "App_PacketCommunication.h"
#include "OS_Dependent.h"

/* application data is kept global for use by multiples modules */
extern PROTOCOL_DESCRIPTOR_T g_tRealtimeProtocolHandlers;
extern APP_DATA_T g_tAppData;


/**************************************************************************************/
/*! FUNCTIONS                                                                         */
/**************************************************************************************/
/**************************************************************************************/
/*! Read board information
*
* \param hDriver     [in]   Handle if cifX driver
* \param ptBoardInfo [out]  Board information
*/
/**************************************************************************************/
void App_ReadBoardInfo( const CIFXHANDLE         hDriver,
                        BOARD_INFORMATION* const ptBoardInfo )
{
  CIFXHANDLE                       hSys     = NULL;
  SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK tSysInfo = ptBoardInfo->tSystemInfo;
  long                             lRet     = 0;      /** Return value for common error codes  */

  /** Retrieve board 0 information */
  if(CIFX_NO_ERROR != (lRet = xDriverEnumBoards(hDriver, 0, sizeof(BOARD_INFORMATION), ptBoardInfo)))
  {
    PRINTF("No board with the given index available: 0x%08X !" NEWLINE, (unsigned int)lRet);
  }
  else if(CIFX_NO_ERROR != (lRet = xSysdeviceOpen(hDriver, "cifX0", &hSys)))
  {
    PRINTF("Error opening SystemDevice: 0x%08X !" NEWLINE, (unsigned int)lRet);
  }
  /** System channel successfully opened, try to read the System Info Block */
  else if(CIFX_NO_ERROR != (lRet = xSysdeviceInfo(hSys, CIFX_INFO_CMD_SYSTEM_INFO_BLOCK, sizeof(tSysInfo), &tSysInfo)))
  {
    PRINTF("Error querying system information block: 0x%08X !" NEWLINE, (unsigned int)lRet);
  }
  else
  {
    PRINTF("System Channel Info Block:" NEWLINE);
    PRINTF("DPM Size         : %u" NEWLINE, (unsigned int)tSysInfo.ulDpmTotalSize);
    PRINTF("Device Number    : %u" NEWLINE, (unsigned int)tSysInfo.ulDeviceNumber);
    PRINTF("Serial Number    : %u" NEWLINE, (unsigned int)tSysInfo.ulSerialNumber);
    PRINTF("Manufacturer     : %u" NEWLINE, (unsigned int)tSysInfo.usManufacturer);
    PRINTF("Production Date  : %u" NEWLINE, (unsigned int)tSysInfo.usProductionDate);
    PRINTF("Device Class     : %u" NEWLINE, (unsigned int)tSysInfo.usDeviceClass);
    PRINTF("HW Revision      : %u" NEWLINE, (unsigned int)tSysInfo.bHwRevision);
    PRINTF("HW Compatibility : %u" NEWLINE, (unsigned int)tSysInfo.bHwCompatibility);

    xSysdeviceClose(hSys);
  }
}

/**************************************************************************************/
/*! Call the event handler function for all channels
*
*   \param ptAppData    [in]  Pointer to application data
*   \param hDriver      [in]  Drive handle retrieved with xDriverOpen()
*   \param szDeviceName [in]  Name of the board that shall be accessed
*
*   \return 0 on success.
*/
/**************************************************************************************/
int32_t App_AllChannels_Open( APP_DATA_T* ptAppData,
                              CIFXHANDLE  hDriver,
                              char*       szDeviceName )
{
  int i;
  int32_t lRet;

  for (i = 0; i < MAX_COMMUNICATION_CHANNEL_COUNT; i++)
  {
    if ((ptAppData->aptChannels[i] != NULL) &&
        (ptAppData->aptChannels[i]->tProtocol.pfStartChannelConfiguration != NULL))
    {
      if (CIFX_NO_ERROR != (lRet = xChannelOpen(hDriver, szDeviceName, i, &ptAppData->aptChannels[i]->hChannel)))
      {
        PRINTF("ERROR: xChannelOpen for %s, channel %d failed: 0x%08X" NEWLINE, szDeviceName, i, (unsigned int)lRet);
        return lRet;
      }
    }
  }
  return CIFX_NO_ERROR;
}

/**************************************************************************************/
/*! Get channel information of all channels
*
*   \param ptAppData    [in]  Pointer to application data
*
*   \return 0 on success.
*/
/**************************************************************************************/
void App_AllChannels_GetChannelInfo_WaitReady( APP_DATA_T* ptAppData )
{
  int i;

  for (i = 0; i < MAX_COMMUNICATION_CHANNEL_COUNT; i++)
  {
    if ((ptAppData->aptChannels[i] != NULL) &&
        (ptAppData->aptChannels[i]->hChannel != NULL))
    {
      do
      {
        xChannelInfo(ptAppData->aptChannels[i]->hChannel, sizeof(CHANNEL_INFORMATION), &ptAppData->aptChannels[i]->tChannelInfo);
      }
      while (!(ptAppData->aptChannels[i]->tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY) ||
              (ptAppData->aptChannels[i]->tChannelInfo.ulDeviceCOSFlags == CIFX_DPM_NO_MEMORY_ASSIGNED));
    }
  }
}

/**************************************************************************************/
/*! Call the start configuration function for all channels
*
*   \param ptAppData      [in]  Pointer to application data
*
*   \return 0 on success.
*/
/**************************************************************************************/
uint32_t App_AllChannels_Configure( APP_DATA_T* ptAppData )
{
  int     i;
  int32_t ulRet = 0;

  //Protocol_StartConfiguration

  for (i = 0; i < MAX_COMMUNICATION_CHANNEL_COUNT; i++)
  {
    if ((ptAppData->aptChannels[i] != NULL) &&
        (ptAppData->aptChannels[i]->hChannel != NULL) &&
        (ptAppData->aptChannels[i]->tProtocol.pfStartChannelConfiguration != NULL))
    {
      if (CIFX_NO_ERROR != (ulRet = ptAppData->aptChannels[i]->tProtocol.pfStartChannelConfiguration(ptAppData)))
      {
        PRINTF("Error: Protocol_StartConfiguration failed: 0x%08X" NEWLINE, (unsigned int)ulRet);
        return ulRet;
      }
    }
  }
  return 0;
}

/**************************************************************************************/
/*! Call the packet handler function for all channels
*
*   \param ptAppData      [in]  Pointer to application data
*
*   \return 0 on success.
*/
/**************************************************************************************/
uint32_t App_AllChannels_PacketHandler( APP_DATA_T* ptAppData )
{
  int     i;
  int32_t ulRet = 0;

  for (i = 0; i < MAX_COMMUNICATION_CHANNEL_COUNT; i++)
  {
    if ((ptAppData->aptChannels[i] != NULL) &&
        (ptAppData->aptChannels[i]->hChannel != NULL) &&
        (ptAppData->aptChannels[i]->tProtocol.pfPacketHandler != NULL))
    {
      if (0 != (ulRet = ptAppData->aptChannels[i]->tProtocol.pfPacketHandler(ptAppData)))
      {
        PRINTF("Error: Protocol_PacketHandler failed: 0x%08X" NEWLINE, (unsigned int)ulRet);
        return ulRet;
      }
    }
  }
  return 0;
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

  for (i = 0; i < MAX_COMMUNICATION_CHANNEL_COUNT; i++)
  {
    if ((ptAppData->aptChannels[i] != NULL) &&
        (ptAppData->aptChannels[i]->hChannel != NULL))
    {
      xChannelClose( ptAppData->aptChannels[i]->hChannel );
      ptAppData->aptChannels[i]->hChannel = NULL;
    }
  }
}


/**************************************************************************************/
/*! Entry point of cifX Application Demo.
 *
 * \param szDeviceName Name of the cifX device, e.g., "cifX0"
 *
 * \return CIFX_NO_ERROR on success
 */
/**************************************************************************************/
int App_CifXApplicationDemo(char *szDeviceName)
{
  CIFXHANDLE hDriver  = NULL;  /** Handle of cifX driver               */
  int32_t   lRet      = 0;     /** Return value for common error codes */
  uint32_t  ulState   = 0;     /** Actual state returned               */
  uint32_t  ulTimeout = 1000;  /** Timeout in milliseconds             */

  PRINTF("---------- cifX Application Demo ----------" NEWLINE);

  g_tAppData.fRunning = true;

  /* We know the firmware being executed, so we map our different protocol modules directly
     to the channels here. Each protocol module brings a descriptor, providing distinct entry points for
     configuration and packet handling.
  */
  /* communication channel 0 is used by default, it must be explicitly disabled */

  g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)calloc(1, sizeof(APP_COMM_CHANNEL_T));
  g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

  /* open driver */
  if (CIFX_NO_ERROR != (lRet = xDriverOpen(&hDriver)))
  {
    PRINTF("ERROR: xDriverOpen failed: 0x%08x" NEWLINE, (unsigned int)lRet);
    return lRet;
  }

  App_ReadBoardInfo(hDriver, &g_tAppData.tBoardInfo);

  /** Open all communication channels */
  if (CIFX_NO_ERROR != App_AllChannels_Open(&g_tAppData, hDriver, szDeviceName))
  {
    PRINTF("ERROR: Failed to open communication channels: 0x%08x" NEWLINE, (unsigned int)lRet);
    goto error_exit;
  }



  /** Wait for HIL_COMM_COS_READY set in all relevant channels */
  App_AllChannels_GetChannelInfo_WaitReady(&g_tAppData);

  /** Set the Application state flag in the application COS flags */
  if (CIFX_NO_ERROR != (lRet = xChannelHostState(g_tAppData.aptChannels[0]->hChannel, CIFX_HOST_STATE_READY, &ulState, ulTimeout)))
  {
    PRINTF("ERROR: xChannelHostState failed: 0x%08X" NEWLINE, (unsigned int)lRet);
    goto error_exit;
  }

  /* Download the configuration */
  if (CIFX_NO_ERROR != (lRet = App_AllChannels_Configure(&g_tAppData)))	//Protocol_StartConfiguration
  {
    PRINTF("Error: A channel failed to configure: 0x%08X" NEWLINE, (unsigned int)lRet);
    goto error_exit;
  }

  /** now the bus is running */
  while(g_tAppData.fRunning && lRet == CIFX_NO_ERROR)
  {
    /** check and process incoming packets */

	App_IODataHandler(&g_tAppData);
	lRet = App_AllChannels_PacketHandler(&g_tAppData);

    OS_Sleep(1);
  }

error_exit:
  if ((g_tAppData.aptChannels[0] != NULL) &&
      (g_tAppData.aptChannels[0]->hChannel != NULL))
  {
    /** Set the bus state flag in the application COS state flags, to stop communication */
    xChannelBusState(g_tAppData.aptChannels[0]->hChannel, CIFX_BUS_STATE_OFF, &ulState, 10);

    /** Set Host not ready to stop bus communication */
    xChannelHostState(g_tAppData.aptChannels[0]->hChannel, CIFX_HOST_STATE_NOT_READY, &ulState, ulTimeout);
  }

  /** close all communication channels */
  App_AllChannels_Close(&g_tAppData);

  {
    /* free all channel memory */
    int iChannelIdx = 0;
    for (iChannelIdx = 0; iChannelIdx < MAX_COMMUNICATION_CHANNEL_COUNT; iChannelIdx++)
    {
      if (g_tAppData.aptChannels[iChannelIdx] != NULL)
      {
        free(g_tAppData.aptChannels[iChannelIdx]);
      }
    }
  }

  Pkt_Deinit();

  g_tAppData.fRunning = false;

  /** Close driver */
  xDriverClose(hDriver);
  hDriver = NULL;

  PRINTF(" Application terminated: Status = 0x%08X !" NEWLINE, (unsigned int)lRet);
  PRINTF("----------------------------------------------------" NEWLINE);

  return lRet;
}


/* in some rare cases no cyclic access to the IO data area of the DPM is required,
 * in these cases only DPM mailbox packets are used for communication between host application and protocol stack
 * the macro DEMO_ONLY_MAILBOX can be set in the wscript in order to remove the code related to cyclic IO data access*/
#ifndef DEMO_ONLY_MAILBOX
/**************************************************************************************/
/*! Cyclic IO data handler: fast data exchange with netX via DPM called from a timer ISR
 *
 * \param ptAppData  [in]  Pointer to application data
 */
/**************************************************************************************/
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR; /** Return value for common error codes  */
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;

  if(ptAppData->aptChannels[0]->hChannel != NULL)
  {
    /** INPUT DATA *********************************************************************/
    lRet = xChannelIORead(ptAppData->aptChannels[0]->hChannel, 0, 0, sizeof(ptAppData->tInputData), &ptAppData->tInputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
      /** Something failed?
       * Reason for error could be:
       * 1) netX is not "ready" yet. May happen during startup.
       * 2) netX is not "running" yet. May happen during startup in case the netX is not fully configured yet.
       * 3) netX has not yet established an IO connection. */

      ptAppData->fInputDataValid = false;
    }
    else
    {
      /** process newly received input data image */
      ptAppData->fInputDataValid = true;

    }

    /** OUTPUT DATA ***************************************/
    /** update output data image to be sent in this cycle */
    ptAppData->tOutputData.output[0]++;
    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0, sizeof(ptAppData->tOutputData), &ptAppData->tOutputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
      /** Something failed?
       * Reason for error could be:
       * 1) netX is not "ready" yet. May happen during startup.
       * 2) netX is not "running" yet. May happen during startup in case the netX is not fully configured yet.
       * 3) netX has not yet established an IO connection. */
    }
  }
}
#endif
