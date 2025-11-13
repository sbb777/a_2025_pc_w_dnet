/*!************************************************************************//**
 * \file    netx_drv_canctrl.h
 * \brief   Header file of CAN controller module.
 * \warning
 * WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * The pad_ctrl has to be modified to use the can!
 * The following lines of code has to be added at best to the main.
 * It is necessary to gain the modification rights for the proper registers by
 * reading and writing the asic access key and directly after this the pad_ctrl
 * value of the pin shall be modified to the value 0x00000041.
 * The following both lines of code show this by reading and wirting the key,
 * followed by setting the value
 *   \code{.cpp}
 *   ((volatile unsigned int *) 0xff4012c0)[0] = ((volatile unsigned int *) 0xff4012c0)[0];
 *   ((volatile unsigned int *) 0xff401000)[47] = 0x41ull;
 *   \endcode
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
#ifndef __NETX_DRV_CANCTRL_H
#define __NETX_DRV_CANCTRL_H

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
 * \addtogroup CANCTRL
 * \{
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>   /* ISO C99: uint8_t/uint16_t/uint32_t definitions */
#include <stdbool.h>  /* ISO C99: "bool" definition */

/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/
#define DRV_CANCTRL_BAUDRATE_1000K   1000
#define DRV_CANCTRL_BAUDRATE_800K     800
#define DRV_CANCTRL_BAUDRATE_500K     500
#define DRV_CANCTRL_BAUDRATE_250K     250
#define DRV_CANCTRL_BAUDRATE_125K     125
#define DRV_CANCTRL_BAUDRATE_50K       50
#define DRV_CANCTRL_BAUDRATE_20K       20
#define DRV_CANCTRL_BAUDRATE_10K       10

#define DRV_CANCTRL_OVERSAMPLING_EN    1
#define DRV_CANCTRL_OVERSAMPLING_DIS   0

/* Interrupt Enable register */
#define DRV_CANCTRL_MSK_IRQ_EN_RX_IRQ_EN          0x00000001U
#define DRV_CANCTRL_MSK_IRQ_EN_TX_IRQ_EN          0x00000002U
#define DRV_CANCTRL_MSK_IRQ_EN_WARNING_IRQ_EN     0x00000004U
#define DRV_CANCTRL_MSK_IRQ_EN_OVERRUN_IRQ_EN     0x00000008U
#define DRV_CANCTRL_MSK_IRQ_EN_ERR_PASSIVE_IRQ_EN 0x00000020U
#define DRV_CANCTRL_MSK_IRQ_EN_ARB_LOST_IRQ_EN    0x00000040U
#define DRV_CANCTRL_MSK_IRQ_EN_BUS_ERROR_IRQ_EN   0x00000080U

/* Interrupt register */
#define DRV_CANCTRL_MSK_IRQ_RX_IRQ          0x00000001U
#define DRV_CANCTRL_MSK_IRQ_TX_IRQ          0x00000002U
#define DRV_CANCTRL_MSK_IRQ_WARNING_IRQ     0x00000004U
#define DRV_CANCTRL_MSK_IRQ_OVERRUN_IRQ     0x00000008U
#define DRV_CANCTRL_MSK_IRQ_ERR_PASSIVE_IRQ 0x00000020U
#define DRV_CANCTRL_MSK_IRQ_ARB_LOST_IRQ    0x00000040U
#define DRV_CANCTRL_MSK_IRQ_BUS_ERROR_IRQ   0x00000080U

/* Mode register */
#define DRV_CANCTRL_MSK_MODE_RESET_MODE      0x00000001U
#define DRV_CANCTRL_MSK_MODE_LISTEN_MODE     0x00000002U
#define DRV_CANCTRL_MSK_MODE_SELFTEST        0x00000004U
#define DRV_CANCTRL_MSK_MODE_ACCEPTANCE_MODE 0x00000008U

