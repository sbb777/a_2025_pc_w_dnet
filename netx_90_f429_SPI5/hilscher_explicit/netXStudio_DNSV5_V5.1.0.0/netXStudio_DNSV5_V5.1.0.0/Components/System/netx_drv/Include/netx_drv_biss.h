/*!************************************************************************//**
 * \file    netx_drv_biss.h
 * \brief   Header file of BISS controller module.
 * \warning
 * WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * The BISS does not work out of the box or via hardware config!
 * $Revision: 6027 $
 * $Date: 2019-08-09 14:15:52 +0200 (Fr, 09 Aug 2019) $
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
#ifndef __NETX_DRV_BISS_H_
#define __NETX_DRV_BISS_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*! \addtogroup Legacy Legacy Drivers
 * \{
 */

/*!
 * \addtogroup BISS
 * \{
 */

#include <stdint.h>
#include <stdbool.h>

#define HW_PTR_BISS(var)             static NX90_BISS_AREA_T*              const var[] = \
  {\
    (NX90_BISS_AREA_T*) Addr_NX90_biss0_app,\
    (NX90_BISS_AREA_T*) Addr_NX90_biss1_app\
  };

#define HW_PTR_BISS_CTRL(var)             static NX90_BISS_CTRL_AREA_T*              const var[] = \
  {\
    (NX90_BISS_CTRL_AREA_T*) Addr_NX90_biss_ctrl0_app,\
    (NX90_BISS_CTRL_AREA_T*) Addr_NX90_biss_ctrl1_app\
  };

/****************************************************************************************
 *  custom/other stuff begin
 ****************************************************************************************/

typedef struct CYCLE_Ttag
{
  /* prevent optimizer ripping out important reads and writes here */
  volatile uint32_t ulCycCount;
  volatile uint32_t ulFrmErrors;
  volatile uint32_t ulCrcErrors;
  volatile uint64_t ullInputData;
  uint64_t ullInputPositionMask;
  uint64_t ullInputPositionRightShift;
} CYCLE_T;
/****************************************************************************************
 *  custom/other stuff end
 ****************************************************************************************/

/***********************************************************************************************/
/*! BiSS HAL Configuration
 * \description
 *    BiSS interface configuration                                                             */
/***********************************************************************************************/
typedef struct DRV_BISSM_CFG_Ttag
{
  unsigned int uGpioIn; /**< number of GPIO pin used as SL input */
} DRV_BISSM_CFG_T;

/*****************************************************************************/
/*! BiSS Interface Configuration
 * \description
 *    Enumeration interface configuration.                                    */
/*****************************************************************************/
typedef enum DRV_BISSM_CFG_IF_Etag
{
  DRV_BISSM_CFG_IF_DEFAULT = 0, /**< Operate as BiSS C-Mode Master */
  DRV_BISSM_CFG_IF_DISABLE_CDS, /**< Default with B-Mode sensor communication */
  DRV_BISSM_CFG_IF_SSI /**< Operate as SSI Master */
} DRV_BISSM_CFG_IF_E;

/*****************************************************************************/
/*! BiSS Interrupts
 * \description
 *    Enumeration interrupt requests.                                         */
/*****************************************************************************/
typedef enum DRV_BISSM_IRQ_Etag
{
  DRV_BISSM_IRQ_DATA = 0x01, /**< BiSS Frame finished, new state can be read with DRV_BissM_GetState(), new process data can be read with DRV_BissM_GetProccesData() */
  DRV_BISSM_IRQ_RESET = 0x02, /**< BiSS communication stopped, DRV_BissM_SetParam(), DRV_BissM_SetChannelParam() and DRV_BissM_Start() may be called again */
  DRV_BISSM_IRQ_CTRL = 0x04 /**< Control frame finished, DRV_BissM_GetCtrlData() may be called */
} DRV_BISSM_IRQ_E;

/***********************************************************************************************/
/*! BiSS Global Parameters
 * \description
 *    These parameters can be set by DRV_BissM_SetParam() for global DRV_BISSM configuration             */
