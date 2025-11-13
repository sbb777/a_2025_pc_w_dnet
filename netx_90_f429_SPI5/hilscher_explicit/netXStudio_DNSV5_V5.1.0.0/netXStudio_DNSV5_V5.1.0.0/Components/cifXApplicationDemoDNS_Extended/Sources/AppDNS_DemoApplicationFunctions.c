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
/* Includes */

/* DeviceNet stack includes */
#include "DNS_Includes.h"

/* DeviceNet example includes */
#include "AppDNS_DemoApplication_Config.h"
#include "AppDNS_DemoApplication.h"

/* Generic example application framework includes */
#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"
#include "App_SystemPackets.h"


extern void OS_Sleep(uint32_t ulSleepTimeMs);

/**************************************************************************************************/
/* CIP Identity Object - Configuration                                                            */
/**************************************************************************************************/
#define VENDOR_ID                            CIP_VENDORID_HILSCHER
#define DEVICE_TYPE_COMMUNICATION_ADAPTER    0x0C       /* CIP Profile - "Communication Adapter" */
/* Set the product code to Hilscher generic DNS product code */
#define PRODUCT_CODE                         PRODUCT_CODE_DNS_NETX
#define MAJOR_REVISION                       5
#define MINOR_REVISION                       4
char    abProductName[]                      = "DNS_V5_EXTENDED_CONFIG_DEMO";


/**************************************************************************************************/
/* Default network parameter                                                                      */
/**************************************************************************************************/
#define DEFAULT_NODE_ADDRESS   62
#define DEFAULT_NODE_BAUDRATE  DNS_BAUDRATE_125kB


/**************************************************************************************************/
/* Default Assembly Object - Configuration                                                        */
/**************************************************************************************************/
#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE        0x64 /* 100 */
#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_SIZE   DNS_PROCESS_DATA_INPUT_SIZE   /* 6 Byte */

#define DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE        0x65 /* 101 */
#define DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE_SIZE   DNS_PROCESS_DATA_OUTPUT_SIZE /* 10 Byte */


/**************************************************************************************************/
/* DNS Application specific variables.                                                           * /
***************************************************************************************************/
APP_DNS_DATA_T g_tAppDnsData = { DEFAULT_NODE_ADDRESS, DEFAULT_NODE_BAUDRATE };


/***************************************************************************************************
*! Function to send the basic configuration packet to the DeviceNet Slave Stack
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
uint32_t AppDNS_SetConfiguration(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = CIFX_NO_ERROR;
  DNS_PACKET_SET_CONFIGURATION_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);

  DNS_CONFIGURATION_V1_T *ptCfg = &ptReq->tData.unCfg.tV1;

  /* Set the packet command, length and DNS configuration version */
  ptReq->tHead.ulCmd        = DNS_CMD_SET_CONFIGURATION_REQ;
  ptReq->tHead.ulLen        = DNS_SET_CONFIGURATION_V1_REQ_SIZE;
  ptReq->tHead.ulSta        = 0;
  ptReq->tData.ulVersion    = DNS_CONFIGURATION_V1;

  /* Set the slave related parameters */
  memset(ptCfg, 0x00, sizeof(DNS_CONFIGURATION_V1_T));
  ptCfg->ulSystemFlags      = DNS_SYS_FLG_MANUAL_START;
  ptCfg->ulWdgTime          = 0;

  /* Set network parameter */
  ptCfg->ulNodeId = g_tAppDnsData.bNodeIdValue;
  ptCfg->ulBaudrate = g_tAppDnsData.bBaudRateValue;

  /* If the host application provides the node id and baud rate via switches */
  #ifdef DNS_HOST_APP_SWITCH_SUPPORT
   ptCfg->ulSystemFlags |= DNS_SYS_FLG_ADR_SW_ENABLE;
   ptCfg->ulSystemFlags |= DNS_SYS_FLG_BAUD_SW_ENABLE;
  #endif

  ptCfg->ulConfigFlags      = 0;
  ptCfg->ulObjectFlags      = 0;

  /* Set Identity Data */
  ptCfg->usVendorId         = VENDOR_ID;
  ptCfg->usDeviceType       = DEVICE_TYPE_COMMUNICATION_ADAPTER;
  ptCfg->usProductCode      = PRODUCT_CODE;
  ptCfg->bMajorRev          = MAJOR_REVISION;
  ptCfg->bMinorRev          = MINOR_REVISION;
  ptCfg->bProductNameLen    = sizeof(abProductName)-1;
  memcpy(&ptCfg->abProductName[0], &abProductName[0], ptCfg->bProductNameLen);

  /* Set Process Data */
  ptCfg->ulProduceAsInstance  = DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE;
  ptCfg->ulProduceAsSize      = DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE_SIZE;
  ptCfg->ulConsumeAsInstance  = DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE;
  ptCfg->ulConsumeAsSize      = DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_SIZE;


  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  return ulRet;
}


