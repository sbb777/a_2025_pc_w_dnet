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
#include "DNS_Includes.h"
#include "AppDNS_DemoApplication.h"

#include "Hil_Results.h"
#include "Hil_Packet.h"
#include "Hil_ComponentID.h"


static uint32_t RemanentDataStore(char*    szName,
                                  uint8_t* pbData,
                                  uint32_t ulDataLen);

static uint32_t RemanentDataLoad(char*     szName,
                                 uint8_t*  pbData,
                                 uint32_t  ulMaxDataSize);

static uint32_t s_ulRemanentDataSize = 0;
char szRemanentFile[] = "DnsRemanent";

/* Pass BLOB of NV-data down to the stack (as stored in an earlier STORE_REMANENT_DATA_IND) */

/***************************************************************************************************
*! AppDNS_SetRemanentData.
*  This function is called once at startup to pass BLOB of NV-data down to the stack, as stored in
*  the earlier STORE_REMANENT_DATA_IND packet. This packet need to be send only, when the storage of
*  remanent data is explicit enabled to be handled in the host appliaction. Normaly the handling of
*  none volatile data is handled by the stack itself.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_SetRemanentData(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  /* Set remanent data for the stack when host stores (according to taglist) */
  uint32_t                     ulRet = CIFX_NO_ERROR;
  HIL_SET_REMANENT_DATA_REQ_T* ptReq = (HIL_SET_REMANENT_DATA_REQ_T*)AppDNS_GlobalPacket_Init(ptDnsRsc);
  CIFX_PACKET* ptCifxPckt = (CIFX_PACKET*)ptReq;

  RemanentDataLoad(szRemanentFile,
                  (uint8_t*)&ptReq->tData,
                  sizeof(ptCifxPckt->abData));

  ptReq->tHead.ulCmd = HIL_SET_REMANENT_DATA_REQ;

  if(s_ulRemanentDataSize < sizeof(ptReq->tData.ulComponentId))
  {
    /* to less or no remanent data loaded */
    ptReq->tData.ulComponentId = HIL_COMPONENT_ID_DEVNET_GCI_SLAVE;
    ptReq->tHead.ulLen = sizeof(ptReq->tData.ulComponentId);
  }
  else if (ptReq->tData.ulComponentId != HIL_COMPONENT_ID_DEVNET_GCI_SLAVE)
  {
    /* seems the loaded remanent data are no DNS remanent data  */
    ptReq->tData.ulComponentId = HIL_COMPONENT_ID_DEVNET_GCI_SLAVE;
    ptReq->tHead.ulLen = sizeof(ptReq->tData.ulComponentId);
  }
  else
  {
    /* valid remanent DNS data loaded */
    ptReq->tHead.ulLen = s_ulRemanentDataSize;
  }

  /* Set remanent data to the stack.
    Under the following conditions, the stack will reset the remanent data.
    1) If a packet of size HIL_SET_REMANENT_DATA_REQ_SIZE is sent.
       (i.e. not containing any actual remanent data).
    2) If inconsistent remanent data is sent.
       (i.e. data of correct length but with, e.g., a checksum failure)
    3) If remanent data has previously been invalidated via HIL_DELETE_CONFIG_REQ.
  */

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  return ulRet;
}

/***************************************************************************************************
*! AppDNS_StoreRemanent_Indication.
*  This function is called when stack sends an indication to store remanent data. For the host application
*  the reamanent data are complete transparent as byte stream. This indication will only be sent by the
*  stack, if it is configured accordingly in the taglist of the firmware. Per default, the stack stores
*  the remanent data by itself.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
void AppDNS_StoreRemanent_Indication(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  HIL_STORE_REMANENT_DATA_IND_T* ptInd = (HIL_STORE_REMANENT_DATA_IND_T*)AppDNS_GlobalPacket_Get(ptDnsRsc);
  uint32_t ulDataWritten = 0;

  ulDataWritten = RemanentDataStore(szRemanentFile, (uint8_t*)&ptInd->tData, ptInd->tHead.ulLen);
  if (ulDataWritten != ptInd->tHead.ulLen)
  {
    ptInd->tHead.ulSta = ERR_HIL_FAIL;
  }

  ptInd->tHead.ulLen = HIL_STORE_REMANENT_DATA_RES_SIZE;
  ptInd->tHead.ulCmd |= 0x01;

  AppDNS_GlobalPacket_Send(ptDnsRsc);
}


/**************************************************************************************/
static uint32_t RemanentDataStore(char*    szName,
                                  uint8_t* pbData,
                                  uint32_t ulDataLen)
{
#ifdef FILE_SYSTEM_SUPPORT
  uint32_t ulWritten = 0;
  FILE * ptFile = NULL;

  ptFile = fopen(szName, "wb");

  if (ptFile)
  {
    ulWritten = (uint32_t)fwrite(pbData, 1, ulDataLen, ptFile);
    fclose(ptFile);
  }

  return ulWritten;
#else
  /* implement here own remanent data storage method, e.g. raw flash */
  return 0;
#endif
}


/**************************************************************************************/
static uint32_t RemanentDataLoad(char*     szName,
                                 uint8_t*  pbData,
                                 uint32_t  ulMaxDataSize)
{
#ifdef FILE_SYSTEM_SUPPORT
  uint32_t ulRead = 0;
  FILE* ptFile = NULL;

  ptFile = fopen(szName, "rb");

  if (ptFile)
  {
    ulRead = (uint32_t)fread(pbData, 1, ulMaxDataSize, ptFile);
    fclose(ptFile);
  }
  return ulRead;
#else
  /* implement here own remanent data storage method, e.g. raw flash */
  return 0;
#endif
}


