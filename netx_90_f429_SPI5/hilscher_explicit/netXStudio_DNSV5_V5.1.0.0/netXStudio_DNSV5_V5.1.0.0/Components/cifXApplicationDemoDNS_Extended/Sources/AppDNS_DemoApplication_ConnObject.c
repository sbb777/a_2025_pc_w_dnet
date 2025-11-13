/***************************************************************************************************
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

/*************************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

**************************************************************************************************/
/* Includes */
#include "DNS_Includes.h"
#include "AppDNS_DemoApplication.h"


#define POLL_CONNECTION    2
#define STROBE_CONNECTION  3
#define COS_CYC_CONNECTION 4

/***************************************************************************************************
*! AppDNS_ConnObject_Registration.
*  This function is called once at startup to register to the EPR of the Connection Object.
*  The registration is done via attribute option flags. In this example the registration is done
*  only for the POLL instance of the connection object. Once registered the stack will always send
*  an indication when ever the DeviceNet master sets the EPR to activate the connection.
*  This an option for the host appliaction to validate its context if all parameter and conditions
*  are given to release the comunication.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_ConnObject_Registration(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet  = 0;
  CIPHIL_SET_ATTR_OPTION_T tAttOption = { 0 };
  tAttOption.usMask  = CIP_FLG_TREAT_NOTIFY;
  tAttOption.usValue = CIP_FLG_TREAT_NOTIFY;

  ulRet = AppDNS_CipService(ptDnsRsc,
                            CIPHIL_SERVICE_SET_ATTR_OPTION,                /* Service */
                            CIP_CLASS_CONNECTION, 2,                       /* Class, Instance */
                            CIP_CLASS_CONNECTION_ATT_EXPECTED_PACKET_RATE, /* Attribute */
                            (uint8_t*)&tAttOption,                         /* Data */
                            sizeof(CIPHIL_SET_ATTR_OPTION_T));

  return ulRet;
}

/***************************************************************************************************
*! AppDNS_ConnObject_Indication.
*  This function is called when the DeviceNet master has writte to the EPR of the Connection Object.
*  At this point the application has the option to validate its context if all parameter and conditions
*  are given to release the communication. If the application wants to reject the connection it has to
*  respond with a error response.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
void AppDNS_ConnObject_Indication(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
                                  DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
                                  DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
                                  uint32_t*                      pulResDataSize)
{
  if( (CIP_SERVICE_SET_ATTRIBUTE_SINGLE == ptInd->tData.ulService) &&
    (CIP_CLASS_CONNECTION_ATT_EXPECTED_PACKET_RATE == ptInd->tData.ulAttribute) &&
    ((ptInd->tData.ulInstance == POLL_CONNECTION)||
     (ptInd->tData.ulInstance == COS_CYC_CONNECTION)||
     (ptInd->tData.ulInstance == STROBE_CONNECTION)) )
  {
    if (1 /* Application context ready */)
    {
      ptRes->tData.ulGRC = CIP_GRC_SUCCESS;
      ptRes->tData.ulERC = 0;
    }
    else
    {
      ptRes->tData.ulGRC = CIP_GRC_DEVICE_STATE_CONFLICT;
      ptRes->tData.ulERC = 0;

    }
  }
  else
  {
    ptRes->tData.ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
    ptRes->tData.ulERC = 0;
  }

  return;
}
