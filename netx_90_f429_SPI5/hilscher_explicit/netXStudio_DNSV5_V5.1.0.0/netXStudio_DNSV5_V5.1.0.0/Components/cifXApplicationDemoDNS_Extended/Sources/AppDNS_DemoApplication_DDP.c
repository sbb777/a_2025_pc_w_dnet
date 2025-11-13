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
#include "DNS_Includes.h"

#include "AppDNS_DemoApplication.h"
#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"

/* Common Hilscher includes */
#include "Hil_ApplicationCmd.h"
#include "Hil_Results.h"
#include "Hil_Packet.h"
#include "Hil_SystemCmd.h"


/**************************************************************************************************
*! AppDNS_DDP_SetSerialNumber.
*  This function can be called optionally once at startup if the host application wants to provide
*  a serial number to the firmware. Normally the firmware takes the serial number from its associated
*  source of device data e.g. Flash Device Label or Security Memory. Alternatively the host application
*  can provide a serial number to the firmware with this function. Therefore the firmware must start in
*  DDP mode passive. DDP stands for Device Data Provider. To force the firmware to start in DDP mode
*  passive use the tag list editor tool and refer to the API manual for additional details.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_DDP_SetSerialNumber(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t                   ulRet = CIFX_NO_ERROR;
  HIL_DDP_SERVICE_SET_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);
  char*                      szSerialNumber = "76543";

  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd = HIL_DDP_SERVICE_SET_REQ;
  ptReq->tHead.ulLen = sizeof(ptReq->tData.ulDataType) + strlen(szSerialNumber) + 1;
  ptReq->tData.ulDataType = HIL_DDP_SERVICE_DATATYPE_OEM_SERIALNUMBER;

  memcpy(ptReq->tData.uDataType.szString, szSerialNumber, strlen(szSerialNumber) + 1);

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  if (CIFX_NO_ERROR != ulRet)
  {
    /* Could not set serial serial number. Most likely the firmware is not in DDP mode passive.
     * Make sure the firmware is configured with the tag list editor to hold on in DDP state passive
     * at startup. Only in DDP mode passive it is possible to set the serial number.
     */
  }
  else
  {
    /* Render the OEM parameter to "valid" */
    ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
    ptReq->tHead.ulCmd = HIL_DDP_SERVICE_SET_REQ;
    ptReq->tHead.ulLen = sizeof(ptReq->tData.ulDataType) + sizeof(ptReq->tData.uDataType.ulValue);
    ptReq->tData.ulDataType = HIL_DDP_SERVICE_DATATYPE_OEM_OPTIONS;
    ptReq->tData.uDataType.ulValue = 0xF;   /* Set all OEM bits valid  */

    ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  }

  return ulRet;
}

/**************************************************************************************************
*! AppDNS_DDP_Activate.
*  This function needs to be called after OEM parameter (e.g serial number) are written in DDP passive
*  state. Switching the DDP mode to active will force the firmware to apply the OEM parameter.
*  After switching the DDP state to active the host application can proceed with the normal
*  communication stack configuration.
***************************************************************************************************/
uint32_t AppDNS_DDP_Activate(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t                   ulRet = CIFX_NO_ERROR;
  HIL_DDP_SERVICE_SET_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);

  /* required when initial DPP state is passive: Set DDP active now */
  ptReq->tHead.ulCmd = HIL_DDP_SERVICE_SET_REQ;
  ptReq->tHead.ulLen = sizeof(ptReq->tData.ulDataType) + sizeof(ptReq->tData.uDataType.ulValue);
  ptReq->tData.ulDataType = HIL_DDP_SERVICE_DATATYPE_STATE;
  ptReq->tData.uDataType.ulValue = HIL_DDP_SERVICE_STATE_ACTIVE;

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  return ulRet;
}
