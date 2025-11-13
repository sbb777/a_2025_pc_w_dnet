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
#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"


/**************************************************************************************/
/**************************************************************************************/
CIPHIL_CLASS_MNS_ATT_ALL_T g_tMnsState = { 0,0 };

char * g_aszModuleState[] =
{
  "MODULE_STATUS_NO_POWER",
  "MODULE_STATUS_SELFTEST",
  "MODULE_STATUS_STANDBY",
  "MODULE_STATUS_OPERATE",
  "MODULE_STATUS_MINOR_FAULT",
  "MODULE_STATUS_MAJOR_FAULT",
};

char * g_aszNetworkState[] =
{
  "NETWORK_STATUS_NO_POWER",
  "NETWORK_STATUS_NO_CONNECTION",
  "NETWORK_STATUS_CONNECTED",
  "NETWORK_STATUS_CONNECTION_TIMEOUT",
  "NETWORK_STATUS_MAJOR_FAULT",
  "MNS_NETWORK_STATUS_SELFTEST",
};


/***************************************************************************************************
*! AppDNS_UpdateModuleNetworStatusData.
*  This function is called to update the new received Module Network Status data.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
void AppDNS_UpdateModuleNetworStatusData(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
                                         CIPHIL_CLASS_MNS_ATT_ALL_T*    ptMnsState)
{
  if (g_tMnsState.ulModuleStatus != ptMnsState->ulModuleStatus ||
      g_tMnsState.ulNetworkStatus != ptMnsState->ulNetworkStatus)
  {
    g_tMnsState.ulModuleStatus = ptMnsState->ulModuleStatus;
    g_tMnsState.ulNetworkStatus = ptMnsState->ulNetworkStatus;

    PRINTF("%s" NEWLINE, g_aszModuleState[g_tMnsState.ulModuleStatus]);
    PRINTF("%s" NEWLINE, g_aszNetworkState[g_tMnsState.ulNetworkStatus]);
  }
}


/***************************************************************************************************
*! AppDNS_MnsObject_Registration.
*  This function is called once at startup to register for MNS Object indications.
*  The registration is done via attribute option flags.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_MnsObject_Registration(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = 0;
  CIPHIL_SET_ATTR_OPTION_T tAttOption = { 0 };
  tAttOption.usMask = CIP_FLG_TREAT_NOTIFY;
  tAttOption.usValue = CIP_FLG_TREAT_NOTIFY;


  ulRet = AppDNS_CipService(ptDnsRsc,
                            CIPHIL_SERVICE_SET_ATTR_OPTION,     /* Service */
                            CIPHIL_CLASS_MN_STATUS, 1,          /* Class, Instance */
                            CIPHIL_CLASS_MNS_ATT_MODULE_STATUS, /* Attribute */
                            (uint8_t*)&tAttOption,              /* Data */
                            sizeof(CIPHIL_SET_ATTR_OPTION_T));

  return ulRet;
}

/***************************************************************************************************
*! AppDNS_MnsObject_Indication.
*  Handle of the MNS Object Indication
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
void AppDNS_MnsObject_Indication(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
                                 DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
                                 DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
                                 uint32_t*                      pulResDataSize)
{
  if (CIP_SERVICE_SET_ATTRIBUTES_ALL == ptInd->tData.ulService)
  {
    /* this is the only supported service of the MNS object */
    AppDNS_UpdateModuleNetworStatusData(ptDnsRsc, (CIPHIL_CLASS_MNS_ATT_ALL_T*)&ptInd->tData.abData[0]);
  }

  ptRes->tData.ulGRC = CIP_GRC_SUCCESS;
  ptRes->tData.ulGRC = CIP_GRC_SUCCESS;
  return;
}
