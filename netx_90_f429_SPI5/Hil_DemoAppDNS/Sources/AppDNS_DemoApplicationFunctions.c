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

/* ⭐ 여기에 추가 ⭐ */
#include "DNS_packet_register_class.h"
#include "DNS_packet_cip_service.h"
#include "AppDNS_ExplicitMsg.h"
#include "App_VAT_Parameters.h"
#include "App_VAT_ExplicitHandler.h"  /* VAT Explicit Message Handler */
#include "App_VAT_UserObject.h"        /* VAT UserObject Registration and Handling */

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
/* CIP Identity Object - Configuration - VAT Adaptive Pressure Controller                         */
/**************************************************************************************************/
#define VENDOR_ID                            404         /* VAT Vakuumventile AG */
#define DEVICE_TYPE_PROCESS_CONTROL_VALVE    29          /* Process Control Valve */
/* Set the product code to VAT Adaptive Pressure Controller */
#define PRODUCT_CODE                         650         /* VAT Product Code */
#define MAJOR_REVISION                       2
#define MINOR_REVISION                       1
char    abProductName[]                      = "VAT Adaptive Pressure Controller";


/**************************************************************************************************/
/* Default network parameter                                                                      */
/**************************************************************************************************/
#define DEFAULT_NODE_ADDRESS   1
#define DEFAULT_NODE_BAUDRATE  DNS_BAUDRATE_125kB


/**************************************************************************************************/
/* Default Assembly Object - Configuration - VAT Assemblies                                       */
/**************************************************************************************************/
/* Master -> Slave (Consuming): Output Assembly 8 */
#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE        0x08 /* 8 - Control Mode, Control Setpoint, Control Instance */
#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_SIZE   5    /* 5 Bytes */

