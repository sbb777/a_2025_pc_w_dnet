/**************************************************************************//**
 * \file     netx_drv_xpic.c
 * \brief    XPIC module driver.
 * $Revision: 6592 $
 * $Date: 2019-12-17 14:14:57 +0100 (Di, 17 Dez 2019) $
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
#include "netx_drv_user_conf.h"
#ifdef DRV_XPIC_MODULE_ENABLED

/*! \defgroup XPIC XPIC
 * \{
 * \brief XPIC module driver
 *
 * \details This driver is one of our legacy HALs. The support for the legacy hall's is
 * very limited. They are developed and maintained by our chip development and
 * verification department and provide only a hardware near abstraction layer. They
 * are in most cases not convenient or do not support as much functionality as that
 */
/*****************************************************************************/
/*  Includes                                                                 */
/*****************************************************************************/

#ifdef DRV_DEVICE_NETX_90_MPW
#error "The MPW chip is no longer supported!"
#else
#ifdef DRV_DEVICE_NETX_90
#include "regdef/netx90_app/regdef_netx90_arm_app.h"
#else
#error "No chip Selected!"
#endif
#endif
#include "netx_drv_xpic.h"
#include <string.h> /* definition of NULL */

/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/

#define DRV_XPIC_SEGDEF_HEADER_MAGIC              0x43495058
#define DRV_XPIC_SEGDEF_HEADER_MSK_VERSION_MAJOR  0xffff0000
#define DRV_XPIC_SEGDEF_HEADER_MSK_VERSION_MINOR  0x0000ffff
#define DRV_XPIC_SEGDEF_HEADER_VERSION_MAJOR      0x00010000
#define DRV_XPIC_SEGDEF_HEADER_VERSION_MINOR      0x00000000

/*****************************************************************************/
/*  Variables                                                                */
/*****************************************************************************/

static NX90_XPIC_AREA_T* const s_aptXpic[] = { (NX90_XPIC_AREA_T*) Addr_NX90_xpic_app_regs };
static NX90_XPIC_DEBUG_AREA_T* const s_aptXpicDebug[] = { (NX90_XPIC_DEBUG_AREA_T*) Addr_NX90_xpic_app_debug };
static uint32_t* const s_apulXpicDram[] = { (uint32_t*) Addr_NX90_xpic_app_dram };
static uint32_t* const s_apulXpicPram[] = { (uint32_t*) Addr_NX90_xpic_app_pram };

#ifndef _HW_CONCAT
#define _HW_CONCAT(a,b) a ## b
#define HW_MSK(bf)         _HW_CONCAT(MSK_NX90_, bf)
#define HW_SRT(bf)         _HW_CONCAT(SRT_NX90_, bf)
#define HW_DFLT_BF_VAL(bf) _HW_CONCAT(DFLT_BF_VAL_NX90_, bf)
#define HW_DFLT_VAL(reg)   _HW_CONCAT(DFLT_VAL_NX90_, reg)
#define HW_REGADR(reg)     _HW_CONCAT(Adr_NX90_, reg)
#define HW_AREAADR(area)   _HW_CONCAT(Addr_NX90_, area)
#define HW_TYPE(area)      _HW_CONCAT(NX90_, area)
#endif

/*****************************************************************************/
/*  Functions                                                                */
/*****************************************************************************/

/*****************************************************************************/
/*! Load data/code segments
 * \description
 *   Load dword aligned data into memory
 *   Can be used for xPIC program or any other areas
 * \class
 *   XPIC
 * \params
 *   pulData [in]  Pointer to microcode
 *                                                                            */
/*****************************************************************************/

static void DRV_XPIC_LoadSegments(unsigned int uiXpicNo, const uint32_t *pulData)
{
  const uint32_t *pulCur = &pulData[2];
  uint32_t* pulDest;
  uint32_t ulLen;

  /* check header magic */
  if(*pulCur++ != DRV_XPIC_SEGDEF_HEADER_MAGIC)
  {
    /* invalid header */
    return;
  }
  /* check major version */
  if((*pulCur++ & DRV_XPIC_SEGDEF_HEADER_MSK_VERSION_MAJOR) > DRV_XPIC_SEGDEF_HEADER_VERSION_MAJOR)
  {
    /* version not supported */
    return;
  }

  pulDest = (uint32_t*) *pulCur++;
  ulLen = *pulCur++;
  while(ulLen)
  {
    /* load segment */
    while(ulLen--)
    {
      *pulDest++ = *pulCur++;
    }
    /* read next segment */
    pulDest = (uint32_t*) *pulCur++;
    ulLen = *pulCur++;
  }
}

