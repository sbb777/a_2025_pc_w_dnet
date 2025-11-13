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

//#include "AppDNS_DemoApplication_Config.h"
#include "AppDNS_DemoApplication.h"
#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"
#include "App_SystemPackets.h"
//#include "hostAbstractionLayer.h"
#include "Hil_ApplicationCmd.h"
#include "Hil_Results.h"
#include "Hil_Packet.h"
#include "Hil_DualPortMemory.h"
//#include "App_EventHandler.h"

/***************************************************************************************
* Firmware verification.
****************************************************************************************/
PROTOCOL_IDENTIFICATION_T g_tProtocolIdentification =
{
  .szFirmwareName = "DeviceNet Slave",
  .usFirmwareMajor = 5,
  .usFirmwareMinor = 2,
};


/***************************************************************************************
* Lookup command table for debug purposes.
****************************************************************************************/
#define STR(tok) #tok

static struct
{
  char* szCommandName;
  uint32_t ulCommandCode;
} s_atCommandNameLookupTable[] =
{
  /* Protocol stack independent packets */
  { STR(HIL_CHANNEL_INIT_REQ), HIL_CHANNEL_INIT_REQ },
  { STR(HIL_CHANNEL_INIT_CNF), HIL_CHANNEL_INIT_CNF },

  { STR(HIL_REGISTER_APP_REQ), HIL_REGISTER_APP_REQ },
  { STR(HIL_REGISTER_APP_CNF), HIL_REGISTER_APP_CNF },

  { STR(HIL_UNREGISTER_APP_REQ), HIL_UNREGISTER_APP_REQ },
  { STR(HIL_UNREGISTER_APP_CNF), HIL_UNREGISTER_APP_CNF },

  { STR(HIL_START_STOP_COMM_REQ), HIL_START_STOP_COMM_REQ },
  { STR(HIL_START_STOP_COMM_CNF), HIL_START_STOP_COMM_CNF },

  { STR(HIL_LINK_STATUS_CHANGE_IND), HIL_LINK_STATUS_CHANGE_IND },
  { STR(HIL_LINK_STATUS_CHANGE_RES), HIL_LINK_STATUS_CHANGE_RES },

  { STR(  HIL_SET_REMANENT_DATA_REQ), HIL_SET_REMANENT_DATA_REQ },
  { STR(  HIL_SET_REMANENT_DATA_CNF), HIL_SET_REMANENT_DATA_CNF },

  { STR(HIL_STORE_REMANENT_DATA_IND), HIL_STORE_REMANENT_DATA_IND },
  { STR(HIL_STORE_REMANENT_DATA_RES), HIL_STORE_REMANENT_DATA_RES },

  /* Protocol stack specific packets */
  { STR(DNS_CMD_SET_CONFIGURATION_REQ), DNS_CMD_SET_CONFIGURATION_REQ },
  { STR(DNS_CMD_SET_CONFIGURATION_CNF), DNS_CMD_SET_CONFIGURATION_CNF },

  { STR(DNS_CMD_REGISTER_CLASS_REQ), DNS_CMD_REGISTER_CLASS_REQ },
  { STR(DNS_CMD_REGISTER_CLASS_CNF), DNS_CMD_REGISTER_CLASS_CNF },

  { STR(DNS_CMD_UNREGISTER_CLASS_REQ), DNS_CMD_UNREGISTER_CLASS_REQ },
  { STR(DNS_CMD_UNREGISTER_CLASS_CNF), DNS_CMD_UNREGISTER_CLASS_CNF },

  { STR(DNS_CMD_CIP_SERVICE_REQ), DNS_CMD_CIP_SERVICE_REQ },
  { STR(DNS_CMD_CIP_SERVICE_CNF), DNS_CMD_CIP_SERVICE_CNF },

  { STR(DNS_CMD_RESET_IND), DNS_CMD_RESET_IND },
  { STR(DNS_CMD_RESET_RES), DNS_CMD_RESET_RES },

  { STR(DNS_CMD_CIP_SERVICE_IND), DNS_CMD_CIP_SERVICE_IND },
  { STR(DNS_CMD_CIP_SERVICE_RES), DNS_CMD_CIP_SERVICE_RES },

  { STR(HIL_DELETE_CONFIG_REQ), HIL_DELETE_CONFIG_REQ },
  { STR(HIL_DELETE_CONFIG_CNF), HIL_DELETE_CONFIG_CNF },

  { STR(DNS_CMD_CREATE_ASSEMBLY_REQ), DNS_CMD_CREATE_ASSEMBLY_REQ },
  { STR(DNS_CMD_CREATE_ASSEMBLY_CNF), DNS_CMD_CREATE_ASSEMBLY_CNF },

  /* This always must be the last entry in this table */
  { "Unknown command", 0xffffffff },
};