/* Slave -> Master (Producing): Input Assembly 100 */
#define DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE        0x64 /* 100 - Exception Status, Pressure, Position, Device Status, Access Mode */
#define DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE_SIZE   7    /* 7 Bytes */


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
  ptCfg->usDeviceType       = DEVICE_TYPE_PROCESS_CONTROL_VALVE;
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
  if (ulRet != 0)
    return ulRet;

  /* Register VAT User Objects (Class 0x64, 0x65) */
  printf("\n=== VAT User Object Registration ===\n");
  ulRet = AppDNS_VAT_UserObject_Registration(ptAppData);
  if (ulRet != 0) {
    printf("ERROR: VAT User Object Registration failed: 0x%08X\n", (unsigned int)ulRet);
    return ulRet;
  }
  printf("=== VAT User Object Registration Complete ===\n\n");

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
    printf("Unexpected packet resource received!!!" NEWLINE);
    return false;
  }

  /* ⭐ Debug: Print received packet info */
  printf("[PKT] Cmd=0x%08X, Len=%u, State=0x%08X" NEWLINE,
         (unsigned int)ptPacket->tHeader.ulCmd,
         (unsigned int)ptPacket->tHeader.ulLen,
         (unsigned int)ptPacket->tHeader.ulState);

  switch( ptPacket->tHeader.ulCmd )
  {
  /* ⭐ CIP Service Indication Handler ⭐ */
  case DNS_CMD_CIP_SERVICE_IND:
    printf("[INFO] ✅ CIP Service Indication Received!" NEWLINE);
    /* Route to VAT UserObject Handler */
    AppDNS_HandleCipServiceIndication(ptAppData);
    fPacketHandled = true;
    break;

    default:
    {
      if ((ptPacket->tHeader.ulCmd & 0x1) == 0)
      {
        printf("Warning: Disregarded indication packet received!" NEWLINE);

        ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
        ptPacket->tHeader.ulCmd |= 0x01; /* Make it a response */
        /* Send the response packet */
        AppDNS_GlobalPacket_Send(ptAppData);
      }
      else
      {
        printf("Warning: Disregarded confirmation packet received!" NEWLINE);
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
    printf("Sending DNS_RESET_RES failed with error code 0x%08x" NEWLINE, (unsigned int)ulRet);
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


/* =========================================================================
 * 2. Class 등록 함수 (파일 끝부분에 추가)
 * ========================================================================= */

/******************************************************************************
 * Register CIP Class to DeviceNet Stack
 ******************************************************************************/
uint32_t AppDNS_RegisterClass(APP_DATA_T* ptAppData, uint32_t ulClass)
{
    uint32_t ulRet = CIFX_NO_ERROR;
    DNS_PACKET_REGISTER_CLASS_REQ_T* ptReq =
        (DNS_PACKET_REGISTER_CLASS_REQ_T*)AppDNS_GlobalPacket_Init(ptAppData);

    /* Set packet header */
    ptReq->tHead.ulCmd = DNS_CMD_REGISTER_CLASS_REQ;
    ptReq->tHead.ulLen = DNS_REGISTER_CLASS_REQ_SIZE;
    ptReq->tHead.ulSta = 0;

    /* Set class to register */
    ptReq->tData.ulClass = ulClass;
    ptReq->tData.ulServiceMask = 0xFFFFFFFF;  /* All services */

    /* Send and receive */
    ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);

    if (ulRet == CIFX_NO_ERROR) {
        printf("    [OK] Class 0x%02X registered successfully\n", (unsigned int)ulClass);
    } else {
        printf("    [FAIL] Failed to register Class 0x%02X, error: 0x%08X\n",
               (unsigned int)ulClass, (unsigned int)ulRet);
    }

    return ulRet;
}


/******************************************************************************
 * Register All VAT CIP Classes
 ******************************************************************************/
uint32_t AppDNS_RegisterAllVatClasses(APP_DATA_T* ptAppData)
{
    uint32_t ulRet = CIFX_NO_ERROR;

    printf("\n=== Registering VAT CIP Classes ===\n");

    /* Register Identity Object (Class 0x01) */
    ulRet = AppDNS_RegisterClass(ptAppData, 0x01);
    if (ulRet != CIFX_NO_ERROR) {
        return ulRet;
    }

    /* Register VAT Object (Class 0x30) */
    ulRet = AppDNS_RegisterClass(ptAppData, 0x30);
    if (ulRet != CIFX_NO_ERROR) {
        return ulRet;
    }

    printf("===================================\n\n");

    return ulRet;
}



/* =========================================================================
 * 3. CIP Service Indication 처리 함수 (파일 끝부분에 추가)
 * ========================================================================= */

/******************************************************************************
 * Handle CIP Service Indication (Explicit Message from Master)
 ******************************************************************************/
void AppDNS_HandleCipServiceIndication(APP_DATA_T* ptAppData)
{
    DNS_PACKET_CIP_SERVICE_IND_T* ptInd =
        (DNS_PACKET_CIP_SERVICE_IND_T*)AppDNS_GlobalPacket_Get(ptAppData);

    DNS_PACKET_CIP_SERVICE_RES_T* ptRes =
        (DNS_PACKET_CIP_SERVICE_RES_T*)AppDNS_GlobalPacket_Get(ptAppData);

    /* Extract CIP message information */
    uint8_t service = (uint8_t)ptInd->tData.ulService;
    uint8_t class_id = (uint8_t)ptInd->tData.ulClass;
    uint8_t instance_id = (uint8_t)ptInd->tData.ulInstance;
    uint8_t attribute_id = (uint8_t)ptInd->tData.ulAttribute;
    uint32_t dataLen = ptInd->tHead.ulLen - DNS_CIP_SERVICE_IND_SIZE;

    /* Debug output */
    printf("\n=== CIP Service Indication ===\n");
    printf("Service:   0x%02X ", service);
    if (service == 0x0E) printf("(Get Attribute Single)\n");
    else if (service == 0x10) printf("(Set Attribute Single)\n");
    else if (service == 0x01) printf("(Get Attribute All)\n");
    else if (service == 0x16) printf("(Save)\n");
    else if (service == 0x05) printf("(Reset)\n");
    else printf("(Unknown)\n");

    printf("Class:     0x%02X\n", class_id);
    printf("Instance:  0x%02X\n", instance_id);
    printf("Attribute: 0x%02X\n", attribute_id);
    printf("Data Len:  %d\n", (int)dataLen);

    /* Route to VAT UserObject Handler for VAT-specific classes */
    if (class_id == 0x64 || class_id == 0x65) {
        /* VAT Parameter (0x64) or Diagnostic (0x65) Object */
        printf("  -> Routing to VAT UserObject Handler\n");

        /* Check if object is registered */
        if (!AppDNS_VAT_UserObject_IsRegistered(class_id, instance_id)) {
            printf("  -> ERROR: Object Class 0x%02X Instance %u not registered\n",
                   class_id, instance_id);
            /* Send error response */
            ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
            ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE;
            ptRes->tHead.ulSta = CIFX_NO_ERROR;
            ptRes->tData.ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
            ptRes->tData.ulERC = 0;
            AppDNS_GlobalPacket_Send(ptAppData);
            return;
        }

        /* Call VAT UserObject indication handler */
        uint32_t ulResDataSize = 0;
        AppDNS_VAT_UserObject_Indication(ptAppData, ptInd, ptRes, &ulResDataSize);

        /* Send response */
        ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
        ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE + ulResDataSize;
        ptRes->tHead.ulSta = CIFX_NO_ERROR;
        AppDNS_GlobalPacket_Send(ptAppData);

        return;  /* VAT handler processed request */
    }

    /* Process CIP service */
    uint8_t error = 0;
    uint32_t respDataLen = 0;

    switch (service) {
        case 0x0E:  /* Get Attribute Single */
        {
            uint8_t respSize = 0;
            error = CIP_HandleGetAttributeSingle(class_id, instance_id, attribute_id,
                                                  ptRes->tData.abData, &respSize);
            respDataLen = respSize;

            if (error == 0) {
                printf("  -> Get: Success, Data=");
                for (uint32_t i = 0; i < respDataLen; i++) {
                    printf("%02X ", ptRes->tData.abData[i]);
                }
                printf("\n");
            } else {
                printf("  -> Get: Error 0x%02X\n", error);
            }
            break;
        }

        case 0x10:  /* Set Attribute Single */
        {
            printf("  Data: ");
            for (uint32_t i = 0; i < dataLen; i++) {
                printf("%02X ", ptInd->tData.abData[i]);
            }
            printf("\n");

            error = CIP_HandleSetAttributeSingle(class_id, instance_id, attribute_id,
                                                  ptInd->tData.abData, (uint8_t)dataLen);
            respDataLen = 0;

            if (error == 0) {
                printf("  -> Set: Success\n");
            } else {
                printf("  -> Set: Error 0x%02X\n", error);
            }
            break;
        }

        case 0x01:  /* Get Attribute All */
        {
            uint16_t respSize = 0;
            error = CIP_HandleGetAttributeAll(class_id, instance_id,
                                              ptRes->tData.abData, &respSize);
            respDataLen = respSize;
            printf("  -> Get All: %s, Size=%d\n",
                   (error == 0) ? "Success" : "Error", respSize);
            break;
        }

        case 0x16:  /* Save */
        {
            error = CIP_HandleSave();
            respDataLen = 0;
            printf("  -> Save: %s\n", (error == 0) ? "Success" : "Error");
            break;
        }

        case 0x05:  /* Reset */
        {
            error = CIP_HandleReset();
            respDataLen = 0;
            printf("  -> Reset: %s\n", (error == 0) ? "Success" : "Error");
            break;
        }

        default:
            error = 0x08;  /* Service not supported */
            respDataLen = 0;
            printf("  -> Unsupported service!\n");
            break;
    }

    /* Build response packet */
    ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
    ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE + respDataLen;
    ptRes->tHead.ulSta = 0;

    ptRes->tData.ulService = ptInd->tData.ulService;
    ptRes->tData.ulClass = ptInd->tData.ulClass;
    ptRes->tData.ulInstance = ptInd->tData.ulInstance;
    ptRes->tData.ulAttribute = ptInd->tData.ulAttribute;
    ptRes->tData.ulMember = ptInd->tData.ulMember;
    ptRes->tData.ulGRC = error;  /* General Error Code */
    ptRes->tData.ulERC = 0;      /* Extended Error Code */

    printf("Response:  Cmd=0x%08X, Len=%d, Error=0x%02X\n",
           (unsigned int)ptRes->tHead.ulCmd, (int)ptRes->tHead.ulLen, error);
    printf("==============================\n\n");

    /* Send response */
    AppDNS_GlobalPacket_Send(ptAppData);
}
