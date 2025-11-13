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

#include "cifXErrors.h"

#include <stdbool.h>
#include <malloc.h>


/**/
extern APP_COMM_CHANNEL_HANDLER_T g_tRealtimeProtocolHandler;
extern APP_COMM_CHANNEL_HANDLER_T g_tNetworkServicesHandlers;
extern APP_COMM_CHANNEL_HANDLER_T g_tNetProxyHandlers;

/** Application resources **/
APP_DATA_T  g_tAppData = {0};


void App_ApplicationTimerJob()
{
  APP_DATA_T* ptAppData = &g_tAppData;

  for(int i = 0; i < USED_COMMUNICATION_CHANNELS; i++)
  {
    if(ptAppData->atCommChannels[i].tCommChannelHandler.pfnTimerTask &&
       ptAppData->atCommChannels[i].hCommChannelRsc)
    {
      ptAppData->atCommChannels[i].tCommChannelHandler.pfnTimerTask(ptAppData->atCommChannels[i].hCommChannelRsc);
    }
  }
}

int App_ApplicationDemo(char *szBoardName)
{
  int32_t     lRet       = 0;
  APP_DATA_T* ptAppData  = &g_tAppData;

  PRINTF("---------- cifX Application Demo ----------" NEWLINE);

  /** Copy the channel communication handlers to our resources */
  ptAppData->atCommChannels[0].tCommChannelHandler = g_tRealtimeProtocolHandler;
#if USED_COMMUNICATION_CHANNELS >= 2
  ptAppData->atCommChannels[1].tCommChannelHandler = g_tNetworkServicesHandlers;
#endif
#if USED_COMMUNICATION_CHANNELS >= 3
  ptAppData->atCommChannels[2].tCommChannelHandler = g_tNetProxyHandlers;
#endif

  /** Open the cifX driver */
  lRet = xDriverOpen(&ptAppData->hCifXDriver);
  if (CIFX_NO_ERROR != lRet){
    PRINTF("ERROR: xDriverOpen failed: 0x%08x" NEWLINE, (unsigned int)lRet);
    return lRet;
  }

  /** This section is only for demo and debug purpose */
  if (CIFX_NO_ERROR == lRet){
    App_PrintBoards(ptAppData->hCifXDriver);
  }

  if (CIFX_NO_ERROR == lRet){
    /** Open and initialize all channels where a stack handler is available */
    lRet = App_InitializeChannels(ptAppData, szBoardName);
    if (lRet){
      PRINTF("ERROR: Failed to open communication channels: 0x%08x" NEWLINE, (unsigned int)lRet);
    }
  }

  if (CIFX_NO_ERROR == lRet){
    /** Call the Configure function of all channels */
    lRet = App_ConfigureChannels(ptAppData);
    if (lRet){
      PRINTF("Error: A channel failed to configure: 0x%08X" NEWLINE, (unsigned int)lRet);
    }
  }

  /* This is our "main demo process loop" */
  while(lRet == CIFX_NO_ERROR)
  {
    /** Call Routine tasks of all stack handlers */
    App_CallRoutineTasks(ptAppData);
  }

  return lRet;
}

