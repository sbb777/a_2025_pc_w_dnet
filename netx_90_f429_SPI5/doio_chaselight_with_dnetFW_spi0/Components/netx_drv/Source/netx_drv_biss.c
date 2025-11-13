/**************************************************************************//**
 * \file     netx_drv_biss.c
 * \brief    BiSS controller module driver.
 * $Revision: 6592 $
 * $Date: 2019-12-17 15:14:57 +0200 (Tue, 17 Dec 2019) $
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
#ifdef DRV_BISS_MODULE_ENABLED

/*! \defgroup BISS BISS
 * \{
 * \brief This is the BISS peripheral module driver.
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
#include "netx_drv_biss.h"
#include <string.h>

/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/
#ifndef DRV_BISSM_INST
#define DRV_BISSM_INST 0
#endif

#define DRV_BISSM_CTRL_CHN_CNT  8
#define DRV_BISSM_CTRL_REG_CNT 64

#define DRV_BISSM_INSTR_CDM1          0x1UL
#define DRV_BISSM_INSTR_CDM0          0x2UL
#define DRV_BISSM_INSTR_REGCOM        0x4UL
#define DRV_BISSM_INSTR_REGCOM_REDUCE 0x7UL

#define DRV_BISSM_CC_SL_CFG_BISS_B 0x0
#define DRV_BISSM_CC_SL_CFG_BISS_C 0x1
#define DRV_BISSM_CC_SL_CFG_SSI    0x2

/*****************************************************************************/
/* Variables                                                                 */
/*****************************************************************************/

HW_PTR_BISS(s_aptBiss)
HW_PTR_BISS_CTRL(s_aptBissCtrl)

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/


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

#ifndef HW_WR32
#define HW_WR32(var,val) (var) = (val)
#endif

#ifndef HW_RD32
#define HW_RD32(var)      (var)
#endif

#ifndef HW_WR16
#define HW_WR16(var,val) (var) = (val)
#endif

#ifndef HW_RD16
#define HW_RD16(var)      (var)
#endif

#ifndef HW_WR8
#define HW_WR8(var,val)  (var) = (val)
#endif

#ifndef HW_RD8
#define HW_RD8(var)       (var)
#endif

/*****************************************************************************/
/*! Initialize HAL
* \description
*   Initializes the HAL according to the setting given in ptConfig. This function
*   must be called before any other.
* \class
*   Control Service Class
* \params
*   ptConfig  [in]  Global HAL configuration
*   pvUser    [in]  User defined parameter passed into system calls
* \return
*   DRV_BISSM_OK                                                                  */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_Init( DRV_BISSM_CFG_T* ptConfig,
                           void*        pvUser )
{
  unsigned int uiIdx;
  uintptr_t pulSCD;
  DRV_BISSM_RESULT_E ret;
  if(DRV_BISSM_OK != (ret = DRV_BissM_Stop()))
  {
    return ret;
  }

  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0, 1UL << HW_SRT(biss_ccc1_mc0_CHSEL)
                                                | HW_MSK(biss_ccc1_mc0_REGVERS)
                                                );


  pulSCD = (uintptr_t)s_aptBiss[DRV_BISSM_INST]->aulBiss_scdata0_;
  for(uiIdx = 0; uiIdx < DRV_BISSM_CTRL_CHN_CNT; ++uiIdx)
  {
    HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uiIdx], HW_MSK(biss_sc0_LSTOP0));
    HW_WR32(((uint32_t*)pulSCD)[0], 0);
    pulSCD+=4;
    HW_WR32(((uint32_t*)pulSCD)[0], 0);
    pulSCD+=4;
  }

  for(uiIdx = 0; uiIdx < (DRV_BISSM_CTRL_REG_CNT/sizeof(uint32_t)); ++uiIdx)
  {
    HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_rdata[uiIdx], 0);
  }

  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_cc_sl, 1 | (DRV_BISSM_CC_SL_CFG_BISS_C << HW_SRT(biss_cc_sl_CFGCH1)));

  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc0, 0);

  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ir, HW_MSK(biss_ir_BREAK));

  DRV_BissM_ConfirmAllIrq();

  return DRV_BISSM_OK;
}

