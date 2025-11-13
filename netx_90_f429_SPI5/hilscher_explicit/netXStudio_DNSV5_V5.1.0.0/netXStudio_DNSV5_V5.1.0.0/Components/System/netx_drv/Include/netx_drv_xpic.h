/*!************************************************************************//*!
 * \file     netx_drv_xpic.h
 * \brief     peripheral module driver.
 * $Revision: 4646 $
 * $Date: 2018-12-19 08:47:36 +0100 (Mi, 19 Dez 2018) $
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

/* Define to prevent recursive inclusion -------------------------------------*/
#include "netx_drv.h"
#ifndef __DRV_XPIC_H
#define __DRV_XPIC_H

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
 * \addtogroup XPIC
 * \{
 */

#include <stdint.h>
#include <stdbool.h>

typedef struct DRV_XPIC_WATCHPOINT_Ttag
{
  uint32_t  ulAddrValue;
  uint32_t  ulAddrDcMsk;
  uint32_t  ulDataValue;
  uint32_t  ulDataDcMsk;
  uint32_t  ulCtrlValue;
  uint32_t  ulCtrlDcMsk;
} DRV_XPIC_WATCHPOINT_T;

typedef enum
{
  DRV_XPIC_REG_R0   = 0,
  DRV_XPIC_REG_R1   = 1,
  DRV_XPIC_REG_R2   = 2,
  DRV_XPIC_REG_R3   = 3,
  DRV_XPIC_REG_R4   = 4,
  DRV_XPIC_REG_R5   = 5,
  DRV_XPIC_REG_R6   = 6,
  DRV_XPIC_REG_R7   = 7,
  DRV_XPIC_REG_USR0 = 8,
  DRV_XPIC_REG_USR1 = 9,
  DRV_XPIC_REG_USR2 = 10,
  DRV_XPIC_REG_USR3 = 11,
  DRV_XPIC_REG_USR4 = 12,
  DRV_XPIC_REG_PC   = 13,
  DRV_XPIC_REG_STAT = 14,
  DRV_XPIC_REG_ZERO = 15
} DRV_XPIC_REG_E;


/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

void     DRV_XPIC_Reset( unsigned int uiXpicNo, void* pvUser );
void     DRV_XPIC_Load( unsigned int uiXpicNo, const uint32_t *pulXpicPrg, void* pvUser );
void     DRV_XPIC_Start( unsigned int uiXpicNo, void* pvUser );
void     DRV_XPIC_StartEx( unsigned int uiXpicNo, bool fSinglestep, bool fMonitor, bool fIntDis, bool fMisalignHold, void* pvUser);
void     DRV_XPIC_Stop( unsigned int uiXpicNo, void* pvUser );
void     DRV_XPIC_SetBankSelect( unsigned int uiXpicNo, bool fSel, bool fFiqBank);
uint32_t DRV_XPIC_GetLastPc( unsigned int uiXpicNo );
uint32_t DRV_XPIC_GetIrqStatus( unsigned int uiXpicNo );
uint32_t DRV_XPIC_GetIrqReturnPc( unsigned int uiXpicNo );
uint32_t DRV_XPIC_GetFiqReturnPc( unsigned int uiXpicNo );
uint32_t DRV_XPIC_GetBreakStatus( unsigned int uiXpicNo );
uint32_t DRV_XPIC_GetBreakIrqRaw( unsigned int uiXpicNo );
void     DRV_XPIC_ConfirmBreakIrq( unsigned int uiXpicNo, uint32_t ulIrq );
uint32_t DRV_XPIC_GetBreakIrq( unsigned int uiXpicNo, bool fOwn );
void     DRV_XPIC_EnableBreakIrq( unsigned int uiXpicNo, uint32_t ulIrq, bool fOwn );
void     DRV_XPIC_DisableBreakIrq( unsigned int uiXpicNo, uint32_t ulIrq, bool fOwn );
void     DRV_XPIC_SetupWatchPoint( unsigned int uiXpicNo, unsigned int uWpNo, DRV_XPIC_WATCHPOINT_T* ptWp );
uint32_t DRV_XPIC_GetReg( unsigned int uiXpicNo, DRV_XPIC_REG_E eReg );
void     DRV_XPIC_SetReg( unsigned int uiXpicNo, DRV_XPIC_REG_E eReg, uint32_t ulValue );
void     DRV_XPIC_WrDramDw( unsigned int uiXpicNo, unsigned int uiOffset, uint32_t ulValue );
uint32_t DRV_XPIC_RdDramDw( unsigned int uiXpicNo, unsigned int uiOffset );
void     DRV_XPIC_WrPramDw( unsigned int uiXpicNo, unsigned int uiOffset, uint32_t ulValue );
uint32_t DRV_XPIC_RdPramDw( unsigned int uiXpicNo, unsigned int uiOffset );



/*! \} *//* End of group XPIC */

/*! \} *//* End of group Legacy */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif
#endif /* #ifndef __DRV_XPIC_H */
