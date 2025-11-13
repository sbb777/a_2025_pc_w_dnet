/*!************************************************************************//*!
 * \file     netx_drv_eth_xpic.h
 * \brief    Ethernet xpic mac peripheral module driver.
 * $Revision: 4646 $
 * $Date: 2018-12-19 09:47:36 +0200 (Wed, 19 Dec 2018) $
 * \copyright Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 * \note Exclusion of Liability for this demo software:
 * The following software is intended for and must only be used for reference and in an
 * evaluation laboratory environment. It is provided without charge and is subject to
 * alterations. There is no warranty for the software, to the extent permitted by
 * applicable law. Except when otherwise stated in writing the copyright holders and/or
 * other parties provide the software "as is" without warranty of any kind, either
 * expressed or implied.
 * Please refer to the Agreement in README_DISCLAIMER.txt, provided together with this file!
 * By installing or otherwise using the software, you accept the terms of this Agreement.
 * If you do not agree to the terms of this Agreement, then do not install or use the
 * Software!
 ******************************************************************************/
#include "netx_drv.h"
#ifndef _HAL_ETH_STD_MAC_XPIC_H
#define _HAL_ETH_STD_MAC_XPIC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*! \addtogroup Legacy
 * \{
 */

/*!
 * \addtogroup ETH_XPIC
 * \{
 */

#include "netx_drv_eth_xpic_def.h"
#include <stdbool.h>

#define HAL_ETHXPIC_VERSION_MAJOR    2
#define HAL_ETHXPIC_VERSION_MINOR    0
#define HAL_ETHXPIC_VERSION_BUILD    0
#define HAL_ETHXPIC_VERSION_REVISION 0

/*****************************************************************************/
/*! Initial HAL Settings
* \description
*   This structure holds all runtime parameters for the HAL.
*   The function DRV_ETH_Xpic_Initialize() takes a pointer to this structure
*   as parameter and initializes the HAL according to the structure's values. */
/*****************************************************************************/
typedef struct DRV_ETH_XPIC_CONFIG_Ttag
{
  bool                  fPhyMode;  /**< true: netX behaves like a PHY, driving the MII RXCLK and TXCLK;
                                        false: netX behaves like a MAC, sampling the MII RXCLK and TXCLK
                                        of an external PHY */
  ETH_XPIC_MII_CFG_T    tMiiCfg;   /**< Configuration of operating mode (Half-/Full-DUPLEX, 10/100Mbps)
                                        This value is ignored if fPhyMode is true. */
} DRV_ETH_XPIC_CONFIG_T;


/*****************************************************************************/
/*! Result Codes for HAL Functions
* \description
*   All DRV_ETH_XPIC HAL functions return one of the following values.
*   These values shall always be evaluated by the calling function.          */
/*****************************************************************************/
typedef enum
{
  DRV_ETH_XPIC_OK = 0,             /**< Success */
  DRV_ETH_XPIC_NOT_INITIALIZED,    /**< The function EthStdMax_Xpic_Initialize() was not called */
  DRV_ETH_XPIC_NOT_RUNNING,        /**< The function DRV_ETH_Xpic_Start() was not called */
  DRV_ETH_XPIC_INVALID_ARG,        /**< One or more function parameters are invalid, e.g. NULL pointers etc. */
  DRV_ETH_XPIC_NO_FRAME_AVAILABLE, /**< There are currently no more frame buffers available */
  DRV_ETH_XPIC_INVALID_HANDLE      /**< The frame handle in parameter hFrame is invalid */
} DRV_ETH_XPIC_RESULT;

/*****************************************************************************/
/*! HAL Parameter Types
* \description
*   Parameter types used by the functions DRV_ETH_Xpic_SetParam() and DRV_ETH_Xpic_GetParam(). */
/*****************************************************************************/
typedef enum
{
  DRV_ETH_XPIC_PARAM_IRQ_MASK = 0,            /**< Interrupt enable mask, the value should be a combination of the MSK_XPIC_SHM_IRQ_IND_... masks */
  DRV_ETH_XPIC_PARAM_VLAN_PRIORITIZATION,     /**< Traffic class arrangement for VLAN priority classes
                                                      parameter value = 0..8
                                                      VLAN priority >= parameter value: high priority
                                                      VLAN priority <  parameter value: low priority */
  DRV_ETH_XPIC_PARAM_DSCP_PRIORITIZATION,     /**< Traffic Class arrangement for IP frames (overwrites VLAN priority)
                                                      parameter value = 0..0x40
                                                      DSCP >= parameter value: high priority
                                                      DSCP <  parameter value : low  priority
                                                      parameter value = 0x00 : all DSCP frames have high priority
                                                      parameter value = 0x40 : all DSCP frames have low priority */
  DRV_ETH_XPIC_PROMISC_MODE,                  /**< Enables promiscuous mode (all frame are indicated regardless of MAC address)
                                                      parameter value 0/1: disable/enable promiscuous mode */
  DRV_ETH_XPIC_CONNECTION_STATE               /**< Connection state from the PHY (read only).
                                                      The bit masks MSK_XPIC_SHM_CONNECTION_STATE_... define the value of this parameter.
                                                      bit 0: 0/1: PHY is in 10/100 MBps mode
                                                      bit 1: 0/1: PHY is in Half/Full DUPLEX mode
                                                      bit 2: 0/1: Link is down/up */
} DRV_ETH_XPIC_PARAM;


DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Reset(void* pvUser);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Initialize(DRV_ETH_XPIC_CONFIG_T const* ptConfig, void* pvUser);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_SetMacAddr(uint8_t const* pabMacAdr);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetMacAddr(uint8_t* pabMacAdr);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Start(void* pvUser);

DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetConfirmIrq(uint32_t* pulIrq);

DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetFrame(ETH_FRAMEBUF_T** pptFrame);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Send(ETH_FRAMEBUF_T* ptFrame, bool fConfirm, bool fHiPriority);
unsigned int        DRV_ETH_Xpic_GetSendCnfFillLevel(void);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetSendCnf(ETH_FRAMEBUF_T** pptFrame, ETH_XPIC_CNF_ERR* pErrorCode);

unsigned int        DRV_ETH_Xpic_GetRecvFillLevel(bool fHiPriority);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Recv(ETH_FRAMEBUF_T** pptFrame, bool fHiPriority);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_ReleaseFrame(ETH_FRAMEBUF_T* ptFrame);

DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_SetParam(DRV_ETH_XPIC_PARAM eParam, uint32_t ulValue);
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetParam(DRV_ETH_XPIC_PARAM eParam, uint32_t* pulValue);

DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetCounters(XPIC_SHM_CNT_T* ptMacCounter);

DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_ReadPhyReg(unsigned int uMiimuPreamble,
                                                unsigned int uMiimuMdcFreq,
                                                unsigned int uMiimuRtaField,
                                                unsigned int uMiimuPhyAddr,
                                                unsigned int uMiimuReqAddr,
                                                uint16_t* pusData,
                                                void*    pvUser);

DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_WritePhyReg(unsigned int uMiimuPreamble,
                                                 unsigned int uMiimuMdcFreq,
                                                 unsigned int uMiimuPhyAddr,
                                                 unsigned int uMiimuReqAddr,
                                                 uint16_t usData,
                                                 void* pvUser);


/*! \} *//* End of group ETH_XPIC */

/*! \} *//* End of group Legacy */

/*! \} *//* End of group netX_Driver */

#ifdef __cplusplus
}
#endif
#endif /* _HAL_ETH_STD_MAC_XPIC_H */