/*****************************************************************************/
/*! Start BiSS Master Communication
* \description
*   This function starts the BiSS master logic to send frames automatically based
*   on selected external trigger.
* \class
*   Control Service Class
* \params
*   eTriggerCfg   [in] Trigger which starts the BiSS master cycle
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_StartTrigger( DRV_BISSM_TRIGGER_TYPE_E eTriggerCfg )
{
  uint32_t ulVal;
  DRV_BISSM_RESULT_E ret;
  if(DRV_BISSM_OK != (ret = DRV_BissM_Stop()))
  {
    return ret;
  }
  s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_trigger_cfg = (uint32_t)eTriggerCfg;
  ulVal = s_aptBiss[DRV_BISSM_INST]->ulBiss_mc1;
  ulVal &= ~(HW_MSK(biss_mc1_FREQAGS));
  ulVal |= 0x7F << HW_SRT(biss_mc1_FREQAGS);
  s_aptBiss[DRV_BISSM_INST]->ulBiss_mc1 = ulVal;
  s_aptBiss[DRV_BISSM_INST]->ulBiss_ir |= HW_MSK(biss_ir_AGS);
  return DRV_BISSM_OK;
}

/*****************************************************************************/
/*! Start BiSS Master Communication
* \description
*   This function starts the BiSS interface logic to send frames automatically directly
*   after having completed the previous cycle.
* \class
*   Control Service Class
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_StartAgsMin( void )
{
  uint32_t ulVal;
  DRV_BISSM_RESULT_E ret;
  if(DRV_BISSM_OK != (ret = DRV_BissM_Stop()))
  {
    return ret;
  }
  s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_trigger_cfg = 0;
  ulVal = s_aptBiss[DRV_BISSM_INST]->ulBiss_mc1;
  ulVal &= ~(HW_MSK(biss_mc1_FREQAGS));
  ulVal |= 0x7C << HW_SRT(biss_mc1_FREQAGS);
  s_aptBiss[DRV_BISSM_INST]->ulBiss_mc1 = ulVal;
  s_aptBiss[DRV_BISSM_INST]->ulBiss_ir |= HW_MSK(biss_ir_AGS);
  return DRV_BISSM_OK;
}

/*****************************************************************************/
/*! Start BiSS Master Communication
* \description
*   This function starts the BiSS master logic to send frames automatically based
*   on a given cycle time.
* \class
*   Control Service Class
* \params
*   ulCycleTimeUs   [in] cycle time at which the BiSS frame cycle is sent
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_StartCycle( uint32_t ulCycleTimeUs )
{
  uint32_t ulVal;
  DRV_BISSM_RESULT_E ret;
  if(ulCycleTimeUs > 4000) /* 4ms is max cycle time */
  {
    return DRV_BISSM_ERR_INVAL_PARAMETER;
  }
  else if(ulCycleTimeUs > 123)
  {
    ulCycleTimeUs *= 20;
    ulCycleTimeUs += 624;
    ulCycleTimeUs /= 625;
    if(ulCycleTimeUs > 0x80)
    {
      return DRV_BISSM_ERR_INVAL_PARAMETER;
    }
    ulCycleTimeUs += 0x7F;
  }
  if(DRV_BISSM_OK != (ret = DRV_BissM_Stop()))
  {
    return ret;
  }
  s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_trigger_cfg = 0;
  ulVal = s_aptBiss[DRV_BISSM_INST]->ulBiss_mc1;
  ulVal &= ~(HW_MSK(biss_mc1_FREQAGS));
  ulVal |= ulCycleTimeUs << HW_SRT(biss_mc1_FREQAGS);
  s_aptBiss[DRV_BISSM_INST]->ulBiss_mc1 = ulVal;
  s_aptBiss[DRV_BISSM_INST]->ulBiss_ir |= HW_MSK(biss_ir_AGS);
  return DRV_BISSM_OK;
}

/*****************************************************************************/
/*! Stop BiSS Master Communication
* \description
*   This function stops the BiSS interface logic.
* \class
*   Control Service Class
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_Stop( void )
{
  s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_trigger_cfg = 0;
  s_aptBiss[DRV_BISSM_INST]->ulBiss_ir &= ~HW_MSK(biss_ir_AGS);
  while(!(s_aptBiss[DRV_BISSM_INST]->ulBiss_status0 & HW_MSK(biss_status0_EOT)))
  {
    /* wait for stop */
  }

  return DRV_BISSM_OK;
}

