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

#ifndef COMPONENTS_CIFXAPPLICATIONDEMOEIS_INCLUDES_APPEIS_DEMOAPPLICATION_H_
#define COMPONENTS_CIFXAPPLICATIONDEMOEIS_INCLUDES_APPEIS_DEMOAPPLICATION_H_

#include "App_DemoApplication.h"

/***************************************************************************************/

#define DNS_DEMO_CHANNEL_INDEX  0

/*************************************************************************************/
/*************************************************************************************/


/*************************************************************************************/
/*************************************************************************************/
typedef struct APP_DNS_DATA_Ttag
{
  uint8_t bNodeIdValue;
  uint8_t bBaudRateValue;
} APP_DNS_DATA_T;

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/

bool     AppDNS_PacketHandler_callback   ( CIFX_PACKET* ptPacket, void* pvUserData );

uint32_t AppDNS_GlobalPacket_SendReceive ( APP_DATA_T* ptAppData );
uint32_t AppDNS_GlobalPacket_Send        ( APP_DATA_T* ptAppData);
void*    AppDNS_GlobalPacket_Get         ( APP_DATA_T* ptAppData);
void*    AppDNS_GlobalPacket_Init        ( APP_DATA_T* ptAppData );

uint32_t AppDNS_ConfigureStack       ( APP_DATA_T* ptAppData, uint32_t ulBusOnDelay);


#endif /* COMPONENTS_CIFXAPPLICATIONDEMOEIS_INCLUDES_APPEIS_DEMOAPPLICATION_H_ */
