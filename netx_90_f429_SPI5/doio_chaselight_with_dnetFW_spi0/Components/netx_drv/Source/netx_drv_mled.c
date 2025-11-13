/**************************************************************************//**
 * \file     netx_drv_mled.c
 * \brief    MLED module driver.
 * $Revision: 10355 $
 * $Date: 2023-02-02 13:44:10 +0200 (Thu, 02 Feb 2023) $
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
#ifdef DRV_MLED_MODULE_ENABLED

/*lint -save -e685 */
/*lint -save -e568 */

/*! \addtogroup MLED MLED
 * \{
 * \brief The MLED driver, defined by DRV_MLED_HANDLE_T
 *
 * \details
 */

/*!
 * \brief Table of the device addresses.
 *
 * Used to identify the device addresses by the device id.
 */
static DRV_MLED_DEVICE_T * const s_apChannelDeviceAddressTable[DRV_MLED_DEVICE_COUNT] = DRV_MLED_DEVICE_LIST;

/*!
 * Define for checking the consistency of the handle or static representation of the driver.
 */
#ifndef DRV_HANDLE_CHECK_INACTIVE
#define DRV_HANDLE_CHECK(handle);\
  if((handle)==0){ \
    return DRV_ERROR_PARAM; \
  }\
  if((handle)->ptDevice==0){ \
    return DRV_ERROR_PARAM; \
  }
#else
#define DRV_HANDLE_CHECK(handle) ;
#endif

/*!
 * \brief This function initializes the mled context object.
 *
 * In case of an error during initialization, the function returns an error value, but the lock for the handle, will not be released.
 * Together with the return value, this will ensure that the driver can only be interacted with if it has been correctly initialized.
 * Since the lock is initialized when this function is called, this function can still be called again with other parameters after a failed initialization.
 *
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_Init(DRV_MLED_HANDLE_T * const ptDriver)
{
  unsigned int i;
  if(ptDriver == 0)
  {
    return DRV_ERROR_PARAM;
  }
  ptDriver->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptDriver);
  if(ptDriver->tConfiguration.eBlinkBorder == DRV_MLED_BLINK_BORDER_RESET)
  {
    ptDriver->tConfiguration.eBlinkBorder = DRV_MLED_BLINK_BORDER_DEFAULT;
  }
  else
  {
    if(ptDriver->tConfiguration.eBlinkBorder < DRV_MLED_BLINK_BORDER_MIN || ptDriver->tConfiguration.eBlinkBorder > DRV_MLED_BLINK_BORDER_MAX)
    {
      return DRV_ERROR_PARAM;
    }
  }
  if(ptDriver->tConfiguration.ePrescaleBorder == DRV_MLED_PRESCALE_BORDER_RESET)
  {
    ptDriver->tConfiguration.ePrescaleBorder = DRV_MLED_PRESCALE_BORDER_DEFAULT;
  }
  else
  {
    if(ptDriver->tConfiguration.ePrescaleBorder < DRV_MLED_PRESCALE_BORDER_MIN
      || ptDriver->tConfiguration.ePrescaleBorder > DRV_MLED_PRESCALE_BORDER_MAX)
    {
      return DRV_ERROR_PARAM;
    }
  }
  if(ptDriver->tConfiguration.eBrightnessDefault != DRV_MLED_BRIGHTNESS_RESET
    && (ptDriver->tConfiguration.eBrightnessDefault < DRV_MLED_BRIGHTNESS_MIN && ptDriver->tConfiguration.eBrightnessDefault > DRV_MLED_BRIGHTNESS_MAX))
  {
    return DRV_ERROR_PARAM;
  }
  ptDriver->ptDevice = s_apChannelDeviceAddressTable[0];
  ptDriver->ptDevice->mled_ctrl_app_cfg_b.enable = 1u;
  if(ptDriver->tConfiguration.eBrightnessDefault != DRV_MLED_BRIGHTNESS_RESET)
  {
    for(i = 0; i <= DRV_MLED_ID_MAX; i++)
    {
      ptDriver->ptDevice->mled_ctrl_app_output_on_time[i] = ptDriver->tConfiguration.eBrightnessDefault - 1u;
    }
  }
  ptDriver->ptDevice->mled_ctrl_app_cfg_b.prescale_counter_max = ptDriver->tConfiguration.ePrescaleBorder - 1u;
  ptDriver->ptDevice->mled_ctrl_app_cfg_b.blink_counter_max = ptDriver->tConfiguration.eBlinkBorder - 1u;
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * It trys to get a lock on the handle so that it is able to be reset.
 * Then it will reset the nvic, the device states and the context given.
 *
 * \memberof DRV_MLED_HANDLE_T
 * \param[in,out] ptDriver The handle of the driver
 * \return DRV_OK Everything is ok.
 *         DRV_LOCKED The api is locked and not accessible.
 *         DRV_ERROR_PARAM A given parameter is not correct.
 */