/*****************************************************************************/
/*! Start BiSS Master Control Communication
* \description
*   This function triggers a selected control communication.
*   The master cycle has to be started before.
* \class
*   Control Service Class
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_CtrlStart( void )
{
  uint32_t ulVal;
  if( 0 == (HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_status0) & HW_MSK(biss_status0_REGEND)))
  {
    return DRV_BISSM_ERR_INVAL_STATE;
  }

  ulVal = DRV_BISSM_INSTR_REGCOM << HW_SRT(biss_ir_INSTR);
  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ir, ulVal | HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ir));

  return DRV_BISSM_OK;
}

/*****************************************************************************/
/*! Set Parameter
* \description
*   This function sets a global parameter.
* \class
*   Control Service Class
* \params
*   ePrmID     [in] Parameter ID
*   ulPrmVal   [in] Parameter value
* \return
*   DRV_BISSM_OKAY
*   DRV_BISSM_ERR_INVAL_PARAMETER                                                  */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_SetParam( DRV_BISSM_PARAM_E ePrmID,
                               uint32_t      ulPrmVal )
{
  uint32_t ulVal;

  switch(ePrmID)
  {
    case DRV_BISSM_PARAM_CFG_IF:
    break;

    case DRV_BISSM_PARAM_PERIOD:
      ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0) & ~HW_MSK(biss_ccc1_mc0_FREQS);
      ulPrmVal = (ulPrmVal + 9) / 10 - 1;
      if(ulPrmVal > 0xF)
      {
        return DRV_BISSM_ERR_INVAL_PARAMETER;
      }
      ulVal |= (ulPrmVal << HW_SRT(biss_ccc1_mc0_FREQS));
      HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0, ulVal);
    break;

    case DRV_BISSM_PARAM_CYCLE_TIME:
    break;

    case DRV_BISSM_PARAM_BUSY_TIME:
    break;

    case DRV_BISSM_PARAM_SSI_TSAR:
    break;

    case DRV_BISSM_PARAM_IRQ_MASK:
      HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_msk_reset, ~ulPrmVal);
      HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_msk_set,    ulPrmVal);
    break;

    case DRV_BISSM_PARAM_CTRL_TIMEOUT:
    break;

    case DRV_BISSM_PARAM_TRGCFG:
      if(ulPrmVal != 0)
      {
        ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_mc1) & ~HW_MSK(biss_mc1_FREQAGS);
        ulVal |= 0x7F << HW_SRT(biss_mc1_FREQAGS);
        HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_mc1, ulVal);
      }
      HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_trigger_cfg, ulPrmVal);
    break;

    case DRV_BISSM_PARAM_TRGMAN:
      HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_trigger, ulPrmVal);
    break;

    default:
      return DRV_BISSM_ERR_INVAL_PARAMETER;
  }

  return DRV_BISSM_OK;
}

#if 0
static uint8_t DRV_BissM_CalcCrcMagic(uint32_t ulCrcPoly)
{
  uint8_t      bMask;
  uint8_t      bCrc = 0;
  unsigned int uCrcLen;
  unsigned int uIdx;

  ulCrcPoly >>= 1;

  if (ulCrcPoly == 0)
  {
    /* CRC is not used */
    return 0;
  }

  bMask = 0x80;
  uCrcLen = 8;

  /*-- setting the CRC mask and computing CRC length --*/
  while (bMask > ulCrcPoly) {
    bMask = (bMask >> 1);
    uCrcLen--;
  }

  ulCrcPoly = (ulCrcPoly << 1) + 1;

  uIdx = uCrcLen;
  while( uIdx )
  {
    uIdx--;

    if( (bCrc & bMask) != 0 )
    {
      bCrc = (uint8_t)(bCrc << 1);
    }
    else
    {
      bCrc = (uint8_t)((bCrc << 1) ^ ulCrcPoly);
    }
  }

  bMask = (1 << uCrcLen) - 1;
  bCrc &= bMask;

  return bCrc;
}
#endif

