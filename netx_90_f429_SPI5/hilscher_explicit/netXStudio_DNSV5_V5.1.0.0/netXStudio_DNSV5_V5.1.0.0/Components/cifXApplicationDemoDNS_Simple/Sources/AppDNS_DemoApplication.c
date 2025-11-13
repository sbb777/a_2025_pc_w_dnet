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
#include "DNS_includes.h"

#include "AppDNS_DemoApplication_Config.h"
#include "AppDNS_DemoApplication.h"


#ifdef DNS_HOST_APP_TERMINAL_COMMAND_ENABLE
extern uint32_t Terminal_GetCommand(char* pszCmd, uint32_t ulCmdBufferSize);
uint32_t AppDNSDemo_Command(APP_DNS_CHANNEL_HANDLER_RSC_T* ptOmbRsc, char *szBuffer);
#endif


static APP_DNS_CHANNEL_HANDLER_RSC_T s_tDnsRsc = {0};


/***************************************************************************************
* Firmware verification.
****************************************************************************************/
PROTOCOL_IDENTIFICATION_T g_tProtocolIdentification =
{
  .szFirmwareName = "DeviceNet Slave",
  .usFirmwareMajor = 5,
  .usFirmwareMinor = 4,
};


/*************************************************************************************/
static int appDNSDemo_Initialize(APP_COMM_CHANNEL_HANDLER_RSC_H* phStackRsc,
                                 CIFXHANDLE                      hCifXChannel,
                                 CHANNEL_INFORMATION*            ptCifXChannelInfo);

static int  appDNSDemo_Setup      (APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc);
static void appDNSDemo_RoutineTask(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc);
static void appDNSDemo_IOTask     (APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc);


APP_COMM_CHANNEL_HANDLER_T g_tRealtimeProtocolHandler =
{
  .pfnInitialize  = appDNSDemo_Initialize,
  .pfnSetup       = appDNSDemo_Setup,
  .pfnRoutineTask = appDNSDemo_RoutineTask,
  .pfnTimerTask   = appDNSDemo_IOTask,
};


#ifdef DNS_HOST_APP_TERMINAL_COMMAND_ENABLE
/***************************************************************************************
*! Protocol specific terminal handler.
*   \param ptAppData   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
*   \param szBuffer    terminal command
****************************************************************************************/
uint32_t AppDNSDemo_Command(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc, char *szBuffer)
{
  if (0 == strcmp(szBuffer, "help"))
  {
    DNS_PRINTF("Hello from the DNS protocol part: no terminal commands implemented yet" NEWLINE);
  }
  else
  {
    DNS_PRINTF(" 'help' - list commands" NEWLINE);
  }

  return 0;
}
#endif


/*******************************************************************************/
static int appDNSDemo_Initialize(APP_COMM_CHANNEL_HANDLER_RSC_H* phStackRsc,
                                 CIFXHANDLE                      hCifXChannel,
                                 CHANNEL_INFORMATION*            ptCifXChannelInfo)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  APP_DNS_CHANNEL_HANDLER_RSC_T *ptDnsRsc =  &s_tDnsRsc;
  ptDnsRsc->hCifXChannel      = hCifXChannel;
  ptDnsRsc->ptCifXChannelInfo = ptCifXChannelInfo;

  *phStackRsc = ptDnsRsc;

  if(Pkt_Init(&ptDnsRsc->hPktIfRsc, ptDnsRsc->hCifXChannel))
  {
    /* Register handler for indication packets. */
    if(Pkt_RegisterPacketHandler(ptDnsRsc->hPktIfRsc, AppDNS_PacketHandler, (void*)ptDnsRsc))
    {
#ifdef DNS_HOST_APP_REGISTRATION
      /* Register application */
      App_SysPkt_AssembleRegisterAppReq(&ptDnsRsc->tPacket);

      ulRet = Pkt_SendReceivePacket(ptDnsRsc->hPktIfRsc,
                                    &ptDnsRsc->tPacket,
                                    TXRX_TIMEOUT);
      if(CIFX_NO_ERROR != ulRet)
      {
        return (int)ulRet;
      }
#endif
    }
  }

 return (int)ulRet;
}


/*******************************************************************************/
static int appDNSDemo_Setup(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  /* Download the configuration */
  ulRet = AppDNS_ConfigureStack(ptDnsRsc, 0);

  return ulRet;
}


/*******************************************************************************/
static void appDNSDemo_RoutineTask(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  Pkt_CheckReceiveMailbox(ptDnsRsc->hPktIfRsc, &ptDnsRsc->tPacket);

#ifdef DNS_HOST_APP_TERMINAL_COMMAND_ENABLE
  char szCmd[64] = { 0 };
  uint32_t ulCmdSize = 0;

  ulCmdSize = Terminal_GetCommand(&szCmd[0], sizeof(szCmd));
  if (0 != ulCmdSize)
  {
    AppDNSDemo_Command(ptDnsRsc, szCmd);
  }
#endif
}


/*******************************************************************************/
static void appDNSDemo_IOTask(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  int32_t lRet = CIFX_NO_ERROR;

  /** INPUT DATA *********************************************************************/
  lRet = xChannelIORead(ptDnsRsc->hCifXChannel, 0, 0, sizeof(ptDnsRsc->abActorData), ptDnsRsc->abActorData, 0);
  if(CIFX_NO_ERROR != lRet)
  {
    /** Something failed?
     * Reason for error could be:
     * 1) netX is not "ready" yet. May happen during startup.
     * 2) netX is not "running" yet. May happen during startup in case the netX is not fully configured yet.
     * 3) netX has not yet established an IO connection. */
  }
  else
  {
    /** process newly received input data image */
  }

  /* Mirror Data */
  ptDnsRsc->abSensorData[0] = ptDnsRsc->abActorData[0] + 1;


  /** OUTPUT DATA ***************************************/
  lRet = xChannelIOWrite(ptDnsRsc->hCifXChannel,  0, 0, sizeof(ptDnsRsc->abSensorData), ptDnsRsc->abSensorData, 0);
  if(CIFX_NO_ERROR != lRet)
  {
    /** Something failed?
     * Reason for error could be:
     * 1) netX is not "ready" yet. May happen during startup.
     * 2) netX is not "running" yet. May happen during startup in case the netX is not fully configured yet.
     * 3) netX has not yet established an IO connection. */
  }
}


