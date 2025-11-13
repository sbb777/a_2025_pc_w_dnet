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

static APP_DNS_CHANNEL_HANDLER_RSC_T s_tDnsRsc = {0};


#ifdef DNS_HOST_APP_TERMINAL_COMMAND_ENABLE
extern uint32_t Terminal_GetCommand(char* pszCmd, uint32_t ulCmdBufferSize);
uint32_t AppDNSDemo_Command(APP_DNS_CHANNEL_HANDLER_RSC_T* ptOmbRsc, char *szBuffer);
#endif


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



/***************************************************************************************
*! Protocol specific terminal handler.
*   \param ptAppData   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
*   \param szBuffer    terminal command
****************************************************************************************/
uint32_t AppDNSDemo_Command(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc, char *szBuffer)
{
  uint8_t bNodeIdSwitchValue = 0;
  uint8_t bBaudSwitchValue = 0;
  char szArg1[3] = {0,0,0};
  char szArg2[3] = {0,0,0};

  memcpy(&szArg1[0], szBuffer,   1);
  memcpy(&szArg2[0], szBuffer+1, 3);

  if (0 == strcmp(szBuffer, "help"))
  {
    DNS_PRINTF("Hello from the DNS protocol part:" NEWLINE);
    DNS_PRINTF(" 'reconfig':Reconfigure the stack" NEWLINE);
    DNS_PRINTF(" '+'       :Simulate NodeId Switch rotate UP" NEWLINE);
    DNS_PRINTF(" '-'       :Simulate NodeId Switch rotate DOWN" NEWLINE);
    DNS_PRINTF(" 'nxx'     :Set NodeId   Switch to x (0 .. 99)" NEWLINE);
    DNS_PRINTF(" 'rx'      :Set Baudrate Switch to x (0=125k,1=250k,2=500k)" NEWLINE);
  }
  else if (0 == strcmp(szBuffer, "reconfig"))
  {
    /* Start a reconfiguration of the stack */
    AppDNS_ConfigureStack(ptDnsRsc, 0);
  }
  else if (0 == strcmp(szBuffer, "+"))
  {
    /* Simulate address swich to be rotated UP*/
    bNodeIdSwitchValue = AppDNS_GetNodeIdValue(ptDnsRsc);
    if (bNodeIdSwitchValue >= 99) {
      bNodeIdSwitchValue = 0;
    } else {
      bNodeIdSwitchValue += 1;
    }
    AppDNS_SetNodeIdValue(ptDnsRsc, bNodeIdSwitchValue);
  }
  else if (0 == strcmp(szBuffer, "-"))
  {
    bNodeIdSwitchValue = AppDNS_GetNodeIdValue(ptDnsRsc);
    /* Simulate address swich to be rotated DOWN*/
    if (bNodeIdSwitchValue == 0 || bNodeIdSwitchValue > 99) {
      bNodeIdSwitchValue = 99;
    } else {
      bNodeIdSwitchValue -= 1;
    }
    AppDNS_SetNodeIdValue(ptDnsRsc, bNodeIdSwitchValue);
  }
  else if (0 == strcmp(szArg1, "n"))
  {
    bNodeIdSwitchValue = 0xff;
    /* Set node ID to dedicated value */
    if ( (szArg2[0]<48) || (szArg2[0]>57) )
    {
      /* invalid first digit */
    } else if (szArg2[1] == 0x00)
    {
      bNodeIdSwitchValue = szArg2[0] - 48;
    }else if ((szArg2[1] < 48) || (szArg2[1] > 57))
    {
      /* invalid second digit */
    }else if (szArg2[2] == 0x00)
    {
      bNodeIdSwitchValue = (szArg2[0]-48)*10;
      bNodeIdSwitchValue += (szArg2[1] - 48);
    }

    if (bNodeIdSwitchValue != 0xff){
      AppDNS_SetNodeIdValue(ptDnsRsc, bNodeIdSwitchValue);
    }else {
      DNS_PRINTF(" Invalid NodeID Value" NEWLINE);
    }
  }
  else if (0 == strcmp(szArg1, "r"))
  {
    bBaudSwitchValue = 0xff;
    /* Set baudrate to a dedicated value */
    if ((szArg2[0] < 48) || (szArg2[0] > 57))
    {
      /* invalid first digit */
    } else if (szArg2[1] == 0x00)
    {
      bBaudSwitchValue = szArg2[0] - 48;
    }

    if(bBaudSwitchValue != 0xff){
      AppDNS_SetBaudRateValue(ptDnsRsc, bBaudSwitchValue);
    }else{
      DNS_PRINTF(" Invalid Baudrate Value" NEWLINE);
    }
  }
  else
  {
    DNS_PRINTF(" 'help' - list commands" NEWLINE);
  }

  return 0;
}


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

#ifdef DNS_HOST_APP_SETS_SERIAL_NUMBER
      /* optional when initial DDP state is passive: set additional (OEM) DDP base device parameters: serial number */
      ulRet = AppDNS_DDP_SetSerialNumber(ptDnsRsc);
      if( ulRet != CIFX_NO_ERROR )
      {
        return (int)ulRet;
      }

      ulRet = AppDNS_DDP_Activate(ptDnsRsc);
      if( ulRet != CIFX_NO_ERROR )
      {
        return (int)ulRet;
      }
#endif

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
#else
    #error Application registration is switched off! Registration is essential for the DNS Extended Example.
#endif

#ifdef DNS_HOST_APP_STORES_REMANENT_DATA
      /* Set non-volatile BLOB */
      ulRet = AppDNS_SetRemanentData( ptAppData );
      if(CIFX_NO_ERROR != ulRet)
        return (int)ulRet;
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


/***************************************************************************************
*! Protocol specific packet handler.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
****************************************************************************************/
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