/* Status register */
#define DRV_CANCTRL_MSK_STATUS_RX_BUF_STATUS 0x00000001U
#define DRV_CANCTRL_MSK_STATUS_OVERRUN       0x00000002U
#define DRV_CANCTRL_MSK_STATUS_TX_BUF_STATUS 0x00000004U
#define DRV_CANCTRL_MSK_STATUS_TX_COMPLETE   0x00000008U
#define DRV_CANCTRL_MSK_STATUS_RX_STATUS     0x00000010U
#define DRV_CANCTRL_MSK_STATUS_TX_STATUS     0x00000020U
#define DRV_CANCTRL_MSK_STATUS_ERROR_STATUS  0x00000040U
#define DRV_CANCTRL_MSK_STATUS_BUS_STATUS    0x00000080U

/*****************************************************************************/
/*! CAN Single Filter Structure                                              */
/*****************************************************************************/
typedef struct DRV_CANCTRL_SINGLEFILTER_Ttag
{
  uint32_t ulAcceptanceCode; /**< Acceptance Code */
  uint32_t ulAcceptanceMask; /**< Acceptance Mask */
} DRV_CANCTRL_SINGLEFILTER_T;

/*****************************************************************************/
/*! CAN Dual Filter Structure                                                */
/*****************************************************************************/
typedef struct DRV_CANCTRL_DUALFILTER_Ttag
{
  uint16_t usAcceptanceCode0; /**< Acceptance Code 0 */
  uint16_t usAcceptanceMask0; /**< Acceptance Mask 0 */
  uint16_t usAcceptanceCode1; /**< Acceptance Code 1 */
  uint16_t usAcceptanceMask1; /**< Acceptance Mask 1 */
} DRV_CANCTRL_DUALFILTER_T;

/*****************************************************************************/
/*! CAN Message Structure                                                    */
/*****************************************************************************/
typedef struct DRV_CANCTRL_MESSAGE_Ttag
{
  unsigned int uFrameFormat;   /**< 1/0: Extended/Standard identifier */
  unsigned int uRtr;           /**< 1/0: Remote / Data Frame          */
  unsigned int uDlc;           /**< Data Length Code                  */
  uint32_t     ulIdentifier;   /**< 11 bits for standard frame, 29 bits for extended frame, LSB aligned */
  uint8_t      abData[8];      /**< Frame Data                        */
} DRV_CANCTRL_MESSAGE_T;

/*****************************************************************************/
/*! Result Codes for CAN Controller Functions                                */
/*****************************************************************************/
typedef enum DRV_CANCTRL_RESULTtag
{
  DRV_CANCTRL_RESULT_OKAY = 0,             /**< Successful                                      */
  DRV_CANCTRL_RESULT_INVAL_PARAM,          /**< Invalid parameter                               */
  DRV_CANCTRL_RESULT_NO_MESSAGE_AVAILABLE, /**< No message in receive buffer                    */
  DRV_CANCTRL_RESULT_MESSAGE_DROPPED,      /**< Message dropped due low receive buffer          */
  DRV_CANCTRL_RESULT_TX_BUFFER_NOT_FREE,   /**< No free transmission buffer for new message     */
  DRV_CANCTRL_RESULT_WRONG_MODE            /**< Operation only permitted in reset mode but core is in operating mode or vice versa  */
} DRV_CANCTRL_RESULT;

/*****************************************************************************/
/*! Parameters IDs                                                           */
/*****************************************************************************/
typedef enum DRV_CANCTRL_PRM_Etag {
  DRV_CANCTRL_PRM_IRQEN = 0,
  DRV_CANCTRL_PRM_BT0,
  DRV_CANCTRL_PRM_BT1,
  DRV_CANCTRL_PRM_ERR_WARNING_LIMIT,
  DRV_CANCTRL_PRM_RXERR_CNT,
  DRV_CANCTRL_PRM_TXERR_CNT,
  DRV_CANCTRL_PRM_ACCEPTANCE_CODE0,
  DRV_CANCTRL_PRM_ACCEPTANCE_CODE1,
  DRV_CANCTRL_PRM_ACCEPTANCE_CODE2,
  DRV_CANCTRL_PRM_ACCEPTANCE_CODE3,
  DRV_CANCTRL_PRM_ACCEPTANCE_MASK0,
  DRV_CANCTRL_PRM_ACCEPTANCE_MASK1,
  DRV_CANCTRL_PRM_ACCEPTANCE_MASK2,
  DRV_CANCTRL_PRM_ACCEPTANCE_MASK3
} DRV_CANCTRL_PRM_E;