/*****************************************************************************/
/*! Set Data Channel Parameter
* \description
*   This function sets a data channel specific parameter.
* \class
*   Control Service Class
* \params
*   uChNo      [in] Data channel number
*   ePrmID     [in] Parameter ID
*   ulPrmVal   [in] Parameter value
* \return
*   DRV_BISSM_OKAY
*   DRV_BISSM_ERR_INVAL_PARAMETER                                                 */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_SetChannelParam( unsigned int          uChNo,
                                      DRV_BISSM_CHANNEL_PARAM_E ePrmID,
                                      uint32_t              ulPrmVal )
{
  uint32_t ulVal;
  switch(ePrmID)
  {
    case DRV_BISSM_CHANNEL_PARAM_DATA_ACTIVE:
      ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uChNo]);
      if( ulPrmVal == DRV_BISSM_DATA_ACTIVE_ACTIVE )
      {
        ulVal |= HW_MSK(biss_sc0_ENSCD0);
      }
      else
      {
        ulVal &= ~HW_MSK(biss_sc0_ENSCD0);
      }
      HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uChNo], ulVal);
    break;

    case DRV_BISSM_CHANNEL_PARAM_DATA_TYPE:
      if( ulPrmVal == DRV_BISSM_DATA_TYPE_ACT )
      {
        /* enable MO */
        ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0);
        ulVal |= HW_MSK(biss_ccc1_mc0_EN_MO);
        HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0, ulVal);

        /* mark slave as actuator */
        ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_cc_sl);
        ulVal |= 1UL << (uChNo+ HW_SRT(biss_cc_sl_ACTnSENS));
        HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_cc_sl, ulVal);
      }
      else
      {
        /* mark slave as sensor */
        ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_cc_sl);
        ulVal &= ~(1UL << (uChNo+ HW_SRT(biss_cc_sl_ACTnSENS)));
        HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_cc_sl, ulVal);

        /* any actuators left? */
        if( 0 == (ulVal & HW_MSK(biss_cc_sl_ACTnSENS)) )
        {
          /* disable MO */
          ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0);
          ulVal &= ~HW_MSK(biss_ccc1_mc0_EN_MO);
          HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0, ulVal);
        }
      }

    break;

    case DRV_BISSM_CHANNEL_PARAM_DATA_LEN:
      ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uChNo]) & ~HW_MSK(biss_sc0_SCDLEN0);
      ulVal |= (ulPrmVal - 1) << HW_SRT(biss_sc0_SCDLEN0);
      HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uChNo], ulVal);
    break;

    case DRV_BISSM_CHANNEL_PARAM_DATA_ALIGN:
      /* only data right aligned is supported. Configuration of left aligned data is not supported yet */
    break;

    case DRV_BISSM_CHANNEL_PARAM_CRC_POLY:
      ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uChNo]) & ~(HW_MSK(biss_sc0_SCRCPOLY0) | HW_MSK(biss_sc0_SELCRCS0));

      switch(ulPrmVal)
      {
      case 0x00000: break;
      case 0x0000b: ulVal |= (0x03 << HW_SRT(biss_sc0_SCRCPOLY0)); break;
      case 0x00013: ulVal |= (0x04 << HW_SRT(biss_sc0_SCRCPOLY0)); break;
      case 0x00025: ulVal |= (0x05 << HW_SRT(biss_sc0_SCRCPOLY0)); break;
      case 0x00043: ulVal |= (0x06 << HW_SRT(biss_sc0_SCRCPOLY0)); break;
      case 0x00089: ulVal |= (0x07 << HW_SRT(biss_sc0_SCRCPOLY0)); break;
      case 0x0012f: ulVal |= (0x08 << HW_SRT(biss_sc0_SCRCPOLY0)); break;
      case 0x190d9: ulVal |= (0x10 << HW_SRT(biss_sc0_SCRCPOLY0)); break;
      default:
        if(0 != (ulPrmVal & 0xffffff00UL))
        {
          return DRV_BISSM_ERR_INVAL_PARAMETER;
        }
        ulVal |= HW_MSK(biss_sc0_SELCRCS0);
        ulVal |= (ulPrmVal >> 1) << HW_SRT(biss_sc0_SCRCPOLY0);
        break;
      }

      HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uChNo], ulVal);
    break;

    case DRV_BISSM_CHANNEL_PARAM_CRC_START:
      ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uChNo]) & ~HW_MSK(biss_sc0_SCRCSTART0);
      ulVal |= ulPrmVal << HW_SRT(biss_sc0_SCRCSTART0);
      HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_sc[uChNo], ulVal);
    break;

    case DRV_BISSM_CHANNEL_PARAM_EXT_DATA_LEN:
    break;

    case DRV_BISSM_CHANNEL_PARAM_EXT_CRC_POLY:
    break;

    case DRV_BISSM_CHANNEL_PARAM_EXT_CRC_START:
    break;

    default:
      return DRV_BISSM_ERR_INVAL_PARAMETER;
  }
  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Get BiSS Master Global State