/*****************************************************************************/
/*! Load xPIC code
 * \description
 *   Load xPIC code,
 *   Initialize trailing loads
 * \class
 *   XPIC
 * \params
 *   xpic_code [in]  Pointer to microcode
 *   pvUser    [in] ignored
 *                                                                            */
/*****************************************************************************/

void DRV_XPIC_Load(unsigned int uiXpicNo, const uint32_t *xpic_code, void* pvUser)
{
  unsigned int programm_size;
  unsigned int tl_size;
  unsigned int programm_start_adr;
  unsigned int i;
  unsigned int tl_start;
  uint32_t* pulData;

  if((xpic_code[0] == 0) && (xpic_code[1] == 0))
  {
    /* xPIC GNU Toolchain - segment definitions */
    DRV_XPIC_LoadSegments(uiXpicNo, xpic_code);
  }
  else
  {
    /* xpicass generated program and trailing loads */
    programm_size = xpic_code[0] / 4; /* in Bytes / 4 */
    tl_size = xpic_code[1] / 4;
    programm_start_adr = xpic_code[2];
    tl_start = programm_size + 2;

    /* program loader */
    for(i = 1; i < programm_size; i++)
    {
      pulData = (uint32_t*) (programm_start_adr + (i - 1) * 4);
      *pulData = xpic_code[i + 2];
    }

    /* trailing loader */
    for(i = 0; i < tl_size; i = i + 2)
    {
      pulData = (uint32_t*) (xpic_code[tl_start + i]);
      *pulData = xpic_code[tl_start + i + 1];
    }
  }
}

/*****************************************************************************/
/*! Start xPIC
 * \description
 *   Start xPIC
 * \class
 *   XPIC
 * \params
 *   pvUser [in] ignored
 *                                                                            */
/*****************************************************************************/
void DRV_XPIC_Start(unsigned int uiXpicNo, void* pvUser)
{
  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = 0;
}

/*****************************************************************************/
/*! Stop xPIC
 * \description
 *   Halt xPIC
 * \class
 *   XPIC
 * \params
 *   pvUser [in] ignored
 *                                                                            */
/*****************************************************************************/
void DRV_XPIC_Stop(unsigned int uiXpicNo, void* pvUser)
{
  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = HW_MSK(xpic_hold_pc_hold);
}

/*****************************************************************************/
/*! Reset xPIC
 * \description
 *   Hold xPIC, reset xPIC state,
 *   clear work registers (both banks), clear user registers
 * \class
 *   XPIC
 * \params
 *   pvUser [in] ignored
 *
 *                                                                            */
/*****************************************************************************/
void DRV_XPIC_Reset(unsigned int uiXpicNo, void* pvUser)
{
  uint32_t ulVal;

  /* hold xPIC */
  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = MSK_NX90_xpic_hold_pc_hold;

  /* clear all hold reasons */
  s_aptXpicDebug[uiXpicNo]->ulXpic_break_irq_raw = 0x1f;

  /* hold xPIC and request reset */
  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = HW_MSK(xpic_hold_pc_hold) | HW_MSK(xpic_hold_pc_reset_xpic);

  /* wait for reset to be finished */
  do
  {
    ulVal = s_aptXpicDebug[uiXpicNo]->ulXpic_break_status;
  } while((ulVal & HW_MSK(xpic_break_status_xpic_reset_status)) == 0);

  /* release reset request, engage bank control, select bank 1 */
  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = HW_MSK(xpic_hold_pc_hold) | HW_MSK(xpic_hold_pc_bank_control) | HW_MSK(xpic_hold_pc_bank_select);

  /* reset xPIC user registers */
  for(ulVal = 0; ulVal < 5; ulVal++)
  {
    s_aptXpic[uiXpicNo]->aulXpic_usr[ulVal] = 0;
  }

  /* reset xPIC work registers (bank 1) */
  for(ulVal = 0; ulVal < 8; ulVal++)
  {
    s_aptXpic[uiXpicNo]->aulXpic_r[ulVal] = 0;
  }

  /* select bank 0 */
  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = HW_MSK(xpic_hold_pc_hold) | HW_MSK(xpic_hold_pc_bank_control);

  /* reset xPIC work registers (bank 0) */
  for(ulVal = 0; ulVal < 8; ulVal++)
  {
    s_aptXpic[uiXpicNo]->aulXpic_r[ulVal] = 0;
  }

  /* release bank control */
  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = HW_MSK(xpic_hold_pc_hold);
}
/*****************************************************************************/
/*! Get Previous Program Counter
 * \description
 *  Read the xPIC last program counter register,
 *  i. e. the program counter of the last executed instruction.
 * \class
 *   XPIC
 * \params
 * \return
 *   raw register value                                                       */