/***************************************************************************************************
*! Send the common "Channel Init" packet to apply configuration parameter to the stack.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
uint32_t AppDNS_ChannelInit(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = 0;
  CIFX_PACKET* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);

  App_SysPkt_AssembleChannelInitReq(ptReq);

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  return ulRet;
}


/***************************************************************************************************
*! Help function for other modules of the DNS example project to send a CIP service request to the
*   local CIP object library of the DNS stack.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
uint32_t AppDNS_CipService(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
                           uint32_t                       ulService,
                           uint32_t                       ulClass,
                           uint32_t                       ulInstance,
                           uint32_t                       ulAttribute,
                           uint8_t*                       pbData,
                           uint32_t                       ulSize)
{
  uint32_t ulRet = CIFX_NO_ERROR;
  DNS_PACKET_CIP_SERVICE_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);

  /* prepare packet with service data */
  ptReq->tData.ulService   = ulService;
  ptReq->tData.ulClass     = ulClass;
  ptReq->tData.ulInstance  = ulInstance;
  ptReq->tData.ulAttribute = ulAttribute;
  ptReq->tData.ulMember    = 0;

  memcpy( &ptReq->tData.abData[0], pbData, ulSize );

  /* Issue CIP Service Request */
  ptReq->tHead.ulCmd  = DNS_CMD_CIP_SERVICE_REQ;
  ptReq->tHead.ulLen  = DNS_CIP_SERVICE_REQ_SIZE + ulSize;

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  return ulRet;
}


/***************************************************************************************************
*! Common delete configuration packet. This should be send, when the remanten data shall be deleted
*  which are hold by the DNS stack.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
uint32_t AppDNS_DeleteConfig(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = 0;
  CIFX_PACKET* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);

  ptReq->tHeader.ulCmd  = HIL_DELETE_CONFIG_REQ;
  ptReq->tHeader.ulLen  = 0;

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  return ulRet;
}


/***************************************************************************************************
*! Common command to release the network communication.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
uint32_t AppDNS_StartCommunication(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = 0;
  CIFX_PACKET* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);

  App_SysPkt_AssembleStartStopCommReq(ptReq, true);

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);

  return ulRet;
}


/***************************************************************************************************
*! Central configuration function to configure the DeviceNet Slave stack.
*  The three required steps are:
*  1) Send the SetConfig packet with stack parameter.
*  2) Send the common ChannelInit packet to activate the provided configuration.
*  3) Send the StartCommunication packet to release the network communication.
*
*  Additionaly other feature that shall be supported can be enabled or disable by defines in
*  the file "AppDNS_DemoApplication_Config.h". According the defines the configuration function will
*  send additional registration packets between step 2) ans 3).
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
uint32_t AppDNS_ConfigureStack(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc, uint32_t ulBusOnDelay)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  /* Mandatory: Submit a new configuration*/
  ulRet = AppDNS_SetConfiguration(ptDnsRsc);
  if (ulRet != 0)
    return ulRet;

  /* Mandatory: Activate the new configuration */
  ulRet = AppDNS_ChannelInit(ptDnsRsc);
  if (ulRet != 0)
    return ulRet;

#ifdef DNS_HOST_APP_REGISTER_USER_OBJECTS
  /* Optional: register additional user objects if configured */
  ulRet = AppDNS_UserObject_Registration(ptDnsRsc);
  if (ulRet != 0)
    return ulRet;
#endif

#ifdef DNS_HOST_APP_REGISTER_ADDITIONAL_ASSEMBLY_OBJECTS
  /* Optional: Register additional assemblies if configured */
  ulRet = AppDNS_AsObject_Registration(ptDnsRsc);
  if (ulRet != 0)
    return ulRet;
#endif

#ifdef DNS_HOST_APP_REGISTER_MNS_OBJECT
  /* Optional: Register Module Network Status indications */
  ulRet = AppDNS_MnsObject_Registration(ptDnsRsc);
  if (ulRet != 0)
    return ulRet;
#endif

#ifdef DNS_HOST_APP_REGISTER_CONNECTION_OBJECT
  /* Optional: Register EPR indications*/
  ulRet = AppDNS_ConnObject_Registration(ptDnsRsc);
  if (ulRet != 0)
    return ulRet;