/***********************************************************************************************/
typedef enum DRV_BISSM_PARAM_Etag
{
  DRV_BISSM_PARAM_CFG_IF = 0, /**< Interface configuration, possible values are defined by DRV_BISSM_CFG_IF_E */
  DRV_BISSM_PARAM_PERIOD, /**< For one clock in BiSS frame in 10ns */
  DRV_BISSM_PARAM_CYCLE_TIME, /**< Time lag between two BiSS frame cycles, a value of 0 enables the manual mode */
  DRV_BISSM_PARAM_BUSY_TIME, /**< Processing time for sensor data (in clocks) */
  DRV_BISSM_PARAM_CTRL_TIMEOUT, /**< Processing time for register access (in cycles) */
  DRV_BISSM_PARAM_IRQ_MASK, /**< Interrupt enable mask, possible values are defined by DRV_BISSM_IRQ_E */
  DRV_BISSM_PARAM_SSI_TSAR, /**< SSI mode tSAR: Processing time for sensor data (in clocks) */
  DRV_BISSM_PARAM_TRGCFG, /**< Trigger configuration: Event selector to start the BiSS cycle in automatic mode */
  DRV_BISSM_PARAM_TRGMAN /**< Trigger manually: Set to 1 to start the BiSS cycle manually in automatic mode */
} DRV_BISSM_PARAM_E;

/*****************************************************************************/
/*! BiSS Data Active Configuration
 * \description
 *    Enumeration data active configuration.                                  */
/*****************************************************************************/
typedef enum DRV_BISSM_DATA_ACTIVE_Etag
{
  DRV_BISSM_DATA_ACTIVE_INACTIVE = 0, /**< Inactive during current control frame      */
  DRV_BISSM_DATA_ACTIVE_ACTIVE_PRIOR_CTRL, /**< Active until first access in control frame */
  DRV_BISSM_DATA_ACTIVE_ACTIVE_AFTER_CTRL, /**< Active after first access in control frame */
  DRV_BISSM_DATA_ACTIVE_ACTIVE /**< Active during current control frame        */
} DRV_BISSM_DATA_ACTIVE_E;

/*****************************************************************************/
/*! BiSS Data Type Configuration
 * \description
 *    Enumeration data type configuration.                                    */
/*****************************************************************************/
typedef enum DRV_BISSM_DATA_TYPE_Etag
{
  DRV_BISSM_DATA_TYPE_SENS = 0, /**< Single cycle sensor data   */
  DRV_BISSM_DATA_TYPE_ACT /**< Single cycle actuator data */
} DRV_BISSM_DATA_TYPE_E;

/*****************************************************************************/
/*! BiSS Data Align Configuration
 * \description
 *    Enumeration data align configuration.                                   */
/*****************************************************************************/
typedef enum DRV_BISSM_DATA_ALIGN_Etag
{
  DRV_BISSM_DATA_ALIGN_LEFT = 0, /**< "101" is stored as "1010000" */
  DRV_BISSM_DATA_ALIGN_RIGHT /**< "101" is stored as "0000101" */
} DRV_BISSM_DATA_ALIGN_E;

/***********************************************************************************************/
/*! BiSS Channel Parameters
 * \description
 *    These parameters can be set by DRV_BissM_SetChannelParam() for each data channel              */
/***********************************************************************************************/
typedef enum DRV_BISSM_CHANNEL_PARAM_Etag
{
  DRV_BISSM_CHANNEL_PARAM_DATA_ACTIVE = 0, /**< Data channel used in current frame     */
  DRV_BISSM_CHANNEL_PARAM_DATA_TYPE, /**< Sensor or actuator data                */
  DRV_BISSM_CHANNEL_PARAM_DATA_LEN, /**< Sensor or actuator data length         */
  DRV_BISSM_CHANNEL_PARAM_DATA_ALIGN, /**< Sensor or actuator data alignment      */
  DRV_BISSM_CHANNEL_PARAM_CRC_POLY, /**< CRC polynomial                         */
  DRV_BISSM_CHANNEL_PARAM_CRC_START, /**< Start value for CRC calculation        */
  DRV_BISSM_CHANNEL_PARAM_EXT_DATA_LEN, /**< SSI Extended: data length,
   set to zero to disable extended mode   */
  DRV_BISSM_CHANNEL_PARAM_EXT_CRC_POLY, /**< SSI Extended: 2nd CRC polynomial       */
  DRV_BISSM_CHANNEL_PARAM_EXT_CRC_START /**< SSI Extended: Start value for 2nd CRC  */
} DRV_BISSM_CHANNEL_PARAM_E;

/*****************************************************************************/
/*! Functions Result Codes
 * \description
 *    All functions return one of the following values after returning from
 *    the function call. Function return values shall always be evaluated
 *    by the calling function.                                                */