/*****************************************************************************/
uint32_t DRV_XPIC_GetLastPc(unsigned int uiXpicNo)
{
  return s_aptXpicDebug[uiXpicNo]->ulXpic_break_last_pc;
}

/*****************************************************************************/
/*! Get xPIC Break Status
 * \description
 *  Read the xPIC break status register.
 * \class
 *   XPIC
 * \params
 * \return
 *   raw register value                                                       */
/*****************************************************************************/
uint32_t DRV_XPIC_GetBreakStatus(unsigned int uiXpicNo)
{
  return s_aptXpicDebug[uiXpicNo]->ulXpic_break_status;
}

/*****************************************************************************/
/*! Get xPIC Raw Break Interrupt
 * \description
 *  Read the xPIC raw break interrupt.
 * \class
 *   XPIC
 * \params
 * \return
 *   raw register value                                                       */
/*****************************************************************************/
uint32_t DRV_XPIC_GetBreakIrqRaw(unsigned int uiXpicNo)
{
  return s_aptXpicDebug[uiXpicNo]->ulXpic_break_irq_raw;
}

/*****************************************************************************/
/*! Confirm xPIC Break Interrupt
 * \description
 *  Clear the xPIC break interrupt.
 * \class
 *   XPIC
 * \params
 *   ulIrq [in] interrupt mask                                                */
/*****************************************************************************/
void DRV_XPIC_ConfirmBreakIrq(unsigned int uiXpicNo, uint32_t ulIrq)
{
  s_aptXpicDebug[uiXpicNo]->ulXpic_break_irq_raw = ulIrq;
}

/*****************************************************************************/
/*! Get xPIC Masked Break Interrupt
 * \description
 *  Read the xPIC masked break interrupt.
 * \class
 *   XPIC
 * \params
 * \return
 *   raw register value                                                       */
/*****************************************************************************/
uint32_t DRV_XPIC_GetBreakIrq(unsigned int uiXpicNo, bool fOwn)
{
  return (fOwn) ? s_aptXpicDebug[uiXpicNo]->ulXpic_break_own_irq_masked : s_aptXpicDebug[uiXpicNo]->ulXpic_break_irq_masked;
}

/*****************************************************************************/
/*! Enable xPIC Break Interrupt
 * \description
 *  Set bits in the xPIC break interrupt mask register.
 * \class
 *   XPIC
 * \params
 *   ulIrq [in] interrupt enable mask                                         */
/*****************************************************************************/
void DRV_XPIC_EnableBreakIrq(unsigned int uiXpicNo, uint32_t ulIrq, bool fOwn)
{
  if(fOwn)
    s_aptXpicDebug[uiXpicNo]->ulXpic_break_own_irq_msk_set = ulIrq;
  else
    s_aptXpicDebug[uiXpicNo]->ulXpic_break_irq_msk_set = ulIrq;
}

/*****************************************************************************/
/*! Disable xPIC Break Interrupt
 * \description
 *  Clear bits in the xPIC break interrupt mask register.
 * \class
 *   XPIC
 * \params
 *   ulIrq [in] interrupt disable mask                                        */
