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

#ifndef COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_SYSTEMPACKETS_H_
#define COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_SYSTEMPACKETS_H_

/*****************************************************************************/
/*! General Inclusion Area                                                   */
/*****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"
#include "cifXUser.h"
#include "cifXErrors.h"

/*****************************************************************************/
/*! FORWARD DECLARATIONS                                                     */
/*****************************************************************************/

struct APP_DATA_Ttag;

/*****************************************************************************/
/*! FUNCTION PROTOTYPES                                                      */
/*****************************************************************************/

void    App_SysPkt_AssembleRegisterAppReq     ( CIFX_PACKET* ptPkt);

void    App_SysPkt_AssembleSetMacAddressReq   ( CIFX_PACKET* ptPkt, uint8_t* abMacAddr );

void    App_SysPkt_AssembleChannelInitReq     ( CIFX_PACKET* ptPkt);

void    App_SysPkt_AssembleStartStopCommReq   ( CIFX_PACKET* ptPkt, bool fStart );

void    App_SysPkt_AssembleFirmwareIdentifyReq( CIFX_PACKET* ptPkt, uint32_t ulChannelId );
void    App_SysPkt_HandleFirmwareIdentifyCnf  ( CIFX_PACKET* ptPkt );

void    App_SysPkt_AssembleHardwareInfoReq    ( CIFX_PACKET* ptPkt );
void    App_SysPkt_HandleHardwareInfoCnf      ( CIFX_PACKET* ptHardwareInfoCnf );

int32_t App_SysPkt_HandleLinkStatusChangeInd  ( struct APP_DATA_Ttag*  ptAppData, int iChannelIdx, CIFX_PACKET* ptPacket );

void App_SysPkt_AssembleFirmwareResetReq( CIFX_PACKET* ptPkt, uint8_t bResetMode, uint8_t bResetParam, uint8_t bDeleteRemanentData);

uint32_t App_SysPkt_ChannelInit(int iChannelIdx);
uint32_t App_SysPkt_DeleteConfig(int iChannelIdx);

#endif /** COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_SYSTEMPACKETS_H_ */
