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

#include "App_SystemPackets.h"

#include "Hil_ApplicationCmd.h"
#include "Hil_SystemCmd.h"
#include "Hil_Results.h"
#include "Hil_Packet.h"

#include <string.h>
#include <stdio.h>


/*! Assembles a HIL_REGISTER_APP_REQ packet.
 *
 * \param ptPkt   [out] Packet resource used for assembling.
 */
void App_SysPkt_AssembleRegisterAppReq(CIFX_PACKET* ptPkt)
{
  HIL_REGISTER_APP_REQ_T *ptReq = (HIL_REGISTER_APP_REQ_T*)ptPkt;

  memset(ptReq, 0, sizeof(*ptReq));

  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd  = HIL_REGISTER_APP_REQ;
  ptReq->tHead.ulLen  = 0;
}

/*! Evaluates a HIL_REGISTER_APP_REQ packet
 *
 *  \param  ptPkt [in] Pointer to confirmation Packet
 */
void App_SysPkt_HandleRegisterAppCnf(CIFX_PACKET* ptPkt)
{
}




/*! Assembles a HIL_CHANNEL_INIT_REQ packet.
 *
 * \param ptPkt  [out] Packet resource used for assembling.
 */
void App_SysPkt_AssembleChannelInitReq(CIFX_PACKET* ptPkt)
{
  HIL_CHANNEL_INIT_REQ_T *ptReq = (HIL_CHANNEL_INIT_REQ_T*)ptPkt;

  memset(ptReq, 0, sizeof(*ptReq));

  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd  = HIL_CHANNEL_INIT_REQ;
  ptReq->tHead.ulLen  = 0;
}

/*! Evaluates a HIL_CHANNEL_INIT_CNF packet
 *
 *  \param  ptPkt [in] Pointer to confirmation Packet
 */
void App_SysPkt_HandleChannelInitCnf(CIFX_PACKET* ptPkt)
{
}




/*! Assembles a HIL_START_STOP_COMM_REQ packet.
 *
 * \param ptPkt    [out]  Packet resource used for assembling.
 * \param fStart   [in]   true:  start communication (BUS_ON),
 *                        false: stop communication  (BUS_OFF)
 */
void App_SysPkt_AssembleStartStopCommReq(CIFX_PACKET* ptPkt,
                                         bool         fStart)
{
  HIL_START_STOP_COMM_REQ_T* ptReq = (HIL_START_STOP_COMM_REQ_T*)ptPkt;

  memset(ptReq, 0, sizeof(*ptReq));

  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd  = HIL_START_STOP_COMM_REQ;
  ptReq->tHead.ulLen  = sizeof(ptReq->tData);

  ptReq->tData.ulParam = fStart ? HIL_START_STOP_COMM_PARAM_START : HIL_START_STOP_COMM_PARAM_STOP;
}

/*! Evaluates a HIL_START_STOP_COMM_CNF packet
 *
 *  \param  ptPkt [in] Pointer to confirmation Packet
 */
void App_SysPkt_HandleStartStopCommCnf(CIFX_PACKET* ptPkt)
{
}




/*! Assembles a HIL_SET_MAC_ADDR_REQ packet.
 *
 * \param ptPkt      [out] Packet resource used for assembling.
 * \param abMacAddr  [in]  Pointer to MAC address
 */
void App_SysPkt_AssembleSetMacAddressReq(CIFX_PACKET* ptPkt,
                                         uint8_t*     abMacAddr)
{
  HIL_SET_MAC_ADDR_REQ_T* ptReq = (HIL_SET_MAC_ADDR_REQ_T*)ptPkt;

  memset(ptReq, 0, sizeof(*ptReq));

  ptReq->tHead.ulDest = HIL_PACKET_DEST_SYSTEM;
  ptReq->tHead.ulCmd  = HIL_SET_MAC_ADDR_REQ;
  ptReq->tHead.ulLen  = sizeof(ptReq->tData);

  ptReq->tData.ulParam = 0x00;
  memcpy(&ptReq->tData.abMacAddr, abMacAddr, sizeof(ptReq->tData.abMacAddr));
}

/*! Evaluates a HIL_SET_MAC_ADDR_CNF packet
 *
 *  \param  ptPkt [in] Pointer to confirmation Packet
 */
void App_SysPkt_HandleSetMacAddressCnf(CIFX_PACKET* ptPkt)
{
}




/*! Assembles a HIL_FIRMWARE_IDENTIFY_REQ packet.
 *
 *   \param ptPkt        [out] Packet resource used for assembling.
 *   \param ulChannelId  [in]  Channel Identification
 *                               SYSTEM_CHANNEL 0xFFFFFFFF
 *                               COMM_CHANNEL_0 0x00000000
 *                               COMM_CHANNEL_1 0x00000001
 *                               COMM_CHANNEL_2 0x00000002
 *                               COMM_CHANNEL_3 0x00000003
 */
void App_SysPkt_AssembleFirmwareIdentifyReq(CIFX_PACKET* ptPkt,
                                            uint32_t     ulChannelId)
{
  HIL_FIRMWARE_IDENTIFY_REQ_T* ptReq = (HIL_FIRMWARE_IDENTIFY_REQ_T*)ptPkt;

  memset(ptReq, 0, sizeof(*ptReq));

  ptReq->tHead.ulDest       = HIL_PACKET_DEST_SYSTEM;
  ptReq->tHead.ulCmd        = HIL_FIRMWARE_IDENTIFY_REQ;
  ptReq->tHead.ulLen        = sizeof(ptReq->tData);

  ptReq->tData.ulChannelId  = ulChannelId;
}