/*****************************************************************************/
void DRV_XPIC_DisableBreakIrq(unsigned int uiXpicNo, uint32_t ulIrq, bool fOwn)
{
  if(fOwn)
    s_aptXpicDebug[uiXpicNo]->ulXpic_break_own_irq_msk_reset = ulIrq;
  else
    s_aptXpicDebug[uiXpicNo]->ulXpic_break_irq_msk_reset = ulIrq;
}

/*****************************************************************************/
/*! Set up xPIC Hardware Watch Point
 * \description
 *  Setup a watchpoint for the xPIC.
 * \class
 *   XPIC
 * \params
 *   uWpNo [in] watchpoint number
 *   ptWp  [in] watchpoint configuration                                      */
/*****************************************************************************/
void DRV_XPIC_SetupWatchPoint(unsigned int uiXpicNo, unsigned int uWpNo, DRV_XPIC_WATCHPOINT_T* ptWp)
{
  if(ptWp == NULL)
  {
    /* disable watchpoint */
    s_aptXpicDebug[uiXpicNo]->asXpic_break[uWpNo].ulContr = 0;
  }
  else
  {
    s_aptXpicDebug[uiXpicNo]->asXpic_break[uWpNo].ulAddr = ptWp->ulAddrValue;
    s_aptXpicDebug[uiXpicNo]->asXpic_break[uWpNo].ulAddr_mask = ptWp->ulAddrDcMsk;
    s_aptXpicDebug[uiXpicNo]->asXpic_break[uWpNo].ulData = ptWp->ulDataValue;
    s_aptXpicDebug[uiXpicNo]->asXpic_break[uWpNo].ulData_mask = ptWp->ulDataDcMsk;
    s_aptXpicDebug[uiXpicNo]->asXpic_break[uWpNo].ulContr = ptWp->ulCtrlValue;
    s_aptXpicDebug[uiXpicNo]->asXpic_break[uWpNo].ulContr_mask = ptWp->ulCtrlDcMsk;
  }
}

/*****************************************************************************/
/*! Start xPIC Extended
 * \description
 *  Extended version of the start function.
 * \class
 *   XPIC
 * \params
 *   fSingleStep   [in] enable single stepping
 *   fMonitorMode  [in] enable monitor mode (don't halt on watchpoints, just set IRQs
 *   fIntDis       [in] disable xPIC interrupts (IRQs and FIQs)
 *   fMisalignHold [in] halt xPIC on misaligned memory accesses
 *   pvUser        [in] user defined parameter                                */
/*****************************************************************************/
void DRV_XPIC_StartEx(unsigned int uiXpicNo, bool fSingleStep, bool fMonitorMode, bool fIntDis, bool fMisalignHold, void* pvUser)
{
  uint32_t ulVal = 0;

  if(fSingleStep)
    ulVal |= HW_MSK(xpic_hold_pc_single_step);

  if(fMonitorMode)
    ulVal |= HW_MSK(xpic_hold_pc_monitor_mode);

  if(fIntDis)
    ulVal |= HW_MSK(xpic_hold_pc_disable_int);

  if(fMisalignHold)
    ulVal |= HW_MSK(xpic_hold_pc_misalignment_hold);

  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = ulVal;
}

/*****************************************************************************/
/*! Select xPIC Rgister Bank
 * \description
 *  Selects the register bank for all following register read/write accesses.
 * \class
 *   XPIC
 * \params
 *   fSel     [in] false: Use current active register bank, bank select is not controlled.
 *                 true: Use register bank as defined in fFiqBank
 *   fFiqBank [in] false: Use default/IRQ register bank
 *                 true: Use FIQ register bank                                */
/*****************************************************************************/
void DRV_XPIC_SetBankSelect(unsigned int uiXpicNo, bool fSel, bool fFiqBank)
{
  uint32_t ulValue = 0;

  if(fSel)
  {
    ulValue |= HW_MSK(xpic_hold_pc_bank_control);

    if(fFiqBank)
    {
      ulValue |= HW_MSK(xpic_hold_pc_bank_select);
    }
  }

  s_aptXpicDebug[uiXpicNo]->ulXpic_hold_pc = ulValue;
}

/*****************************************************************************/
/*! Read xPIC Rgister
 * \description
 *  Reads a processor register of the xPIC.
 * \class
 *   XPIC
 * \params
 *   eReg     [in] Register to read                                           */