DRV_STATUS_E DRV_MLED_DeInit(DRV_MLED_HANDLE_T * const ptDriver)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  ptDriver->ptDevice->mled_ctrl_app_cfg_b.enable = 0u;
  *ptDriver = (DRV_MLED_HANDLE_T ) { 0 };
  return DRV_OK;
}

/*!
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelSetMode(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID, DRV_MLED_MODE_E eMode)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(uMledID > DRV_MLED_ID_MAX || uMledID < DRV_MLED_ID_MIN)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  if(eMode > DRV_MLED_MODE_MAX)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  ptDriver->ptDevice->mled_ctrl_app_output_sel[uMledID] = eMode << 1;
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelSetBrightness(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID, DRV_MLED_BRIGHTNESS_E uBrightness)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(uMledID > DRV_MLED_ID_MAX || uMledID < DRV_MLED_ID_MIN)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  if(uBrightness > DRV_MLED_BRIGHTNESS_MAX || uBrightness <= DRV_MLED_BRIGHTNESS_MIN)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  ptDriver->ptDevice->mled_ctrl_app_output_on_time[uMledID] = uBrightness - 1u;
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelInvert(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(uMledID > DRV_MLED_ID_MAX || uMledID < DRV_MLED_ID_MIN)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  unsigned int ulRegister;
  do
  {
    ulRegister = __LDREXW(&(ptDriver->ptDevice->mled_ctrl_app_output_sel[uMledID]));
    ulRegister ^= 1u;
  } while(0 != __STREXW(ulRegister, &(ptDriver->ptDevice->mled_ctrl_app_output_sel[uMledID])));
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelOutSet(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(uMledID > DRV_MLED_ID_MAX || uMledID < DRV_MLED_ID_MIN)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  ptDriver->ptDevice->mled_ctrl_app_output_sel[uMledID] = 1u;
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelOutReset(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(uMledID > DRV_MLED_ID_MAX || uMledID < DRV_MLED_ID_MIN)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  ptDriver->ptDevice->mled_ctrl_app_output_sel[uMledID] = 0u;
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_LineMaskedSet(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_MSK_T uMask, DRV_MLED_MSK_T uMledMSK)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  unsigned int ulRegister;
  uMledMSK &= ~uMask;
  do
  {
    ulRegister = __LDREXW(&(ptDriver->ptDevice->mled_ctrl_app_line0));
    ulRegister &= ~uMask;
    ulRegister |= uMledMSK;
  } while(0 != __STREXW(ulRegister, &(ptDriver->ptDevice->mled_ctrl_app_line0)));
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_LineOutSet(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_MSK_T uMledMSK)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  unsigned int ulRegister;
  do
  {
    ulRegister = __LDREXW(&(ptDriver->ptDevice->mled_ctrl_app_line0));
    ulRegister |= uMledMSK;
  } while(0 != __STREXW(ulRegister, &(ptDriver->ptDevice->mled_ctrl_app_line0)));
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_LineOutReset(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_MSK_T uMledMSK)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  unsigned int ulRegister;
  do
  {
    ulRegister = __LDREXW(&(ptDriver->ptDevice->mled_ctrl_app_line0));
    ulRegister &= ~uMledMSK;
  } while(0 != __STREXW(ulRegister, &(ptDriver->ptDevice->mled_ctrl_app_line0)));
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*! \} *//* End of group MLED */

/*lint -restore */
/*lint -restore */

#endif /* DRV_MLED_MODULE_ENABLED */