#endif

  if (ulBusOnDelay)
  {
    OS_Sleep(ulBusOnDelay);
  }

  /* Mandatory: Release communication */
  ulRet = AppDNS_StartCommunication(ptDnsRsc);

  return ulRet;
}


/***************************************************************************************************
*! Reset Handler Funktion.
*  If the host application is registered to the DeviceNet Slave stack to receive indication packets
*  from the stack, then the host application will also receive reset indications from the stack
*  if a reset is required. The host applikation is resposible to perform the reset.
*  Following reason may cause a reset indication:
*  - DNS_RESET_REASON_ID_OBJECT_NET_RESET:
*    A DeviceNet master or other comissioning tool has send a reset service to identity object.
*    The host application is resposible to perform the reset according the reset type.
*  - DNS_RESET_REASON_DN_OBJECT_MACID_SET:
*    A DeviceNet master or other comissioning tool has set the node address remotly to the
*    DeviceNet Object. To apply the new node address the host application has to perform a reset.
*  - DNS_RESET_REASON_NP_RESUME_SWITCH_CHANGE
*    This event appears, when the rotary switches for node id or baudrate are used and enabled.
*    If the switch position has been changed in online state and the 24V network power is resumed,
*    then a reset is required to apply the new switch values.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
static void AppDNS_Reset_Indication(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{

  uint32_t ulRet = CIFX_NO_ERROR;


  DNS_PACKET_RESET_IND_T*  ptInd = (DNS_PACKET_RESET_IND_T*) (&ptDnsRsc->tPacket);
  DNS_PACKET_RESET_RES_T*  ptRes = (DNS_PACKET_RESET_RES_T*) (&ptDnsRsc->tPacket);

  bool fPerformReset = true;
  bool fPerformDeleteConfig = false;

  PRINTF("Reset Indication (Reason: %i) (Type: %i)" NEWLINE, (int)ptInd->tData.ulReason, (int)ptInd->tData.ulType);

  if (DNS_RESET_REASON_ID_OBJECT_NET_RESET == ptInd->tData.ulReason)
  {
    /* A reset via network to the identity object was received */
    if(DNS_RESET_TYPE_POWER_CYCLE == ptInd->tData.ulType)
    {
      /* Perform a reset as close apossible to a power cycle */
    }
    else
    if (DNS_RESET_TYPE_FACTORY_DEFAULT == ptInd->tData.ulType)
    {
      /* Perform a reset close as apossible to a factory default,
        including remanent stored communication data like
        node id or baudrate set via network, as well as remanent
        data of the application process.
      */
      fPerformDeleteConfig = true;
    }
    else
    if (DNS_RESET_TYPE_FACTORY_WO_COMM_PRM == ptInd->tData.ulType)
    {
      /* Type 2 /User Type. The user is free to select the reset procedure.
         To fulfill the Conformance Test requirements ulGRC must be set to CIP_GRC_INVALID_PARAMETER.
      */
      fPerformReset = false;
    }
    else
    {
      /* Unknown reset type*/
      fPerformReset = false;
    }
  }
  else
  if(DNS_RESET_REASON_DN_OBJECT_MACID_SET == ptInd->tData.ulReason)
  {
    /* Perform a reset because a set mac id service via network
       was received to change the node address. This is required a
       to go online with the new node address.
     */
  }
  else
  if (DNS_RESET_REASON_NP_RESUME_SWITCH_CHANGE == ptInd->tData.ulReason)
  {
    /* Perform a reset because the vaule of a rotary swich (mac or baudrate)
       was changed at runtime and the 24V network power was resumed.
       According the spec. this is a trigger to go online with the new
       values from the switches.
     */
  }
  else
  {
    /* Unknown reset type*/
    fPerformReset = false;
  }


  if (true == fPerformReset)
  {
    ptInd->tHead.ulLen = DNS_RESET_RES_SIZE;
    ptInd->tHead.ulSta = SUCCESS_HIL_OK;
    ptInd->tHead.ulCmd |= 0x01; /* Make it a response */
    ptRes->tData.ulGRC = 0;
    ptRes->tData.ulERC = 0;
  }
  else
  {
    ptInd->tHead.ulLen = DNS_RESET_RES_SIZE;
    ptInd->tHead.ulSta = ERR_HIL_FAIL;
    ptInd->tHead.ulCmd |= 0x01; /* Make it a response */
    ptRes->tData.ulGRC = CIP_GRC_INVALID_PARAMETER;
    ptRes->tData.ulERC = 0;
  }

  /* Send the reset response packet*/
  AppDNS_GlobalPacket_Send(ptDnsRsc);

  if (true == fPerformReset)
  {
    if (true == fPerformDeleteConfig)
    {
      if (CIFX_NO_ERROR != (ulRet = AppDNS_DeleteConfig(ptDnsRsc)))
      {
        DNS_PRINTF("HIL_CONFIG_DELETE_REQ failed with error code 0x%08x" NEWLINE, (unsigned int)ulRet);
      }
    }

    /* Reconfigure the stack */
    AppDNS_ConfigureStack(ptDnsRsc, 100);
  }
}