* \description
*   This function retrieves the global state of the BiSS Master.
*   If no IRQs are used this function must be used to poll the BiSS Master.
*   The functions DRV_BissM_Start(), DRV_BissM_SetParam() and DRV_BissM_SetChannelParam()
*   must not be called as long as the ptState.fBusy flag is set.
* \class
*   Status Service Class
* \params
*   ptState  [out] The global status of the BiSS master
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_GetState( DRV_BISSM_STATE_T* ptState )
{
  uint32_t ulStatus0 = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_status0);
  ptState->fBusy = ( 0 == (ulStatus0 & HW_MSK(biss_status0_EOT)));

  ptState->fSlaveRdy = ( 0 != (HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_status1) & HW_MSK(biss_status1_SL1)) );

  ptState->eError = (0 == (ulStatus0 & HW_MSK(biss_status0_nSCDERR)) ) ? DRV_BISSM_FRM_ERR_SLAVE_NRDY : DRV_BISSM_FRM_OK;

  ptState->usCycleCnt = 0;

  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Set BiSS Frame Process Data
* \description
*   This function sets the process data of the given data channel.
* \class
*   Data Service Class
* \params
*   uChNo    [in]  Data channel number
*   pbData   [in]  Data
* \return
*   DRV_BISSM_OKAY                                                                 */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_SetProcessData( unsigned int  uChNo,
                                     const uint8_t *pbData )
{
  uint32_t ulData;
  volatile uint32_t* pulSCD;

  pulSCD = s_aptBiss[DRV_BISSM_INST]->aulBiss_scdata0_ + 2 * uChNo;

  ulData = pbData[0]
           + ((uint32_t)pbData[1] << 8)
           + ((uint32_t)pbData[2] << 16)
           + ((uint32_t)pbData[3] << 24);
  HW_WR32(pulSCD[0], ulData);
  ulData = pbData[4]
           + ((uint32_t)pbData[5] << 8)
           + ((uint32_t)pbData[6] << 16)
           + ((uint32_t)pbData[7] << 24);
  HW_WR32(pulSCD[1], ulData);


  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Get BiSS Frame Process Data
* \description
*   This function gets the latest process data of the given data channel.
* \class
*   Data Service Class
* \params
*   uChNo       [in]   Data channel number
*   pbStatus    [out]  Status value
*   pusCycleCnt [out]  Cycle counter of the according BiSS frame (FrameID)
*   pbData      [out]  Process data
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_GetProcessData( unsigned int       uChNo,
                                     DRV_BISSM_DATA_STATE_E *peStatus,
                                     uint16_t           *pusCycleCnt,
                                     uint8_t            *pbData )
{
  unsigned int    uIdx;
  uint32_t        ulState;
  volatile uint32_t* pulSCD;

  union {
    uint32_t aul[2];
    uint8_t  ab[8];
  } data;

  pulSCD = s_aptBiss[DRV_BISSM_INST]->aulBiss_scdata0_ + 2 * uChNo;

  *peStatus = DRV_BISSM_DATA_OK;

  /* 2 bytes cycle counter */
  if( pusCycleCnt != NULL )
  {
    *pusCycleCnt = 0;
  }

  data.aul[0] = HW_RD32(pulSCD[0]);
  data.aul[1] = HW_RD32(pulSCD[1]);

  for( uIdx = 0; uIdx < 8; ++uIdx)
  {
    pbData[uIdx] = data.ab[uIdx];
  }

  /* status */
  ulState = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_status0);

  if( 0 == (ulState & HW_MSK(biss_status0_nSCDERR)) )
  {
    *peStatus = DRV_BISSM_DATA_ERR_FRM;
    return DRV_BISSM_OK;
  }

  if( 0 == (ulState & (HW_MSK(biss_status0_SVALID0) << (2 * uChNo))) )
  {
    *peStatus = DRV_BISSM_DATA_ERR_CRC;
    return DRV_BISSM_OK;
  }

  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Get SSI Extended Frame Process Data
* \description
*   This function gets the latest SSI Extended data of the given data channel.
*   This is the data after the first CRC of the SSI frame. The length of this
*   data field is configured by the channel parameter
*   DRV_BISSM_CHANNEL_PARAM_EXT_DATA_LEN.
*   Note: The SSI Extended data is only available if the parameter
*   DRV_BISSM_CHANNEL_PARAM_EXT_DATA_LEN is greater than one.
* \class
*   Data Service Class
* \params
*   uChNo       [in]   Data channel number
*   pbStatus    [out]  Status value
*   pusCycleCnt [out]  Cycle counter of the according BiSS frame (FrameID)
*   pbData      [out]  Process data
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_GetSsiExtData( unsigned int       uChNo,
                                    DRV_BISSM_DATA_STATE_E *peStatus,
                                    uint16_t           *pusCycleCnt,
                                    uint8_t            *pbData )
{
#if 0
  uint8_t*        pbFrameData;
  DRV_BISSM_FRM_ERR_E bBissState;

#ifndef __DRV_BISSM_DISABLE_CHECKS__
  if( uChNo > DRV_BISSM_CHANNEL_CNT )
    return DRV_BISSM_ERR_INVAL_PARAMETER;
#endif


  /* 1 byte data */
  pbFrameData = (uint8_t*)&s_ptXpicShm->atXPIC_SHM_FRAME_DATA[uChNo];

  pbData[0] = NX_READ8(pbFrameData[DRV_BISSM_OFS_FRAME_DATA_EXT]);

  /* 1 byte status */
  bBissState = pbFrameData[DRV_BISSM_OFS_FRAME_DATA_STATE];

  /* 2 bytes cycle counter */
  if( pusCycleCnt != NULL )
  {
    *pusCycleCnt = NX_READ8(pbFrameData[DRV_BISSM_OFS_FRAME_DATA_CYCLE])
                | (NX_READ8(pbFrameData[DRV_BISSM_OFS_FRAME_DATA_CYCLE + 1]) << 8);
  }

  if( bBissState != 0 )
  {
    *peStatus = DRV_BISSM_DATA_ERR_FRM;
    return DRV_BISSM_OK;
  }

  /* CRC OK? */
  if( s_abExtCrcMagic[uChNo] != NX_READ8(pbFrameData[DRV_BISSM_OFS_FRAME_DATA_EXT_CRC]) )
  {
    *peStatus = DRV_BISSM_DATA_ERR_CRC;
  }
  else
  {
    *peStatus = DRV_BISSM_DATA_OK;
  }
#endif

  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Start Control Frame with Command Access
* \description
*   This function starts a control frame with command access.
*   To check if the control frame finished you can either\n
*    - poll the control state via the function DRV_BissM_GetCtrlState() or\n
*    - setup a handler for the DRV_BISSM_IRQ_CTRL event.\n
* \class
*   Control Service Class
* \params
*   uSlaveId    [in]  Bit mask of slave IDs
*   uOpcode     [in]  opcode
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_StartCtrlCmd( unsigned int  uSlaveId,
                                   unsigned int  uOpcode )
{
  uint32_t ulStatus0 = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_status0);
  uint32_t ulVal;

  if( 0 == (ulStatus0 & HW_MSK(biss_status0_CDMTIMEOUT)))
  {
    return DRV_BISSM_ERR_INVAL_STATE;
  }

  ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0);

  ulVal &= ~(HW_MSK(biss_ccc1_mc0_CTS)
            |HW_MSK(biss_ccc1_mc0_CMD)
            |HW_MSK(biss_ccc1_mc0_IDA_TEST)
            );

  ulVal |= uOpcode << HW_SRT(biss_ccc1_mc0_CMD);
  ulVal |= HW_MSK(biss_ccc1_mc0_REGVERS);

  if( uSlaveId != 0 )
  {
    /* broadcast, set EX bit immediately */
    ulVal |= HW_MSK(biss_ccc1_mc0_IDA_TEST);
  }

  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0, ulVal);
  HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_rdata[0], uSlaveId);

  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Start Control Frame with Register Access