/*****************************************************************************/
typedef enum
{
  DRV_BISSM_OK, /**< Successful                                   */
  DRV_BISSM_ERR_INIT_FAILED, /**< Error occurred during initialization         */
  DRV_BISSM_ERR_INVAL_PARAMETER, /**< Invalid parameter in function call           */
  DRV_BISSM_ERR_INVAL_STATE /**< Function call not allowed in current state   */
} DRV_BISSM_RESULT_E;

/*****************************************************************************/
/*! BiSS Frame Error Codes
 * \description
 *    Error Codes for a single BiSS frame.                                    */
/*****************************************************************************/
typedef enum
{
  DRV_BISSM_FRM_OK = 0, /**< no error */
  DRV_BISSM_FRM_ERR_SLAVE_NRDY = 1, /**< SL is low at frame start */
  DRV_BISSM_FRM_ERR_ACK_TIMEOUT = 2, /**< SL stayed high after frame start until configured timeout */
  DRV_BISSM_FRM_ERR_ACK_INVALID = 3, /**< SL was high 1/2 period after falling edge */
  DRV_BISSM_FRM_ERR_BUSY_TIMEOUT = 4, /**< SL stayed low after falling edge until configured timeout */
  DRV_BISSM_FRM_ERR_STOP_INVALID = 7 /**< SL was high at end of frame */
} DRV_BISSM_FRM_ERR_E;

/*****************************************************************************/
/*! Global BiSS State
 * \description
 *    Global BiSS status.                                                     */
/*****************************************************************************/
typedef struct DRV_BISSM_STATE_Ttag
{
  bool fBusy; /**< true: Master is running
   false: Master stopped, no BiSS frames are transmitted */
  uint16_t usCycleCnt; /**< cycle counter */
  DRV_BISSM_FRM_ERR_E eError; /**< BiSS frame error code of latest cycle */
  bool fSlaveRdy; /**< true: SL is high (idle), master may start next BiSS frame
   false: SL is low (busy), BiSS timeout not yet reached */
} DRV_BISSM_STATE_T;

/*****************************************************************************/
typedef enum
{
  DRV_BISSM_TRIGGER_TYPE_NONE,
  DRV_BISSM_TRIGGER_TYPE_MANUAL,
  DRV_BISSM_TRIGGER_TYPE_XC_TRIGGER_OUT0,
  DRV_BISSM_TRIGGER_TYPE_XC_TRIGGER_OUT0_INV,
  DRV_BISSM_TRIGGER_TYPE_XC_TRIGGER_OUT1,
  DRV_BISSM_TRIGGER_TYPE_XC_TRIGGER_OUT1_INV,
  DRV_BISSM_TRIGGER_TYPE_XC_SAMPLE_IN0,
  DRV_BISSM_TRIGGER_TYPE_XC_SAMPLE_IN0_INV,
  DRV_BISSM_TRIGGER_TYPE_XC_SAMPLE_IN1,
  DRV_BISSM_TRIGGER_TYPE_XC_SAMPLE_IN1_INV,
  DRV_BISSM_TRIGGER_TYPE_GPIO_APP_COUNTER_ZERO0,
  DRV_BISSM_TRIGGER_TYPE_GPIO_APP_COUNTER_ZERO1,
  DRV_BISSM_TRIGGER_TYPE_GPIO_APP_COUNTER_ZERO2
} DRV_BISSM_TRIGGER_TYPE_E;

/*****************************************************************************/
/*! Control Frame Error Codes
 * \description
 *    Error Codes of an acyclic control frame.                                */
/*****************************************************************************/
typedef enum DRV_BISSM_CTRL_RESULT_Etag
{
  DRV_BISSM_CTRL_OK = 0, /**< Control State = OK               */
  DRV_BISSM_CTRL_ERR_ID_LOCK_INVALID, /**< Detection of invalid id lock     */
  DRV_BISSM_CTRL_ERR_NO_ID_ACK, /**< No ID acknowledge detected       */
  DRV_BISSM_CTRL_ERR_ADDR_NOT_IMPLEMENTED, /**< Register address not implemented */
  DRV_BISSM_CTRL_ERR_DATA_OR_CRC_ERR, /**< Detection of data or CRC error   */
  DRV_BISSM_CTRL_ERR_TIMEOUT, /**< Slave reply timed out according to DRV_BISSM_PARAM_CTRL_TIMEOUT */
  DRV_BISSM_CTRL_ERR_ABORT, /**< Control frame is aborted         */
  DRV_BISSM_CTRL_ERR_BISS_FRAME_ERR /**< BiSS frame error detected        */
} DRV_BISSM_CTRL_RESULT_E;

