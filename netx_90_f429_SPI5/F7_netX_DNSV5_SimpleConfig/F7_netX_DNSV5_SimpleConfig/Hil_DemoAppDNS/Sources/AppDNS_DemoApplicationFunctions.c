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
#include <stdio.h>

/* DeviceNet stack includes */
#include "DNS_Includes.h"

/* DeviceNet example includes */
//#include "AppDNS_DemoApplication_Config.h"
#include "Hil_ProductCodeCip.h"
#include "AppDNS_DemoApplication.h"

/* Generic example application framework includes */
#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"
#include "App_SystemPackets.h"
//#include "App_EventHandler.h"
//#include "hostAbstractionLayer.h"

/* Common Hilscher includes */
#include "Hil_ApplicationCmd.h"
#include "Hil_Results.h"
#include "Hil_Packet.h"
#include "Hil_DualPortMemory.h"
#include "Hil_SystemCmd.h"


/**************************************************************************************************/
/* CIP Identity Object - Configuration                                                            */
/**************************************************************************************************/
#define VENDOR_ID                            CIP_VENDORID_HILSCHER
#define DEVICE_TYPE_COMMUNICATION_ADAPTER    0x0C       /* CIP Profile - "Communication Adapter" */
/* Set the product code to Hilscher generic DNS product code */
#define PRODUCT_CODE                         PRODUCT_CODE_DNS_NETX
#define MAJOR_REVISION                       5
#define MINOR_REVISION                       2
char    abProductName[]                      = "DNS_V5_SIMPLE_CONFIG_DEMO";


/**************************************************************************************************/
/* Default network parameter                                                                      */
/**************************************************************************************************/
#define DEFAULT_NODE_ADDRESS   1
#define DEFAULT_NODE_BAUDRATE  DNS_BAUDRATE_125kB


/**************************************************************************************************/
/* Default Assembly Object - Configuration                                                        */
/**************************************************************************************************/
#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE        0x64 /* 100 */
#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_SIZE   sizeof(APP_PROCESS_DATA_INPUT_T)    /* 6 Byte */

#define DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE        0x65 /* 101 */
#define DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE_SIZE   sizeof(APP_PROCESS_DATA_OUTPUT_T) /* 10 Byte */


/**************************************************************************************************/
/* DNS Application specific variables.                                                           * /
 **************************************************************************************************/
APP_DNS_DATA_T g_tAppDnsData = { DEFAULT_NODE_ADDRESS, DEFAULT_NODE_BAUDRATE };


/***************************************************************************************************
*! Function to send the basic configuration packet to the DeviceNet Slave Stack
*   \param ptAppData   pointer to APP_DATA_T structure
 **************************************************************************************************/
uint32_t AppDNS_SetConfiguration(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = CIFX_NO_ERROR;
  DNS_PACKET_SET_CONFIGURATION_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptAppData);

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


  ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);
  return ulRet;
}


/***************************************************************************************************
*! Send the common "Channel Init" packet to apply configuration parameter to the stack.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_ChannelInit(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = 0;
  CIFX_PACKET* ptReq = AppDNS_GlobalPacket_Init(ptAppData);

  App_SysPkt_AssembleChannelInitReq(ptReq);

  ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);
  return ulRet;
}


/***************************************************************************************************
*! Common command to release the network communication.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_StartCommunication(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = 0;
  CIFX_PACKET* ptReq = AppDNS_GlobalPacket_Init(ptAppData);

  App_SysPkt_AssembleStartStopCommReq(ptReq, true);

  ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);

  return ulRet;
}


/***************************************************************************************************
*! Central configuration function to configure the DeviceNet Slave stack.
*  The three required steps are:
*  1) Send the SetConfig packet with stack parameter.
*  2) Send the common ChannelInit packet to activate the provided configuration.
*  3) Send the StartCommunication packet to release the network communication.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_ConfigureStack(APP_DATA_T* ptAppData, uint32_t ulBusOnDelay)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  /* Mandatory: Submit a new configuration*/
  ulRet = AppDNS_SetConfiguration(ptAppData);
  if (ulRet != 0)
    return ulRet;

  /* Mandatory: Activate the new configuration */
  ulRet = AppDNS_ChannelInit(ptAppData);
  if (ulRet != 0)
    return ulRet;

  /* Mandatory: Release communication */
  ulRet = AppDNS_StartCommunication(ptAppData);

  return ulRet;
}