* \description
*   This function configures a control frame with register access.
* \class
*   Control Service Class
* \params
*   uSlaveId    [in]  Slave id
*   uRegAdr     [in]  Address inside the slave
*   uRegNr      [in]  Number for auto increment access
*   uWnR        [in]  0=read register, 1=write register
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_StartCtrlReg( unsigned int  uSlaveId,
                                   unsigned int  uRegAdr,
                                   unsigned int  uRegNr,
                                   unsigned int  uWnR )
{
  uint32_t ulStatus0 = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_status0);
  uint32_t ulVal;

  if( 0 == (ulStatus0 & HW_MSK(biss_status0_CDMTIMEOUT)))
  {
    return DRV_BISSM_ERR_INVAL_STATE;
  }

  if(uSlaveId > 7)
  {
    return DRV_BISSM_ERR_INVAL_PARAMETER;
  }

  ulVal = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0);
  ulVal &= ~(HW_MSK(biss_ccc1_mc0_CMD)
            |HW_MSK(biss_ccc1_mc0_IDA_TEST));
  ulVal |= HW_MSK(biss_ccc1_mc0_CTS);
  ulVal |= HW_MSK(biss_ccc1_mc0_REGVERS);
  ulVal |= uSlaveId << HW_SRT(biss_ccc1_mc0_IDA_TEST);
  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc1_mc0, ulVal);

  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ccc0,(uRegAdr      << HW_SRT(biss_ccc0_REGADR))
                                            |((uRegNr - 1) << HW_SRT(biss_ccc0_REGNUM))
                                            |(uWnR         << HW_SRT(biss_ccc0_WNR)));

  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Stop Control Frame