/*! Evaluates a HIL_FIRMWARE_IDENTIFY_CNF packet
 *
 * \param  ptPkt [in] Pointer to confirmation Packet
 */
void App_SysPkt_HandleFirmwareIdentifyCnf(CIFX_PACKET* ptPkt)
{
  PRINTF("Identified firmware: %s V%u.%u.%u.%u" NEWLINE,
         ((HIL_FIRMWARE_IDENTIFY_CNF_T*)ptPkt)->tData.tFirmwareIdentification.tFwName.abName,
         ((HIL_FIRMWARE_IDENTIFY_CNF_T*)ptPkt)->tData.tFirmwareIdentification.tFwVersion.usMajor,
         ((HIL_FIRMWARE_IDENTIFY_CNF_T*)ptPkt)->tData.tFirmwareIdentification.tFwVersion.usMinor,
         ((HIL_FIRMWARE_IDENTIFY_CNF_T*)ptPkt)->tData.tFirmwareIdentification.tFwVersion.usBuild,
         ((HIL_FIRMWARE_IDENTIFY_CNF_T*)ptPkt)->tData.tFirmwareIdentification.tFwVersion.usRevision);
}




/*! Assembles a HIL_FIRMWARE_RESET_REQ packet.
 *
 *   \param ptPkt        [out] Packet resource used for assembling.
 *   \param bResetMode   [in]  Reset Mode
 *                             0 = HIL_SYS_CONTROL_RESET_MODE_COLDSTART
 *                             1 = HIL_SYS_CONTROL_RESET_MODE_WARMSTART (Unused)
 *                             2 = HIL_SYS_CONTROL_RESET_MODE_BOOTSTART
 *                             3 = HIL_SYS_CONTROL_RESET_MODE_UPDATESTART
 *   \param bResetParam  [in]  Arguments that will be evaluated upon system reset.
 *                             0x0..0xF = For mode “updatestart”, the variant to be installed (inside fwupdate.zip).
 *   \param bDelRemData  [in]  Delete complete remanent data area after reset.
 *                             1 = For mode “bootstart” and “updatestart”,
 *                                 this value specifies that the remanent data area will be deleted.
 *                             0 = Remanent data area will not be deleted.
 *
 */
void App_SysPkt_AssembleFirmwareResetReq(CIFX_PACKET* ptPkt,
                                         uint8_t      bResetMode,
                                         uint8_t      bResetParam,
                                         uint8_t      bDeleteRemanentData)
{
  HIL_FIRMWARE_RESET_REQ_T* ptReq = (HIL_FIRMWARE_RESET_REQ_T*)ptPkt;

  memset(ptReq, 0, sizeof(*ptReq));

  ptReq->tHead.ulDest        = HIL_PACKET_DEST_SYSTEM;
  ptReq->tHead.ulCmd         = HIL_FIRMWARE_RESET_REQ;
  ptReq->tHead.ulLen         = sizeof(ptReq->tData);

  ptReq->tData.ulTimeToReset = 0;
  ptReq->tData.ulResetMode   = (0x01 & bDeleteRemanentData)<<8 | (0x0F & bResetParam)<<4 | (0x0F & bResetMode);
}

/*! Evaluates a HIL_FIRMWARE_RESET_CNF packet
 *
 *  \param  ptPkt [in] Pointer to confirmation Packet
 */
void App_SysPkt_HandleFirmwareResetCnf(CIFX_PACKET* ptPkt)
{
}




/*! Assembles a HIL_HW_HARDWARE_INFO_REQ packet.
 *
 * \param ptPkt  [in]  Packet resource used for assembling.
 */
void App_SysPkt_AssembleHardwareInfoReq(CIFX_PACKET* ptPkt)
{
  HIL_HW_HARDWARE_INFO_REQ_T* ptReq = (HIL_HW_HARDWARE_INFO_REQ_T*)ptPkt;

  memset(ptReq, 0, sizeof(*ptReq));

  ptReq->tHead.ulDest = HIL_PACKET_DEST_SYSTEM;
  ptReq->tHead.ulCmd  = HIL_HW_HARDWARE_INFO_REQ;
  ptReq->tHead.ulLen  = 0;
}

/*! Evaluates a HIL_HW_HARDWARE_INFO_CNF packet
 *
 *  \param  ptPkt [in] Pointer to confirmation Packet
 */
void App_SysPkt_HandleHardwareInfoCnf(CIFX_PACKET* ptPkt)
{
  PRINTF("identified hardware device number: %d" NEWLINE,(int)((HIL_HW_HARDWARE_INFO_CNF_T*)ptPkt)->tData.ulDeviceNumber);
  PRINTF("identified hardware serial number: %d" NEWLINE,(int)((HIL_HW_HARDWARE_INFO_CNF_T*)ptPkt)->tData.ulSerialNumber);
}



/*******************************************************************************
 *  _           _ _                 _
 * (_)         | (_)           _   (_)
 *  _ ____   _ | |_  ____ ____| |_  _  ___  ____   ___
 * | |  _ \ / || | |/ ___) _  |  _)| |/ _ \|  _ \ /___)
 * | | | | ( (_| | ( (__( ( | | |__| | |_| | | | |___ |
 * |_|_| |_|\____|_|\____)_||_|\___)_|\___/|_| |_(___/
 *
 ******************************************************************************/