/***************************************************************************************************
*! Application Packet Handler function.
*   This function is called from the example framework, when a packet frome the DeviceNet Slave stack
*   is received.
*
*  Note:
*    Per default the DNS Simple Config Example project is not registered to to stack to receive any
*    indication packets. To receive indication packets set the define "DNS_HOST_APP_REGISTRATION"
*    in the file "AppDNS_DemoApplication_Config.h" or in your compiler definitions.
*    If the application is registered, all indications have be handled correctly, especially the
*    Reset Indication.
*
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
bool AppDNS_PacketHandler_callback( CIFX_PACKET* ptPacket,
                           void*        pvUserData )
{
  bool        fPacketHandled = true;
  APP_DATA_T* ptAppData = (APP_DATA_T*)pvUserData;

  /* Check if the received packet is placed in the channel related global packet buffer.
    If not we reject it. The entire example project rely's on that all incomming packts
    are placed into this channel related global packet buffer */
  if( ptPacket != &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket )
  {
    /* The packet is not in our channel related packet buffer */
    PRINTF("Unexpected packet resource received!!!" NEWLINE);
    return false;
  }

  switch( ptPacket->tHeader.ulCmd )
  {
#ifdef DNS_HOST_APP_REGISTRATION
 #error Error no indication packet handling implemented!
 /* Implement here the Indication packet handling */
#endif
    default:
    {
      if ((ptPacket->tHeader.ulCmd & 0x1) == 0)
      {
        PRINTF("Warning: Disregarded indication packet received!" NEWLINE);

        ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
        ptPacket->tHeader.ulCmd |= 0x01; /* Make it a response */
        /* Send the response packet */
        AppDNS_GlobalPacket_Send(ptAppData);
      }
      else
      {
        PRINTF("Warning: Disregarded confirmation packet received!" NEWLINE);
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
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_GlobalPacket_SendReceive(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = 0;
  ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;

  ulRet = Pkt_SendReceivePacket(ptAppData,
                                DNS_DEMO_CHANNEL_INDEX,
                                &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket,
                                TXRX_TIMEOUT);
  if (ulRet != CIFX_NO_ERROR)
  {
    return ulRet;
  }

  ulRet = ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
  return ulRet;
}

/***************************************************************************************************
*! Send a packet.
*   Help function for all modules of the DNS example project to send a packet from the global packet
*   buffer. The calling funktion must prepare the command to be send in global packet buffer.
*   Usally it can be used to send confirmation packet to previosly received indication packets.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_GlobalPacket_Send(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = 0;
  ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;

  ulRet = Pkt_SendPacket(ptAppData,
                         DNS_DEMO_CHANNEL_INDEX,
                         &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket,
                         TX_TIMEOUT);
  if (ulRet != CIFX_NO_ERROR)
  {
    PRINTF("Sending DNS_RESET_RES failed with error code 0x%08x" NEWLINE, (unsigned int)ulRet);
  }
  return ulRet;
}

/***************************************************************************************************
*! Helpfunktion to preset the header of the global packet buffer with zero, befor using it.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
void* AppDNS_GlobalPacket_Init(APP_DATA_T* ptAppData)
{
  void* pvPkt = (void*)&ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket;
  memset(pvPkt, 0x00, sizeof(ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket.tHeader));
  return pvPkt;
}

/***************************************************************************************************
*! Helpfunktion to return the pointer of the global packet buffer of the example project.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
void* AppDNS_GlobalPacket_Get(APP_DATA_T* ptAppData)
{
  void* pvPkt = (void*)&ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket;
  return pvPkt;
}