/***************************************************************************************************
*! Cip Service Handler function.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/

static void AppDNS_CipService_Indication(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  int g_ulNumExplMsgReceived = 0;
  DNS_PACKET_CIP_SERVICE_IND_T *ptInd = AppDNS_GlobalPacket_Get(ptDnsRsc);
  DNS_PACKET_CIP_SERVICE_RES_T *ptRes = (DNS_PACKET_CIP_SERVICE_RES_T*)ptInd;
  uint32_t ulResDataSize = 0;

  g_ulNumExplMsgReceived += 1;
  PRINTF("Explicit Message Received (Cnt:%d)" NEWLINE, g_ulNumExplMsgReceived);
  PRINTF("Service %x, Class %x, Instance %d, Attribute %d" NEWLINE, (unsigned int)ptInd->tData.ulService,
                                                                    (unsigned int)ptInd->tData.ulClass,
                                                                    (unsigned int)ptInd->tData.ulInstance,
                                                                    (unsigned int)ptInd->tData.ulAttribute );

  switch( ptInd->tData.ulClass )
  {
    case CIPHIL_CLASS_MN_STATUS:
    {
      /* "Module Network Status" Class indication recieved */
      AppDNS_MnsObject_Indication(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
    }
    break;

    case CIP_CLASS_CONNECTION:
    {
      /* Connection Class indication recieved */
      AppDNS_ConnObject_Indication(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
    }
    break;

    default:
    {
      /* Check if the CIP request is directed to any application specific user objects */
      if (true == AppDNS_UserObject_IsRegistered(ptDnsRsc, ptInd->tData.ulClass, ptInd->tData.ulInstance))
      {
        AppDNS_UserObject_Indication(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
      }
      else
      {
        ptRes->tData.ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
        ptRes->tData.ulERC = 0;
      }
    }
    break;
  }

  ptRes->tHead.ulLen  = DNS_CIP_SERVICE_CNF_SIZE + ulResDataSize;
  ptRes->tHead.ulCmd |= 0x01; /* Make it a response */
  if (ptRes->tData.ulGRC == CIP_GRC_SUCCESS){
    ptRes->tHead.ulSta = SUCCESS_HIL_OK;
  }else{
    ptRes->tHead.ulSta = ERR_HIL_FAIL;
  }

  /* Send the CIP service response */
  AppDNS_GlobalPacket_Send(ptDnsRsc);

  return;
}


/***************************************************************************************************
*! DeviceNet Slave Stack specific Packet Handler Callback function.
*   This function is called from the example framework, when a packet from the DeviceNet Slave stack
*   is received.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
bool AppDNS_PacketHandler(void* pvUserData, CIFX_PACKET* ptPacket)
{
  bool        fPacketHandled = true;

  APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc = (APP_DNS_CHANNEL_HANDLER_RSC_T*)pvUserData;

  /* Check if the received packet is placed in the channel related global packet buffer.
    If not we reject it. The entire example project rely's on that all incomming packts
    are placed into this channel related global packet buffer */
  if( ptPacket != &ptDnsRsc->tPacket )
  {
    /* The packet is not in our channel related packet buffer */
    DNS_PRINTF("Unexpected packet resource received!!!" NEWLINE);
    return false;
  }

  switch( ptPacket->tHeader.ulCmd )
  {
    case DNS_CMD_RESET_IND:
    {
      AppDNS_Reset_Indication(ptDnsRsc);
    }
    break;

    case DNS_CMD_CIP_SERVICE_IND:
    {
      AppDNS_CipService_Indication(ptDnsRsc);
    }
    break;

    case HIL_STORE_REMANENT_DATA_IND:
    {
      AppDNS_StoreRemanent_Indication(ptDnsRsc);
      break;
    }
    break;

    default:
    {
      if ((ptPacket->tHeader.ulCmd & 0x1) == 0)
      {
        DNS_PRINTF("Warning: Disregarded indication packet received!" NEWLINE);

        ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
        ptPacket->tHeader.ulCmd |= 0x01; /* Make it a response */
        /* Send the response packet */
        AppDNS_GlobalPacket_Send(ptDnsRsc);
      }
      else
      {
        DNS_PRINTF("Warning: Disregarded confirmation packet received!" NEWLINE);
      }
    }
    break;
  }

  return fPacketHandled;
}