/*****************************************************************************/
/*! Control State
 * \description
 *    Global status of the acyclic control state machine.                     */
/*****************************************************************************/
typedef struct DRV_BISSM_CTRL_STATE_Ttag
{
  bool fBusy; /**< true: control frame not yet finished, eError is invalid
   false: control frame finished, eError is valid */
  DRV_BISSM_CTRL_RESULT_E eError; /**< BiSS control error code                */
  unsigned int uIdLock; /**< measured ID_LOCK bits */
  unsigned int uRegCnt; /**< number of valid register accesses */
} DRV_BISSM_CTRL_STATE_T;

/*****************************************************************************/
/*! Channel Error Codes
 * \description
 *    Error Codes of single data channel provided with the process data.      */
/*****************************************************************************/
typedef enum DRV_BISSM_DATA_STATE_Etag
{
  DRV_BISSM_DATA_OK = 0, /**< State = OK                 */
  DRV_BISSM_DATA_ERR_FRM, /**< BiSS frame is erroneous    */
  DRV_BISSM_DATA_ERR_CRC /**< BiSS frame CRC is invalid  */
} DRV_BISSM_DATA_STATE_E;

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

DRV_BISSM_RESULT_E DRV_BissM_Init(DRV_BISSM_CFG_T* ptConfig, void* pvUser);

DRV_BISSM_RESULT_E DRV_BissM_SetParam(DRV_BISSM_PARAM_E ePrmID, uint32_t ulPrmVal);

DRV_BISSM_RESULT_E DRV_BissM_SetChannelParam(unsigned int uChNr, DRV_BISSM_CHANNEL_PARAM_E ePrmID, uint32_t ulPrmVal);

DRV_BISSM_RESULT_E DRV_BissM_SetProcessData(unsigned int uChNr, const uint8_t *pbData);

DRV_BISSM_RESULT_E DRV_BissM_GetProcessData(unsigned int uChNr, DRV_BISSM_DATA_STATE_E *peStatus, uint16_t *pusCycleCnt, uint8_t *pbData);

DRV_BISSM_RESULT_E DRV_BissM_GetSsiExtData(unsigned int uChNr, DRV_BISSM_DATA_STATE_E *peStatus, uint16_t *pusCycleCnt, uint8_t *pbData);

DRV_BISSM_RESULT_E DRV_BissM_StartTrigger(DRV_BISSM_TRIGGER_TYPE_E eTriggerCfg);

DRV_BISSM_RESULT_E DRV_BissM_StartAgsMin(void);

DRV_BISSM_RESULT_E DRV_BissM_StartCycle(uint32_t ulCycleTimeUs);

DRV_BISSM_RESULT_E DRV_BissM_Stop();

DRV_BISSM_RESULT_E DRV_BissM_GetState(DRV_BISSM_STATE_T* ptState);

DRV_BISSM_RESULT_E DRV_BissM_GetCtrlState(DRV_BISSM_CTRL_STATE_T* ptCtrlState);

DRV_BISSM_RESULT_E DRV_BissM_StartCtrlCmd(unsigned int uSlaveId, unsigned int uOpcode);

DRV_BISSM_RESULT_E DRV_BissM_StartCtrlReg(unsigned int uSlaveId, unsigned int uRegAdr, unsigned int uRegNr, unsigned int uWnR);

DRV_BISSM_RESULT_E DRV_BissM_CtrlRst(void);

DRV_BISSM_RESULT_E DRV_BissM_SetCtrlData(unsigned int uRegNr, const uint8_t *pbData);

DRV_BISSM_RESULT_E DRV_BissM_GetCtrlData(unsigned int uRegNr, uint8_t *pbData);

uint32_t DRV_BissM_GetIrqRaw(void);

void DRV_BissM_ConfirmIrq(uint32_t ulIrqMsk);

uint32_t DRV_BissM_GetIrq(void);

void DRV_BissM_IrqEnable(uint32_t ulIrqMsk);

void DRV_BissM_IrqDisable(uint32_t ulIrqMsk);

uint32_t DRV_BissM_GetConfirmIrq(void);

void DRV_BissM_ConfirmAllIrq(void);

void DRV_BissM_Break(void);

uint32_t DRV_BissM_Status0(void);

/*! \} *//* End of group BISS */

/*! \} *//* End of group Legacy */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif

#endif /* NETX_DRV_BISS_H_ */