/*****************************************************************************/
/*! Parameters IDs                                                           */
/*****************************************************************************/
typedef enum DRV_CANCTRL_DEVICE_ID_Etag {
  DRV_CANCTRL_DEVICE_ID_0 = 0u,
  DRV_CANCTRL_DEVICE_ID_1 = 0u,
} DRV_CANCTRL_DEVICE_ID_E;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
#ifndef HALDEC
#define HALDEC(name) DRV_##name
#endif

DRV_CANCTRL_RESULT        HALDEC(CanCtrl_EnterResetMode)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_LeaveResetMode)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_EnterListenMode)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_LeaveListenMode)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_EnterSelftestMode)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_LeaveSelftestMode)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SetBaudrate)( unsigned int uiInst, uint32_t ulBaudrate, unsigned int uOversampling );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SetSingleAcceptanceFilter)( unsigned int uiInst, DRV_CANCTRL_SINGLEFILTER_T* ptFilter );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_GetSingleAcceptanceFilter)( unsigned int uiInst, DRV_CANCTRL_SINGLEFILTER_T* ptFilter );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SetDualAcceptanceFilter)( unsigned int uiInst, DRV_CANCTRL_DUALFILTER_T* ptFilter );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_GetDualAcceptanceFilter)( unsigned int uiInst, DRV_CANCTRL_DUALFILTER_T* ptFilter );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SendMessage)( unsigned int uiInst, DRV_CANCTRL_MESSAGE_T* ptMessage, bool fSingleShot, bool fSelfRx );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_AbortMessage)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_ReceiveMessage)( unsigned int uiInst, DRV_CANCTRL_MESSAGE_T* ptMessage );
uint32_t                  HALDEC(CanCtrl_GetRxMessageCnt)( unsigned int uiInst );
uint32_t                  HALDEC(CanCtrl_GetMode)( unsigned int uiInst );
uint32_t                  HALDEC(CanCtrl_GetStatus)( unsigned int uiInst );
uint32_t                  HALDEC(CanCtrl_GetAndClearIrqStatus)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SetIrqMask)( unsigned int uiInst, uint32_t ulIrqMask );
uint32_t                  HALDEC(CanCtrl_GetIrqMask)( unsigned int uiInst );
uint32_t                  HALDEC(CanCtrl_GetAndClearArblostStatus)( unsigned int uiInst );
uint32_t                  HALDEC(CanCtrl_GetAndClearErrorCode)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SetWarningLimit)( unsigned int uiInst, uint32_t ulLimit );
uint32_t                  HALDEC(CanCtrl_GetWarningLimit)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SetRxErrorCnt)( unsigned int uiInst, uint32_t ulCnt );
uint32_t                  HALDEC(CanCtrl_GetRxErrorCnt)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SetTxErrorCnt)( unsigned int uiInst, uint32_t ulCnt );
uint32_t                  HALDEC(CanCtrl_GetTxErrorCnt)( unsigned int uiInst );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_SetPrm)( unsigned int uiInst, DRV_CANCTRL_PRM_E ePrmID, uint32_t ulPrmVal );
DRV_CANCTRL_RESULT        HALDEC(CanCtrl_GetPrm)( unsigned int uiInst, DRV_CANCTRL_PRM_E ePrmID, uint32_t* pulPrmVal );

/*! \} *//* End of group CANCTRL */

/*! \} *//* End of group Legacy */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __NETX_DRV_CANCTRL_H */