* \description
*   This function aborts the current control frame.
* \class
*   Control Service Class
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_CtrlRst( void )
{
  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Set Control Frame Data
* \description
*   This function sets control frame data.
* \class
*   Control Service Class
* \params
*   uRegNr    [in]  Number for auto increment access
*   pbData    [in]  Data
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_SetCtrlData( unsigned int  uRegCnt,
                                  const uint8_t *pbData )
{
  unsigned int uiCnt;
  uint32_t ulData = 0;
#ifndef __DRV_BISSM_DISABLE_CHECKS__
  if( uRegCnt > DRV_BISSM_CTRL_REG_CNT )
    return DRV_BISSM_ERR_INVAL_PARAMETER;
  if( pbData == NULL )
    return DRV_BISSM_ERR_INVAL_PARAMETER;
#endif

  for(uiCnt = 0; uiCnt < uRegCnt; ++uiCnt)
  {
    unsigned int uiByte = uiCnt & 0x3;
    ulData = ulData | ((uint32_t)pbData[uiCnt] << (8 * uiByte));
    if( (uiCnt & 0x3) == 3)
    {
      /* write 4 bytes at once */
      HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_rdata[uiCnt >> 2], ulData);
      ulData = 0;
    }
  }

  /* write remaining bytes */
  if(0 != (uRegCnt & 0x3))
  {
    HW_WR32(s_aptBiss[DRV_BISSM_INST]->aulBiss_rdata[uRegCnt >> 2], ulData);
  }

  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Get Control Frame Data
* \description
*   This function gets control frame data.
* \class
*   Control Service Class
* \params
*   uRegNr    [in]   Number for auto increment access
*   pbData    [out]  Data
* \return
*   DRV_BISSM_OKAY                                                                */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_GetCtrlData( unsigned int uRegCnt,
                                  uint8_t      *pbData )
{
  uint32_t ulData = 0;
  unsigned int uiIdx;

#ifndef __DRV_BISSM_DISABLE_CHECKS__
  if( uRegCnt > DRV_BISSM_CTRL_REG_CNT )
    return DRV_BISSM_ERR_INVAL_PARAMETER;
  if( pbData == NULL )
    return DRV_BISSM_ERR_INVAL_PARAMETER;
#endif

  for(uiIdx = 0; uiIdx < uRegCnt; ++uiIdx)
  {
    if( (uiIdx & 0x3) == 0 )
    {
      ulData = HW_RD32(s_aptBiss[DRV_BISSM_INST]->aulBiss_rdata[uiIdx >> 2]);
    }

    pbData[uiIdx] = ulData & 0xff;
    ulData >>= 8;
  }


  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Get BiSS Master Control State
* \description
*   This function retrieves the status of the control state machine
*   of the BiSS Master.
*   The functions DRV_BissM_CtrlCmd(), DRV_BissM_CtrlReg(), DRV_BissM_SetCtrlData() and
*   DRV_BissM_GetCtrlData() must not be called as long as the
*   ptCtrlState->fBusy flag is set.
* \class
*   Control Service Class
* \params
*   ptCtrlState  [out] Control state
* \return
*   DRV_BISSM_OK                                                                  */
/*****************************************************************************/
DRV_BISSM_RESULT_E DRV_BissM_GetCtrlState( DRV_BISSM_CTRL_STATE_T* ptCtrlState )
{
  uint32_t ulStatus0 = HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_status0);

  ptCtrlState->fBusy = ( 0 == (ulStatus0 & HW_MSK(biss_status0_CDMTIMEOUT)) );
  ptCtrlState->eError = ( 0 == (ulStatus0 & HW_MSK(biss_status0_nREGERR)) ) ? DRV_BISSM_CTRL_ERR_BISS_FRAME_ERR : DRV_BISSM_CTRL_OK;

  return DRV_BISSM_OK;
}


/*****************************************************************************/
/*! Get and Confirm BiSS Master Interrupts
* \description
*   This function retrieves all currently pending interrupt events.
*   The events are also cleared within this function.
*   The return value is a bitwise OR combination of the bit masks defined
*   by DRV_BISSM_IRQ_E.
* \class
*   Control Service Class
* \params
*   ptCtrlState  [out] Control state
* \return
*   Event bit mask                                                           */
/*****************************************************************************/
uint32_t DRV_BissM_GetIrqRaw( void )
{
  uint32_t ulVal = HW_RD32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_raw);
  return ulVal;
}