/*****************************************************************************/
uint32_t DRV_XPIC_GetReg(unsigned int uiXpicNo, DRV_XPIC_REG_E eReg)
{
  return s_aptXpic[uiXpicNo]->aulXpic_r[eReg];
}

/*****************************************************************************/
/*! Write xPIC Rgister
 * \description
 *  Writes a processor register of the xPIC.
 * \class
 *   XPIC
 * \params
 *   eReg     [in] Register to write
 *   ulValue  [in] Register value                                             */
/*****************************************************************************/
void DRV_XPIC_SetReg(unsigned int uiXpicNo, DRV_XPIC_REG_E eReg, uint32_t ulValue)
{
  s_aptXpic[uiXpicNo]->aulXpic_r[eReg] = ulValue;
}

/*****************************************************************************/
/*! Get xPIC Interrupt status
 * \description
 *  Reads the interrupt status register of the xPIC.
 * \class
 *   XPIC                                                                     */
/*****************************************************************************/
uint32_t DRV_XPIC_GetIrqStatus(unsigned int uiXpicNo)
{
  return s_aptXpicDebug[uiXpicNo]->ulXpic_irq_status;
}

/*****************************************************************************/
/*! Get xPIC IRQ Return Program Counter
 * \description
 *  Reads the last program counter before jump into IRQ.
 * \class
 *   XPIC                                                                     */
/*****************************************************************************/
uint32_t DRV_XPIC_GetIrqReturnPc(unsigned int uiXpicNo)
{
  return s_aptXpicDebug[uiXpicNo]->ulXpic_break_return_irq_pc;
}

/*****************************************************************************/
/*! Get xPIC FIQ Return Program Counter
 * \description
 *  Reads the last program counter before jump into FIQ.
 * \class
 *   XPIC                                                                     */
/*****************************************************************************/
uint32_t DRV_XPIC_GetFiqReturnPc(unsigned int uiXpicNo)
{
  return s_aptXpicDebug[uiXpicNo]->ulXpic_break_return_fiq_pc;
}

/*****************************************************************************/
/*! Write xPIC DRAM DWord wise
 * \description
 *   Write xPIC DRAM DW-wise.
 * \class
 *   XPIC
 * \params
 *   uOffset              [in]  DRAM Offset
 *   ulValue              [in]  Value                                         */
/*****************************************************************************/
void DRV_XPIC_WrDramDw(unsigned int uiXpicNo, unsigned int uiOffset, uint32_t ulValue)
{
  s_apulXpicDram[uiXpicNo][uiOffset] = ulValue;
}

/*****************************************************************************/
/*! Read xPIC DRAM DWord wise
 * \description
 *   Read xPIC DRAM DW-wise.
 * \class
 *   XPIC
 * \params
 *   uOffset              [in]  DRAM Offset
 * \return
 *   Read value                                                               */
/*****************************************************************************/
uint32_t DRV_XPIC_RdDramDw(unsigned int uiXpicNo, unsigned int uiOffset)
{
  return s_apulXpicDram[uiXpicNo][uiOffset];
}

/*****************************************************************************/
/*! Write xPIC PRAM DWord wise
 * \description
 *   Write xPIC PRAM DW-wise.
 * \class
 *   XPIC
 * \params
 *   uOffset              [in]  PRAM Offset
 *   ulValue              [in]  Value                                         */
/*****************************************************************************/
void DRV_XPIC_WrPramDw(unsigned int uiXpicNo, unsigned int uiOffset, uint32_t ulValue)
{
  s_apulXpicPram[uiXpicNo][uiOffset] = ulValue;
}

/*****************************************************************************/
/*! Read xPIC PRAM DWord wise
 * \description
 *   Read xPIC PRAM DW-wise.
 * \class
 *   XPIC
 * \params
 *   uOffset              [in]  PRAM Offset
 * \return
 *   Read value                                                               */
/*****************************************************************************/
uint32_t DRV_XPIC_RdPramDw(unsigned int uiXpicNo, unsigned int uiOffset)
{
  return s_apulXpicPram[uiXpicNo][uiOffset];
}

/*! \} *//* End of group XPIC */

#endif /* DRV_XPIC_MODULE_DISABLED */

