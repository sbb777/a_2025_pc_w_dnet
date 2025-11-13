/**************************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

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
#ifndef APP_SYSTEMPACKETS_H_
#define APP_SYSTEMPACKETS_H_

/**************************************************************************************************
 * System packets module
 *
 * This module provide functions to fill cifX packets with the corresponding request data, so
 * that the application do not need to fill the cifX packets by itself.
 *
 */

/*****************************************************************************/
/*! General Inclusion Area                                                   */
/*****************************************************************************/

#include "App_Config.h"
#include "cifXUser.h"

#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************/
/*! FUNCTION PROTOTYPES                                                      */
/*****************************************************************************/

void    App_SysPkt_AssembleRegisterAppReq     (CIFX_PACKET* ptPkt);
void    App_SysPkt_AssembleChannelInitReq     (CIFX_PACKET* ptPkt);
void    App_SysPkt_AssembleStartStopCommReq   (CIFX_PACKET* ptPkt, bool fStart);
void    App_SysPkt_AssembleSetMacAddressReq   (CIFX_PACKET* ptPkt, uint8_t* abMacAddr);
void    App_SysPkt_AssembleFirmwareIdentifyReq(CIFX_PACKET* ptPkt, uint32_t ulChannelId);

void    App_SysPkt_AssembleFirmwareResetReq   (CIFX_PACKET* ptPkt, uint8_t bResetMode, uint8_t bResetParam, uint8_t bDeleteRemanentData);
void    App_SysPkt_AssembleHardwareInfoReq    (CIFX_PACKET* ptPkt);

void    App_SysPkt_HandleFirmwareIdentifyCnf  (CIFX_PACKET* ptPkt);
void    App_SysPkt_HandleHardwareInfoCnf      (CIFX_PACKET* ptHardwareInfoCnf);


#endif /** APP_SYSTEMPACKETS_H_ */