/*****************************************************************************/
/*! Get and Confirm BiSS Master Interrupts
* \description
*   This function retrieves all currently pending interrupt events.
*   The events are also cleared within this function.
*   The return value is a bitwise OR combination of the bit masks defined
*   by DRV_BISSM_IRQ_E.
* \class
*   Control Service Class
* \params
*   ptCtrlState  [out] Control state
* \return
*   Event bit mask                                                           */
/*****************************************************************************/
void DRV_BissM_ConfirmIrq( uint32_t ulIrqMsk )
{
  HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_raw, ulIrqMsk);
}


/*****************************************************************************/
/*! Get and Confirm BiSS Master Interrupts
* \description
*   This function retrieves all currently pending interrupt events.
*   The events are also cleared within this function.
*   The return value is a bitwise OR combination of the bit masks defined
*   by DRV_BISSM_IRQ_E.
* \class
*   Control Service Class
* \params
*   ptCtrlState  [out] Control state
* \return
*   Event bit mask                                                           */
/*****************************************************************************/
uint32_t DRV_BissM_GetIrq( void )
{
  uint32_t ulVal = HW_RD32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_masked);
  return ulVal;
}


/*****************************************************************************/
/*! Get and Confirm BiSS Master Interrupts
* \description
*   This function retrieves all currently pending interrupt events.
*   The events are also cleared within this function.
*   The return value is a bitwise OR combination of the bit masks defined
*   by DRV_BISSM_IRQ_E.
* \class
*   Control Service Class
* \params
*   ptCtrlState  [out] Control state
* \return
*   Event bit mask                                                           */
/*****************************************************************************/
void DRV_BissM_IrqEnable( uint32_t ulIrqMsk )
{
  HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_msk_set, ulIrqMsk);
}


/*****************************************************************************/
/*! Get and Confirm BiSS Master Interrupts
* \description
*   This function retrieves all currently pending interrupt events.
*   The events are also cleared within this function.
*   The return value is a bitwise OR combination of the bit masks defined
*   by DRV_BISSM_IRQ_E.
* \class
*   Control Service Class
* \params
*   ptCtrlState  [out] Control state
* \return
*   Event bit mask                                                           */
/*****************************************************************************/
void DRV_BissM_IrqDisable( uint32_t ulIrqMsk )
{
  HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_msk_reset, ulIrqMsk);
}


/*****************************************************************************/
/*! Get and Confirm BiSS Master Interrupts
* \description
*   This function retrieves all currently pending interrupt events.
*   The events are also cleared within this function.
*   The return value is a bitwise OR combination of the bit masks defined
*   by DRV_BISSM_IRQ_E.
* \class
*   Control Service Class
* \params
*   ptCtrlState  [out] Control state
* \return
*   Event bit mask                                                           */
/*****************************************************************************/
uint32_t DRV_BissM_GetConfirmIrq( void )
{
  uint32_t ulVal = HW_RD32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_raw);
  HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_raw, ulVal);
  return ulVal;
}

/*****************************************************************************/
/*! Confirms BiSS Master Interrupts
* \description
*   This function cleares all currently pending interrupt events.
* \class
*   Control Service Class
* \params
*   ptCtrlState  [out] Control state                                                */
/*****************************************************************************/
void DRV_BissM_ConfirmAllIrq( void )
{
  uint32_t ulVal = HW_RD32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_raw);
  HW_WR32(s_aptBissCtrl[DRV_BISSM_INST]->ulBiss_ctrl_irq_raw, ulVal);
}

void DRV_BissM_Break( void )
{
  HW_WR32(s_aptBiss[DRV_BISSM_INST]->ulBiss_ir, HW_MSK(biss_ir_BREAK));
}

uint32_t DRV_BissM_Status0( void )
{
  return HW_RD32(s_aptBiss[DRV_BISSM_INST]->ulBiss_status0);
}

/*! \} *//* End of group BISS */

#endif /* DRV_BISS_MODULE_DISABLED */