/***************************************************************************************
*! Protocol specific lookup table to print out command codes as string for debug purposes.
*   \param ptAppData   pointer to APP_DATA_T structure
****************************************************************************************/
extern char* LookupCommandCode(uint32_t ulCmd)
{
  int i;
  for (i = 0; i < sizeof(s_atCommandNameLookupTable) / sizeof(s_atCommandNameLookupTable[0]); i++)
  {
    if (s_atCommandNameLookupTable[i].ulCommandCode == ulCmd)
    {
      return s_atCommandNameLookupTable[i].szCommandName;
    }
  }

  /* Command not in lookup table: Return last entry of table */
  return s_atCommandNameLookupTable[i - 1].szCommandName;
}

/***************************************************************************************
*! Protocol specific function to start the configuration process. It is called once,
*  at startup from the example application framework.
*   \param ptAppData   pointer to APP_DATA_T structure
****************************************************************************************/
static uint32_t Protocol_StartConfiguration_callback( APP_DATA_T* ptAppData )
{
  uint32_t ulRet = CIFX_NO_ERROR;

  if (!Pkt_Init(DNS_DEMO_CHANNEL_INDEX, 1))
  {
    return CIFX_DRV_INIT_ERROR;
  }

  /* Register handler for indication packets. */
  if( Pkt_RegisterIndicationHandler( DNS_DEMO_CHANNEL_INDEX, AppDNS_PacketHandler_callback, (void*)ptAppData ) )
  {

#ifdef DNS_HOST_APP_REGISTRATION
    /* Register application */
    App_SysPkt_AssembleRegisterAppReq( &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket );
    ulRet = Pkt_SendReceivePacket( ptAppData, DNS_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );
    if( ulRet != CIFX_NO_ERROR )
      return ulRet;
#endif


    /* Download the configuration */
    ulRet = AppDNS_ConfigureStack( ptAppData, 0 );
  }
  else
  {
    ulRet = CIFX_DRV_INIT_ERROR;
  }

  return ulRet;
}

/***************************************************************************************
*! Protocol specific packet handler.
*   \param ptAppData   pointer to APP_DATA_T structure
****************************************************************************************/
static uint32_t Protocol_PacketHandler_callback( APP_DATA_T* ptAppData )
{
  uint32_t ulRet = CIFX_NO_ERROR;

  ulRet = Pkt_CheckReceiveMailbox( ptAppData, DNS_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket );

  if( CIFX_DEV_GET_NO_PACKET == ulRet || CIFX_DEV_NOT_READY == ulRet )
  {
    /* Handle "no packet" and "stack not ready" as success. */
    ulRet = CIFX_NO_ERROR;
  }

  return ulRet;
}

/***************************************************************************************
*! Protocol specific application event handler. This function can be called from the
*  example application context to signal special application events to stack.
*  It is up to the stack capabilities to translate the application events to stack
*  specific diagnostic or alarm methods. It is just an option and there is no need
*  to adapt it. Since DeviceNet has not explicit alarm or diagnostic mechanism the events
*  are ignored.
*   \param ptAppData     pointer to APP_DATA_T structure
*   \param eEvent        event type
*   \param ulEventData   event specific data
****************************************************************************************/

/***************************************************************************************
*! Protocol specific terminal handler.
*   \param ptAppData   pointer to APP_DATA_T structure
*   \param szBuffer    termninal command
****************************************************************************************/


/* List of protocol specific handler which passed downward to the generic example framework */
PROTOCOL_DESCRIPTOR_T g_tRealtimeProtocolHandlers =
{
  .pfStartChannelConfiguration = Protocol_StartConfiguration_callback,
  .pfPacketHandler             = Protocol_PacketHandler_callback,
};