/***************************************************************************************************
*! Send and Receive a packet.
*  Help function for all modules of the DNS example project to send a request to stack and receive
*  a corresponding confirmation packet. This funktion works on the global packet buffer. The calling
*  funktion must prepare the command to be send in global packet buffer. The confirmation will be
*  also received in the global packet buffer.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
uint32_t AppDNS_GlobalPacket_SendReceive(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = 0;

  ptDnsRsc->tPacket.tHeader.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;

  ulRet = Pkt_SendReceivePacket(ptDnsRsc->hPktIfRsc,
                                &ptDnsRsc->tPacket,
                                TXRX_TIMEOUT);

  if (ulRet != CIFX_NO_ERROR)
  {
    return ulRet;
  }

  ulRet = ptDnsRsc->tPacket.tHeader.ulState;

  return ulRet;
}


/***************************************************************************************************
*! Send a packet.
*   Help function for all modules of the DNS example project to send a packet from the global packet
*   buffer. The calling funktion must prepare the command to be send in global packet buffer.
*   Usally it can be used to send confirmation packet to previosly received indication packets.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
uint32_t AppDNS_GlobalPacket_Send(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = 0;

  ptDnsRsc->tPacket.tHeader.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;

  (void)Pkt_SendPacket(ptDnsRsc->hPktIfRsc,
                       (CIFX_PACKET*)&ptDnsRsc->tPacket,
                       TX_TIMEOUT);

  if (ulRet != CIFX_NO_ERROR)
  {
    PRINTF("Sending DNS_RESET_RES failed with error code 0x%08x" NEWLINE, (unsigned int)ulRet);
  }
  return ulRet;
}


/***************************************************************************************************
*! Helpfunktion to preset the header of the global packet buffer with zero, befor using it.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
void* AppDNS_GlobalPacket_Init(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  void* pvPkt = (void*)&ptDnsRsc->tPacket;
  memset(pvPkt, 0x00, sizeof(ptDnsRsc->tPacket.tHeader));

  return pvPkt;
}


/***************************************************************************************************
*! Helpfunktion to return the pointer of the global packet buffer of the example project.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
***************************************************************************************************/
void* AppDNS_GlobalPacket_Get(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{

  void* pvPkt = (void*)&ptDnsRsc->tPacket;
  return pvPkt;
}


/***************************************************************************************************
*! Helpfunktion that can be called from a switch handler to set the NodeId swich value.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
*   \param bNodeIdSwitchValue  new switch value
***************************************************************************************************/
void AppDNS_SetNodeIdValue(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc, uint8_t bNodeIdSwitchValue)
{
  g_tAppDnsData.bNodeIdValue = bNodeIdSwitchValue;

  AppDNS_CipService(ptDnsRsc,
                    CIP_SERVICE_SET_ATTRIBUTE_SINGLE,
                    CIP_CLASS_DEVICENET, 1,
                    CIP_CLASS_DEVICENET_ATT_MACID_SWITCH_VALUE,
                    &g_tAppDnsData.bNodeIdValue, 1);

  return;
}


/***************************************************************************************************
*! Helpfunktion that can be called from a switch handler to retrieve the currentr NodeId value.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
*   \reurn bNodeIdSwitchValue
***************************************************************************************************/
uint8_t AppDNS_GetNodeIdValue(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  return g_tAppDnsData.bNodeIdValue;
}


/***************************************************************************************************
*! Helpfunktion that can be called from a switch handler to set the NodeId swicth value.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
*   \param bBaudSwitchValue
***************************************************************************************************/
void AppDNS_SetBaudRateValue(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc, uint8_t bBaudSwitchValue)
{
  g_tAppDnsData.bBaudRateValue = bBaudSwitchValue;

  AppDNS_CipService(ptDnsRsc,
                    CIP_SERVICE_SET_ATTRIBUTE_SINGLE,
                    CIP_CLASS_DEVICENET, 1,
                    CIP_CLASS_DEVICENET_ATT_BAUDRATE_SWITCH_VALUE,
                    &g_tAppDnsData.bBaudRateValue, 1);

  return;
}


/***************************************************************************************************
*! Helpfunktion that can be called from a switch handler to the retrieve the last set baud swicth value.
*   \param ptDnsRsc   pointer to APP_DNS_CHANNEL_HANDLER_RSC_T structure
*   \return bBaudSwitchValue
***************************************************************************************************/
uint8_t AppDNS_GetBaudRateValue(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  return g_tAppDnsData.bBaudRateValue;
}

