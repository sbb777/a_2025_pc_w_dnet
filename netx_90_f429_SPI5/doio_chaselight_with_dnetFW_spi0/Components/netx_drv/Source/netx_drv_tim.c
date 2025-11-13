/*!************************************************************************//*!
 * \file    netx_drv_tim.c
 * \brief   Timer peripheral module driver.
 * \details This file provides firmware functions to manage the following
 *           functionalities of timer peripheral:
 *            + Initialization and de-initialization functions
 *            + Timer operation functions
 * $Revision: 11390 $
 * $Date: 2024-06-27 17:45:36 +0300 (Thu, 27 Jun 2024) $
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

/* Includes ------------------------------------------------------------------*/
#include "netx_drv.h"
#ifdef DRV_TIM_MODULE_ENABLED
/*!
 * \addtogroup TIM TIM
 * \brief Timer peripheral module driver.
 * \{
 */
/*lint -save -e685 */
/*lint -save -e568 */

/*!
 * Those are the defines associated with the irq list enumeration.
 * This should probably be moved to the csp.
 */
enum
{
  DRV_TIM_IRQ_GPIOTIM0 = 0x00u,
  DRV_TIM_IRQ_GPIOTIM1 = 0x01u,
  DRV_TIM_IRQ_GPIOTIM2 = 0x02u,
  DRV_TIM_IRQ_TIMER0 = 0x03u,
  DRV_TIM_IRQ_TIMER1 = 0x04u,
  DRV_TIM_IRQ_TIMER2 = 0x05u,
  DRV_TIM_IRQ_SYSTIME_COMPARE = 0x06u,
  DRV_TIM_IRQ_SYSTICK = 0x07u
};

/*!
 * \brief Table of the IRQ vector numbers.
 *
 * Used to identify the interrupt channels by the device id.
 */
static IRQn_Type const aulIRQnTable[DRV_TIM_IRQ_COUNT] = DRV_TIM_IRQ_LIST;

/*!
 * \brief Used for mapping the handle to an interrupt.
 *
 * Threadsafe and reentrant because its is only written in normal context an used in interrupt context of the specific interrupt.
 */
static DRV_CALLBACK_F s_apTIMCallbackTable[DRV_TIM_IRQ_COUNT] = { 0 };

/*!
 * Interrupt attached handler context pointers.
 */
static void* s_apTIMCallbackHandleTable[DRV_TIM_IRQ_COUNT] = { 0 };

/*!
 * Interrupt driver handler context pointers.
 */
static DRV_TIM_HANDLE_T* s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_COUNT] = { 0 };

/*!
 * Interrupt counter.
 */
static volatile uint32_t s_apTIMIrqCntr[DRV_TIM_IRQ_COUNT] = { 0 };

#ifndef DRV_HANDLE_CHECK_INACTIVE
/*!
 * Define for checking the consistency of the handle or static representation of the driver.
 */
#define DRV_HANDLE_CHECK(handle)\
  if((handle) == 0){ \
    return DRV_ERROR_PARAM; \
  } \
  if((handle)->ptDevice.ptGpio == 0 || \
     (handle)->tConfiguration.eDeviceID < DRV_TIM_DEVICE_ID_GPIOCNTR_MIN || \
     (handle)->tConfiguration.eDeviceID > DRV_TIM_DEVICE_ID_SYSTIME_MAX){ \
    return DRV_ERROR_PARAM; \
  }
#else
/*!
 * If the checking is deactivated, the source line will be replaced with void.
 */
#define DRV_HANDLE_CHECK(handle)
#endif
/*!
 *
 * The device specified in the handle will be initialized and configured after the given attributes.
 * If the interrupt or dma method is chosen, the nvic and dmac is also configured.
 *
 * In case of an error during initialization, the function returns an error value, but the lock for the handle, will not be released.
 * Together with the return value, this will ensure that the driver can only be interacted with if it has been correctly initialized.
 * Since the lock is initialized when this function is called, this function can still be called again with other parameters after a failed initialization.
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_Init(DRV_TIM_HANDLE_T * const ptTim)
{
  /* Check the TIM handle allocation */
  if(ptTim == NULL)
  {
    return DRV_ERROR_PARAM;
  }
  ptTim->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  uint32_t id;
  if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
  {
    ret = DRV_NSUPP;
  }
  else if(ptTim->tConfiguration.eOperationMode > DRV_OPERATION_MODE_DMA)
  {
    ret = DRV_ERROR_PARAM;
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_GPIOCNTR_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
  {
    if(ptTim->tConfiguration.eSystimeReference == DRV_TIM_SYSTIME_REFERENCE_DEFAULT)
    {
      ptTim->tConfiguration.eSystimeReference = DRV_TIM_SYSTIME_REFERENCE_APP;
    }
    if(ptTim->tConfiguration.eSystimeReference != DRV_TIM_SYSTIME_REFERENCE_APP
      || (ptTim->tConfiguration.eDioMskOutputReference
        > DRV_DIO_LINE_MSK_GPIO))
    {
      return DRV_ERROR_PARAM;
    }
    if(ptTim->tConfiguration.eDioIdInputReference < DRV_DIO_ID_GPIO_MIN || ptTim->tConfiguration.eDioIdInputReference > DRV_DIO_ID_GPIO_MAX)
    {
      return DRV_ERROR_PARAM;
    }
    ptTim->ulSubID = (uint32_t) ptTim->tConfiguration.eDeviceID - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN;
    s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID] = 0;
    ptTim->ptDevice.ptGpio = DRV_GPIO_DEVICE;
    ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].event_act = ptTim->tConfiguration.eExternalEventMode;
    ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].gpio_ref = ptTim->tConfiguration.eDioIdInputReference;
    if(ptTim->tConfiguration.eCountingMode == DRV_TIM_COUNTING_MODE_ONCE)
    {
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].once = 0x01u;
    }
    else
    {
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].once = 0x00u;
    }
    ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].run = 0x00u;
    ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].sel_event = ptTim->tConfiguration.eExternalEventTrigger;
    ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].sym_nasym = ptTim->tConfiguration.eSymmetric;
    ptTim->ptDevice.ptGpio->gpio_app_counter_cnt[ptTim->ulSubID] = 0x00ul;
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      NVIC_DisableIRQ((IRQn_Type) (((IRQn_Type[] ) { DRV_GPIO_COUNTER_IRQs } )[0] + ptTim->ulSubID));
      DRV_NVIC_ClearPendingIRQ((IRQn_Type) (((IRQn_Type[] ) { DRV_GPIO_COUNTER_IRQs } )[0] + ptTim->ulSubID));
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].irq_en = 1u;
      ptTim->ptDevice.ptGpio->gpio_app_cnt_irq_raw = 0x01ul << ptTim->ulSubID;
    }
    else
    {
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].irq_en = 0u;
    }
    for(id = 0; id <= DRV_DIO_ID_GPIO_MAX - DRV_DIO_ID_GPIO_MIN; id++)
    {
      if(((uint32_t) ptTim->tConfiguration.eDioMskOutputReference & (((uint32_t) DRV_DIO_MSK_GPIO_0 ) << id)) > 0)
      {
        ptTim->ptDevice.ptGpio->gpio_app_cfg_b[id].count_ref = ptTim->ulSubID;
      }
    }
    ptTim->eState = DRV_TIM_STATE_STOPPED;
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_TIMER_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_TIMER_MAX)
  {
    if(ptTim->tConfiguration.eSymmetric == DRV_TIM_SYMMETRIC_TRIANGLE || ptTim->tConfiguration.eCountingMode >= DRV_TIM_COUNTING_MODE_RESERVED)
    {
      return DRV_ERROR_PARAM;
    }
    ptTim->ulSubID = (uint32_t) ptTim->tConfiguration.eDeviceID - (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MIN;
    s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID] = 0;
    ptTim->ptDevice.ptTimer = DRV_TIMER_DEVICE;
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL && ptTim->tConfiguration.eCountingMode == DRV_TIM_COUNTING_MODE_CONTINUOUS)
    {
      ptTim->ptDevice.ptTimer->timer_config_timer_b[ptTim->ulSubID].mode = DRV_TIM_COUNTING_MODE_ONCE;
    }
    else
    {
      ptTim->ptDevice.ptTimer->timer_config_timer_b[ptTim->ulSubID].mode = ptTim->tConfiguration.eCountingMode;
    }
    if(ptTim->tConfiguration.eSystimeReference == DRV_TIM_SYSTIME_REFERENCE_DEFAULT)
    {
      ptTim->tConfiguration.eSystimeReference = DRV_TIM_SYSTIME_REFERENCE_APP;
    }
    ptTim->ptDevice.ptTimer->timer_config_timer_b[ptTim->ulSubID].systime_config = (uint32_t) ptTim->tConfiguration.eSystimeReference
      - (uint32_t) DRV_TIM_SYSTIME_REFERENCE_MIN;
    ptTim->ptDevice.ptTimer->timer_preload_timer[ptTim->ulSubID] = 0ul;
    ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID] = 0ul;
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      NVIC_DisableIRQ((IRQn_Type) (((IRQn_Type[] ) { DRV_TIMER_IRQs } )[0] + ptTim->ulSubID));
      DRV_NVIC_ClearPendingIRQ((IRQn_Type) (((IRQn_Type[] ) { DRV_TIMER_IRQs } )[0] + ptTim->ulSubID));
      ptTim->ptDevice.ptTimer->timer_irq_raw = 0x01ul << ptTim->ulSubID;
    }
    ptTim->eState = DRV_TIM_STATE_STOPPED;
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIMECOMPARE)
  {
    ptTim->ptDevice.ptTimer = DRV_TIMER_DEVICE;
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      NVIC_DisableIRQ(DRV_SYSTIME_COMPARE_IRQ);
      DRV_NVIC_ClearPendingIRQ(DRV_SYSTIME_COMPARE_IRQ);
      ptTim->ptDevice.ptTimer->timer_irq_raw_b.systime_s_irq = 1;
    }
    ptTim->eState = DRV_TIM_STATE_STOPPED;
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTICK)
  {
    ptTim->ulSubID = 0;
    s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTICK + ptTim->ulSubID] = 0;
    if((ptTim->tConfiguration.tPreloadValue - 1UL) > 0x00FFFFFF)
    {
      ret = DRV_ERROR_PARAM;
    }
    else
    {
      ptTim->ptDevice.ptSysTick = (SysTick_Type*) SysTick;
      ptTim->ptDevice.ptSysTick->LOAD = (uint32_t) 0x00FFFFFF & (ptTim->tConfiguration.tPreloadValue - 1UL); /* set reload register */
      ptTim->ptDevice.ptSysTick->VAL = 0UL; /* Load the SysTick Counter Value */
      ptTim->ptDevice.ptSysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
      if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
      {
        NVIC_DisableIRQ(SysTick_IRQn);
        DRV_NVIC_ClearPendingIRQ(SysTick_IRQn);
        NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
        ptTim->ptDevice.ptSysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;/* Enable SysTick IRQ */
      }
      ret = DRV_OK;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_SYSTIME_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_SYSTIME_MAX)
  {
    ptTim->ulSubID = 0;
    s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTICK + ptTim->ulSubID] = 0;
    ptTim->ptDevice.ptSystimlt = (DRV_SYSTIME_LT_DEVICE_T*) DRV_SYSTIME_LT_DEVICE;
    ret = DRV_OK;
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptTim);
  return ret;
}

/*!
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_DeInit(DRV_TIM_HANDLE_T * const ptTim)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  /* Check the parameters */
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_GPIOCNTR_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      DRV_NVIC_DisableIRQ(
        aulIRQnTable[DRV_TIM_IRQ_GPIOTIM0 + (uint32_t) ptTim->tConfiguration.eDeviceID - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN]);
      DRV_NVIC_ClearPendingIRQ(
        aulIRQnTable[DRV_TIM_IRQ_GPIOTIM0 + (uint32_t) ptTim->tConfiguration.eDeviceID - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN]);
    }
    *ptTim = (DRV_TIM_HANDLE_T ) { 0 };
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_TIMER_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_TIMER_MAX)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      DRV_NVIC_DisableIRQ(aulIRQnTable[DRV_TIM_IRQ_TIMER0 + (uint32_t) ptTim->tConfiguration.eDeviceID - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN]);
      DRV_NVIC_ClearPendingIRQ(
        aulIRQnTable[DRV_TIM_IRQ_TIMER0 + (uint32_t) ptTim->tConfiguration.eDeviceID - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN]);
    }
    *ptTim = (DRV_TIM_HANDLE_T ) { 0 };
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIMECOMPARE)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      DRV_NVIC_DisableIRQ(aulIRQnTable[DRV_TIM_IRQ_SYSTIME_COMPARE]);
      DRV_NVIC_ClearPendingIRQ(aulIRQnTable[DRV_TIM_IRQ_SYSTIME_COMPARE]);
    }
    *ptTim = (DRV_TIM_HANDLE_T ) { 0 };
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTICK)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      DRV_NVIC_DisableIRQ(aulIRQnTable[DRV_TIM_IRQ_SYSTICK]);
      DRV_NVIC_ClearPendingIRQ(aulIRQnTable[DRV_TIM_IRQ_SYSTICK]);
    }
    *ptTim = (DRV_TIM_HANDLE_T ) { 0 };
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_SYSTIME_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_SYSTIME_MAX)
  {
    *ptTim = (DRV_TIM_HANDLE_T ) { 0 };
    ret = DRV_OK;
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }

  /* Release Lock */

  return ret;
}

/*!
 *
 * The timer is started in non blocking mode.
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_Start(DRV_TIM_HANDLE_T * const ptTim)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_GPIOCNTR_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MAX - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ptTim->ptDevice.ptGpio->gpio_app_counter_max[ptTim->ulSubID] = ptTim->tConfiguration.tPreloadValue;
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].run = 0x01u;
      ptTim->eState = DRV_TIM_STATE_RUNNING;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ptTim->ptDevice.ptGpio->gpio_app_counter_max[ptTim->ulSubID] = ptTim->tConfiguration.tPreloadValue;
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].run = 0x01u;
      ptTim->ptDevice.ptGpio->gpio_app_cnt_irq_mask_set = 0x01ul << ptTim->ulSubID;
      DRV_NVIC_EnableIRQ(aulIRQnTable[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID]);
      ptTim->eState = DRV_TIM_STATE_RUNNING;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_TIMER_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_TIMER_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MAX - (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ptTim->ptDevice.ptTimer->timer_preload_timer[ptTim->ulSubID] = ptTim->tConfiguration.tPreloadValue;
      ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID] = ptTim->tConfiguration.tPreloadValue;
      ptTim->eState = DRV_TIM_STATE_RUNNING;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      if(ptTim->tConfiguration.eCountingMode == DRV_TIM_COUNTING_MODE_ONCE)
      {
        ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID] = ptTim->tConfiguration.tPreloadValue;
      }
      else
      {
        ptTim->ptDevice.ptTimer->timer_preload_timer[ptTim->ulSubID] = ptTim->tConfiguration.tPreloadValue;
      }
      ptTim->ptDevice.ptTimer->timer_irq_msk_set = 0x01ul << ptTim->ulSubID;
      DRV_NVIC_EnableIRQ(aulIRQnTable[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID]);
      ptTim->eState = DRV_TIM_STATE_RUNNING;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIMECOMPARE)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ret = DRV_NSUPP;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ptTim->ptDevice.ptTimer->timer_irq_msk_set_b.systime_s_irq = 1;
      DRV_NVIC_EnableIRQ(aulIRQnTable[DRV_SYSTIME_COMPARE_IRQ]);
      ptTim->eState = DRV_TIM_STATE_RUNNING;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTICK)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ptTim->ptDevice.ptSysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;/* Enable SysTick */
      ptTim->eState = DRV_TIM_STATE_RUNNING;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      NVIC_EnableIRQ(SysTick_IRQn);
      ptTim->ptDevice.ptSysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;/* Enable SysTick */
      ptTim->eState = DRV_TIM_STATE_RUNNING;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_SYSTIME_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_SYSTIME_MAX)
  {
    ret = DRV_NSUPP;
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  /* Return function status */
  DRV_UNLOCK(ptTim);
  return ret;
}

/*!
 * The timer is stopped in blocking mode.
 * After the function has finished the timer is inactive, no interrupts will occur anymore and the dma has also stopped.
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_Stop(DRV_TIM_HANDLE_T * const ptTim)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_GPIOCNTR_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MAX - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID] = 0;
      ptTim->ptDevice.ptGpio->gpio_app_counter_cnt[ptTim->ulSubID] = 0;
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].run = 0x00u;
      ptTim->eState = DRV_TIM_STATE_STOPPED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID] = 0;
      DRV_NVIC_DisableIRQ(aulIRQnTable[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID]);
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].run = 0x00u;
      ptTim->ptDevice.ptGpio->gpio_app_cnt_irq_mask_rst = 0x01ul << ptTim->ulSubID;
      ptTim->ptDevice.ptGpio->gpio_app_cnt_irq_raw = 0x01ul << ptTim->ulSubID;
      ptTim->ptDevice.ptGpio->gpio_app_counter_cnt[ptTim->ulSubID] = 0;
      ptTim->eState = DRV_TIM_STATE_STOPPED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_TIMER_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_TIMER_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MAX - (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID] = 0;
      ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID] = 0;
      ptTim->ptDevice.ptTimer->timer_preload_timer[ptTim->ulSubID] = 0;
      ptTim->eState = DRV_TIM_STATE_STOPPED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID] = 0;
      DRV_NVIC_DisableIRQ(aulIRQnTable[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID]);
      ptTim->ptDevice.ptTimer->timer_irq_msk_reset = 0x01ul << ptTim->ulSubID;
      ptTim->ptDevice.ptTimer->timer_irq_raw = 0x01ul << ptTim->ulSubID;
      ptTim->ptDevice.ptTimer->timer_preload_timer[ptTim->ulSubID] = 0;
      ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID] = 0;
      ptTim->eState = DRV_TIM_STATE_STOPPED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIMECOMPARE)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ret = DRV_NSUPP;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      DRV_NVIC_DisableIRQ(aulIRQnTable[DRV_TIM_IRQ_SYSTIME_COMPARE]);
      ptTim->ptDevice.ptTimer->timer_irq_msk_reset_b.systime_s_irq = 1u;
      ptTim->ptDevice.ptTimer->timer_irq_raw_b.systime_s_irq = 1u;
      ptTim->eState = DRV_TIM_STATE_STOPPED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTICK)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTICK] = 0;
      ptTim->ptDevice.ptSysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;/* Disable SysTick  */
      ptTim->ptDevice.ptSysTick->VAL = 0ul;
      ptTim->eState = DRV_TIM_STATE_STOPPED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTICK] = 0;
      DRV_NVIC_DisableIRQ(aulIRQnTable[DRV_TIM_IRQ_SYSTICK]);
      ptTim->ptDevice.ptSysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;/* Disable SysTick  */
      ptTim->ptDevice.ptSysTick->VAL = 0ul;
      ptTim->eState = DRV_TIM_STATE_STOPPED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_SYSTIME_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_SYSTIME_MAX)
  {
    ret = DRV_NSUPP;
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptTim);
  /* Return function status */
  return ret;
}

/*!
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_Pause(DRV_TIM_HANDLE_T * const ptTim)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_GPIOCNTR_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MAX - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].run = 0x00u;
      ptTim->eState = DRV_TIM_STATE_PAUSED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      DRV_NVIC_DisableIRQ(aulIRQnTable[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID]);
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].run = 0x00u;
      ptTim->ptDevice.ptGpio->gpio_app_cnt_irq_mask_rst = 0x01ul << ptTim->ulSubID;
      ptTim->eState = DRV_TIM_STATE_PAUSED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_TIMER_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_TIMER_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MAX - (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ret = DRV_NSUPP;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ret = DRV_NSUPP;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIMECOMPARE)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ret = DRV_NSUPP;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ret = DRV_NSUPP;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTICK)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ptTim->ptDevice.ptSysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;/* Disable SysTick */
      ptTim->eState = DRV_TIM_STATE_PAUSED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      NVIC_DisableIRQ(SysTick_IRQn);
      ptTim->ptDevice.ptSysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;/* Disable SysTick */
      ptTim->eState = DRV_TIM_STATE_PAUSED;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_SYSTIME_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_SYSTIME_MAX)
  {
    ret = DRV_NSUPP;
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptTim);
  /* Return function status */
  return ret;
}

/*!
 *
 * If the value uSkipCnt is 0, the function will wait until the next event has
 * occurred. If the value is at least 1 the function will wait until the given
 * ammount of events have occurred.
 * With a timer of 1ms and a ulSkipCnt of 100 the function will wait for
 * 100ms + 1ms (first event) + runtime > 100.5ms
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \param[in] ulTickCnt Count of events to skip.
 * \return DRV_ERROR
 *         DRV_ERROR_PARAM
 *         DRV_OK
 *         DRV_NSUPP
 */
DRV_STATUS_E DRV_TIM_Wait(DRV_TIM_HANDLE_T * const ptTim, uint32_t ulTickCnt)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ulTickCnt != 0
    && (ptTim->tConfiguration.eCountingMode == DRV_TIM_COUNTING_MODE_ONCE || ptTim->tConfiguration.eCountingMode == DRV_TIM_COUNTING_MODE_RESERVED))
  {
    DRV_UNLOCK(ptTim);
    return DRV_ERROR_PARAM;
  }
  if(ptTim->eState != DRV_TIM_STATE_RUNNING)
  {
    DRV_UNLOCK(ptTim);
    return DRV_ERROR;
  }
  if(ulTickCnt == 0)
  {
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_GPIOCNTR_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MAX - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].once = 1;
      for(; ulTickCnt > 0; ulTickCnt--)
      {
        ptTim->ptDevice.ptGpio->gpio_app_counter_cnt[ptTim->ulSubID] = 0;
        while(ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID] != 0)
        {
        }
        if(s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID] != 0)
        {
          s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID](ptTim, s_apTIMCallbackHandleTable[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID]);
        }
      }
      ptTim->ptDevice.ptGpio->gpio_app_counter_ctrl_b[ptTim->ulSubID].once =
        (ptTim->tConfiguration.eCountingMode == DRV_TIM_COUNTING_MODE_ONCE) ? 1 : 0;
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      uint32_t latch = s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID];
      while(s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID] - latch <= ulTickCnt)
      {
      }
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_TIMER_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_TIMER_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MAX - (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ptTim->ptDevice.ptTimer->timer_config_timer_b[ptTim->ulSubID].mode = DRV_TIM_COUNTING_MODE_ONCE;
      for(; ulTickCnt > 0; ulTickCnt--)
      {
        ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID] = ptTim->tConfiguration.tPreloadValue;
        while(ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID] != 0)
        {
        }
        if(s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID] != 0)
        {
          s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID](ptTim, s_apTIMCallbackHandleTable[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID]);
        }
      }
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      uint32_t latch = s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID];
      while(s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID] - latch <= ulTickCnt)
      {
      }
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIMECOMPARE)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL || ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      while(DRV_TIMER_DEVICE->timer_systime_s < ulTickCnt)
      {
      }
      volatile uint32_t ulDummy;
      ulDummy = DRV_TIMER_DEVICE->timer_systime_ns;
      UNUSED(ulDummy);
      if(s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTIME_COMPARE] != 0)
      {
        s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTIME_COMPARE](ptTim, s_apTIMCallbackHandleTable[DRV_TIM_IRQ_SYSTIME_COMPARE]);
      }
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTICK)
  {
    if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ptTim->ptDevice.ptSysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;
      for(; ulTickCnt > 0; ulTickCnt--)
      {
        while((ptTim->ptDevice.ptSysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0)
        {
          if(s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTICK] != 0)
          {
            s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTICK](ptTim, s_apTIMCallbackHandleTable[DRV_TIM_IRQ_SYSTICK]);
          }
        }
      }
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      uint32_t latch = s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTICK];
      while(s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTICK] - latch <= ulTickCnt)
      {
      }
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_NSUPP;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_SYSTIME_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_SYSTIME_MAX)
  {
    ret = DRV_NSUPP;
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptTim);
  /* Return function status */
  return ret;
}

/*!
 * This function sets the threshold of a digital input output channel.
 *
 * The PWM is set if the counter is greater or equal the threshold, so it
 * defines the low time if the channel is not inverted.
 * In triangle, both values are divided by 2 so that the low time is the same
 * as in sawtooth.
 *
 * \memberof DRV_TIM_HANDLE_T
 * \public
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \param[in] ulChannelID  The id of the channel to be configured
 * \param[in] ulTC     The counter used as reference
 * \return An error code describing the current status of the driver.
 */
DRV_STATUS_E DRV_TIM_ChannelSetThreshold(DRV_TIM_HANDLE_T * const ptTim, DRV_DIO_ID_T ulChannelID, uint32_t ulTC)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ulChannelID <= DRV_DIO_ID_GPIO_MAX && ulChannelID >= DRV_DIO_ID_GPIO_MIN)
  {
    if(ptTim->tConfiguration.eDeviceID < DRV_TIM_DEVICE_ID_GPIOCNTR_MIN || ptTim->tConfiguration.eDeviceID > DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(((uint32_t) ptTim->tConfiguration.eDioMskOutputReference & (0x01ul << ulChannelID)) > 0)
    {
      ptTim->ptDevice.ptGpio->gpio_app_tc[ulChannelID - DRV_DIO_ID_GPIO_MIN] = ulTC;
      ret = DRV_OK;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptTim);
  return ret;
}

/*!
 * This function gets the threshold/capture of a digital input output channel. It is used if the
 * specified channel is in Blink, PWM, PWM2 or capture mode.
 *
 * \memberof DRV_TIM_HANDLE_T
 * \public
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \param[in] ulChannelID  The id of the channel to be read
 * \param[out] ulValue     The captured counter value used as reference
 * \return An error code describing the current status of the driver.
 */
DRV_STATUS_E DRV_TIM_ChannelGetCapture(DRV_TIM_HANDLE_T * const ptTim, DRV_DIO_ID_T ulChannelID, uint32_t * const ulValue)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  if(ulValue == 0)
  {
    DRV_UNLOCK(ptTim);
    return DRV_ERROR_PARAM;
  }
  DRV_STATUS_E ret = DRV_ERROR;
  if(ulChannelID <= DRV_DIO_ID_GPIO_MAX && ulChannelID >= DRV_DIO_ID_GPIO_MIN)
  {
    if(ptTim->tConfiguration.eDeviceID < DRV_TIM_DEVICE_ID_GPIOCNTR_MIN || ptTim->tConfiguration.eDeviceID > DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
    {
      ret = DRV_ERROR_PARAM;
    }
    else if(((uint32_t) ptTim->tConfiguration.eDioMskOutputReference & (0x01ul << ulChannelID)) > 0)
    {
      *ulValue = ptTim->ptDevice.ptGpio->gpio_app_tc[ulChannelID - DRV_DIO_ID_GPIO_MIN];
      ret = DRV_OK;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptTim);
  return ret;
}

/*!
 * This method will attach a user given callback with the user given pointer to the interrupt of the given
 * timer device context object.
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \param[in] pfnUserClb The callback to be called.
 * \param[in] pvUser The handle given as second function parameter.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_IRQAttach(DRV_TIM_HANDLE_T * const ptTim, DRV_CALLBACK_F pfnUserClb, void* pvUser)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  switch (ptTim->tConfiguration.eDeviceID)
  {
  case DRV_TIM_DEVICE_ID_GPIOCNTR0:
    s_apTIMCallbackHandleTable[DRV_TIM_IRQ_GPIOTIM0] = pvUser;
    s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_GPIOTIM0] = ptTim;
    s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM0] = pfnUserClb;
    ret = DRV_OK;
    break;
  case DRV_TIM_DEVICE_ID_GPIOCNTR1:
    s_apTIMCallbackHandleTable[DRV_TIM_IRQ_GPIOTIM1] = pvUser;
    s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_GPIOTIM1] = ptTim;
    s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM1] = pfnUserClb;
    ret = DRV_OK;
    break;
  case DRV_TIM_DEVICE_ID_GPIOCNTR2:
    s_apTIMCallbackHandleTable[DRV_TIM_IRQ_GPIOTIM2] = pvUser;
    s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_GPIOTIM2] = ptTim;
    s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM2] = pfnUserClb;
    ret = DRV_OK;
    break;

  case DRV_TIM_DEVICE_ID_TIMER0:
    s_apTIMCallbackHandleTable[DRV_TIM_IRQ_TIMER0] = pvUser;
    s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_TIMER0] = ptTim;
    s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER0] = pfnUserClb;
    ret = DRV_OK;
    break;
  case DRV_TIM_DEVICE_ID_TIMER1:
    s_apTIMCallbackHandleTable[DRV_TIM_IRQ_TIMER1] = pvUser;
    s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_TIMER1] = ptTim;
    s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER1] = pfnUserClb;
    ret = DRV_OK;
    break;
  case DRV_TIM_DEVICE_ID_TIMER2:
    s_apTIMCallbackHandleTable[DRV_TIM_IRQ_TIMER2] = pvUser;
    s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_TIMER2] = ptTim;
    s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER2] = pfnUserClb;
    ret = DRV_OK;
    break;

  case DRV_TIM_DEVICE_ID_SYSTIMECOMPARE:
    s_apTIMCallbackHandleTable[DRV_TIM_IRQ_SYSTIME_COMPARE] = pvUser;
    s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_SYSTIME_COMPARE] = ptTim;
    s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTIME_COMPARE] = pfnUserClb;
    ret = DRV_OK;
    break;

  case DRV_TIM_DEVICE_ID_SYSTICK:
    s_apTIMCallbackHandleTable[DRV_TIM_IRQ_SYSTICK] = pvUser;
    s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_SYSTICK] = ptTim;
    s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTICK] = pfnUserClb;
    ret = DRV_OK;
    break;

  case DRV_TIM_DEVICE_ID_SYSTIME_COM:
  case DRV_TIM_DEVICE_ID_SYSTIME_COM_UC:
  case DRV_TIM_DEVICE_ID_SYSTIME_APP:
    ret = DRV_NSUPP;
    break;
  }
  DRV_UNLOCK(ptTim);
  return ret;
}

/*!
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \param[out] peState pointer to a DRV_TIM_STATE_E enumeration that will be modified with the current state.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_GetState(DRV_TIM_HANDLE_T * const ptTim, DRV_TIM_STATE_E * const peState)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_STATUS_E ret = DRV_OK;
  *peState = ptTim->eState;
  if(peState == 0)
  {
    return DRV_ERROR_PARAM;
  }
  switch (ptTim->eState)
  {
  case DRV_TIM_STATE_DEFAULT:
    ret = DRV_ERROR_PARAM;
    break;
  case DRV_TIM_STATE_ERROR:
    ret = DRV_ERROR;
    break;
  case DRV_TIM_STATE_RUNNING:
    ret = DRV_BUSY;
    break;
  case DRV_TIM_STATE_STOPPED:
    ret = DRV_OK;
    break;
  case DRV_TIM_STATE_TIMEOUT:
    ret = DRV_TOUT;
    break;
  default:
    ret = DRV_ERROR_PARAM;
    break;
  }
  return ret;
}

/*!
 * This function stores the current counter values in the given pointer position. A software event is
 * triggered if the hardware counter overflows by reaching its border. Keep in mind, that a counter
 * iterates and a timer decrements.
 *
 * If the hardware counters border/preload is set to e.g. 1ms it will overflow each ms and the
 * software counter is incremented.
 *
 * The software counter is iterated by the interrupt handlers and works only in the interrupt operation mode.
 *
 * The systimes have to be latched before read. The software counter value contains the seconds and the
 * hardware counter value contains the nano seconds of the systime counter.
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \param[out] ptSoftwareValue pointer to a uint32_t where the current software counter value is written to.
 * \param[out] ptHardwareValue pointer to a uint32_t where the current hardware counter value is written to.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_GetValue(DRV_TIM_HANDLE_T * const ptTim, uint32_t * const ptSoftwareValue, uint32_t * const ptHardwareValue)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_GPIOCNTR_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
  {
    if(ptTim->ulSubID <= (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MAX - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN)
    {
      if(ptSoftwareValue != NULL)
      {
        *ptSoftwareValue = s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM0 + ptTim->ulSubID];
        ret = DRV_OK;
      }
      if(ptHardwareValue != NULL)
      {
        *ptHardwareValue = ptTim->ptDevice.ptGpio->gpio_app_counter_cnt[ptTim->ulSubID];
        ret = DRV_OK;
      }
      else
      {
        ret = DRV_ERROR_PARAM;
      }
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_TIMER_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_TIMER_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MAX - (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else
    {
      if(ptSoftwareValue != NULL)
      {
        *ptSoftwareValue = s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER0 + ptTim->ulSubID];
        ret = DRV_OK;
      }
      if(ptHardwareValue != NULL)
      {
        *ptHardwareValue = ptTim->ptDevice.ptTimer->timer_timer[ptTim->ulSubID];
        ret = DRV_OK;
      }
      else
      {
        ret = DRV_ERROR_PARAM;
      }
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIMECOMPARE)
  {
    if(ptSoftwareValue != NULL)
    {
      *ptSoftwareValue = ptTim->ptDevice.ptTimer->timer_systime_s;
    }
    if(ptHardwareValue != NULL)
    {
      *ptHardwareValue = ptTim->ptDevice.ptTimer->timer_systime_ns;
    }
    else
    {
      volatile uint32_t ulDummy;
      ulDummy = ptTim->ptDevice.ptTimer->timer_systime_ns;
      UNUSED(ulDummy);
    }
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTICK)
  {
    if(ptSoftwareValue != NULL)
    {
      *ptSoftwareValue = s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTICK + ptTim->ulSubID];
      ret = DRV_OK;
    }
    if(ptHardwareValue != NULL)
    {
      *ptHardwareValue = ptTim->ptDevice.ptSysTick->VAL;
      ret = DRV_OK;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_SYSTIME_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_SYSTIME_MAX)
  {

    if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIME_APP)
    {
      if(ptSoftwareValue != NULL)
      {
        *ptSoftwareValue = DRV_SYSTIME_LT_DEVICE->intlogic_lt_systime_app_s;
      }
      if(ptHardwareValue != NULL)
      {
        *ptHardwareValue = DRV_SYSTIME_LT_DEVICE->intlogic_lt_systime_app_ns;
      }
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIME_COM)
    {
      if(ptSoftwareValue != NULL)
      {
        *ptSoftwareValue = DRV_SYSTIME_LT_DEVICE->intlogic_lt_systime_com_s;
      }
      if(ptHardwareValue != NULL)
      {
        *ptHardwareValue = DRV_SYSTIME_LT_DEVICE->intlogic_lt_systime_com_ns;
      }
      ret = DRV_OK;
    }
    else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIME_COM_UC)
    {
      if(ptSoftwareValue != NULL)
      {
        *ptSoftwareValue = DRV_SYSTIME_LT_DEVICE->intlogic_lt_systime_com_uc_s;
      }
      if(ptHardwareValue != NULL)
      {
        *ptHardwareValue = DRV_SYSTIME_LT_DEVICE->intlogic_lt_systime_com_uc_ns;
      }
      ret = DRV_OK;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptTim);
  /* Return function status */
  return ret;
}

/*!
 *
 * \param[in,out] ptTim pointer to a DRV_TIM_HANDLE_T structure.
 * \param[in] tCounterValue contains the value set as new preload.
 * \return Driver status
 */
DRV_STATUS_E DRV_TIM_SetPreload(DRV_TIM_HANDLE_T * const ptTim, DRV_TIM_PRELOAD_VALUE_E const tCounterValue)
{
  DRV_HANDLE_CHECK(ptTim);
  DRV_LOCK(ptTim);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_GPIOCNTR_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_GPIOCNTR_MAX)
  {
    if(ptTim->ulSubID <= (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MAX - (uint32_t) DRV_TIM_DEVICE_ID_GPIOCNTR_MIN)
    {
      ptTim->tConfiguration.tPreloadValue = tCounterValue;
      ptTim->ptDevice.ptGpio->gpio_app_counter_max[ptTim->ulSubID] = tCounterValue;
      ret = DRV_OK;
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    };
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_TIMER_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_TIMER_MAX)
  {
    if(ptTim->ulSubID > (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MAX - (uint32_t) DRV_TIM_DEVICE_ID_TIMER_MIN)
    {
      ret = DRV_ERROR_PARAM;
    }
    else
    {
      ptTim->tConfiguration.tPreloadValue = tCounterValue;
      ptTim->ptDevice.ptTimer->timer_preload_timer[ptTim->ulSubID] = tCounterValue;
      ret = DRV_OK;
    }
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTIMECOMPARE)
  {
    ptTim->tConfiguration.tPreloadValue = tCounterValue;
    ptTim->ptDevice.ptTimer->timer_compare_systime_s_value = tCounterValue;
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID == DRV_TIM_DEVICE_ID_SYSTICK)
  {
    ptTim->ptDevice.ptSysTick->LOAD = tCounterValue;
    ret = DRV_OK;
  }
  else if(ptTim->tConfiguration.eDeviceID >= DRV_TIM_DEVICE_ID_SYSTIME_MIN && ptTim->tConfiguration.eDeviceID <= DRV_TIM_DEVICE_ID_SYSTIME_MAX)
  {
    ret = DRV_NSUPP;
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptTim);
  /* Return function status */
  return ret;
}

/*!
 * \brief  This function handles interrupts requests.

 * \return None
 */
void GPIOTIM0_IRQHandler(void)
{
  s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM0]++;
  DRV_TIM_GPIOTIM0_Callback();
  DRV_GPIO_DEVICE->gpio_app_cnt_irq_raw = gpio_xpic_app_gpio_app_cnt_irq_raw_cnt0_Msk;
}

/*!
 * It is called from the Interrupt service routine. And calls the function attached to the interrupt by
 * \ref DRV_TIM_IRQAttach() , if there is one. Because the implementation has the attribute weak it
 * is possible to implement the callback inside the application context.
 * EOI-handling and source clearing not necessary.
 */
__WEAK void DRV_TIM_GPIOTIM0_Callback(void)
{
  if(s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM0] != 0)
  {
    s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM0](s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_GPIOTIM0],
      s_apTIMCallbackHandleTable[DRV_TIM_IRQ_GPIOTIM0]);
  }
}

/*!
 * \brief  This function handles interrupts requests.

 * \return None
 */
void GPIOTIM1_IRQHandler(void)
{
  s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM1]++;
  DRV_TIM_GPIOTIM1_Callback();
  DRV_GPIO_DEVICE->gpio_app_cnt_irq_raw = gpio_xpic_app_gpio_app_cnt_irq_raw_cnt1_Msk;
}

/*!
 * It is called from the Interrupt service routine. And calls the function attached to the interrupt by
 * \ref DRV_TIM_IRQAttach() , if there is one. Because the implementation has the attribute weak it
 * is possible to implement the callback inside the application context.
 * EOI-handling and source clearing not necessary.
 */
__WEAK void DRV_TIM_GPIOTIM1_Callback(void)
{
  if(s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM1] != 0)
  {
    s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM1](s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_GPIOTIM1],
      s_apTIMCallbackHandleTable[DRV_TIM_IRQ_GPIOTIM1]);
  }
}

/*!
 * \brief  This function handles interrupts requests.
 * \return None
 */
void GPIOTIM2_IRQHandler(void)
{
  s_apTIMIrqCntr[DRV_TIM_IRQ_GPIOTIM2]++;
  DRV_TIM_GPIOTIM2_Callback();
  DRV_GPIO_DEVICE->gpio_app_cnt_irq_raw = gpio_xpic_app_gpio_app_cnt_irq_raw_cnt2_Msk;
}

/*!
 * It is called from the Interrupt service routine. And calls the function attached to the interrupt by
 * \ref DRV_TIM_IRQAttach() , if there is one. Because the implementation has the attribute weak it
 * is possible to implement the callback inside the application context.
 * EOI-handling and source clearing not necessary.
 */
__WEAK void DRV_TIM_GPIOTIM2_Callback(void)
{
  if(s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM2] != 0)
  {
    s_apTIMCallbackTable[DRV_TIM_IRQ_GPIOTIM2](s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_GPIOTIM2],
      s_apTIMCallbackHandleTable[DRV_TIM_IRQ_GPIOTIM2]);
  }
}

/*!
 * \brief  This function handles interrupts requests.
 * \return None
 */
void TIM0_IRQHandler(void)
{
  s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER0]++;
  DRV_TIM_TIMER0_Callback();
  DRV_TIMER_DEVICE->timer_irq_raw = timer_app_timer_irq_masked_timer0_irq_Msk;
}

/*!
 * It is called from the Interrupt service routine. And calls the function attached to the interrupt by
 * \ref DRV_TIM_IRQAttach() , if there is one. Because the implementation has the attribute weak it
 * is possible to implement the callback inside the application context.
 * EOI-handling and source clearing not necessary.
 */
__WEAK void DRV_TIM_TIMER0_Callback(void)
{
  if(s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER0] != 0)
  {
    s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER0](s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_TIMER0], s_apTIMCallbackHandleTable[DRV_TIM_IRQ_TIMER0]);
  }
}

/*!
 * \brief  This function handles interrupts requests.
 * \return None
 */
void TIM1_IRQHandler(void)
{
  s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER1]++;
  DRV_TIM_TIMER1_Callback();
  DRV_TIMER_DEVICE->timer_irq_raw = timer_app_timer_irq_masked_timer1_irq_Msk;
}

/*!
 * It is called from the Interrupt service routine. And calls the function attached to the interrupt by
 * \ref DRV_TIM_IRQAttach() , if there is one. Because the implementation has the attribute weak it
 * is possible to implement the callback inside the application context.
 * EOI-handling and source clearing not necessary.
 */
__WEAK void DRV_TIM_TIMER1_Callback(void)
{
  if(s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER1] != 0)
  {
    s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER1](s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_TIMER1], s_apTIMCallbackHandleTable[DRV_TIM_IRQ_TIMER1]);
  }
}

/*!
 * \brief  This function handles interrupts requests.
 * \return None
 */
void TIM2_IRQHandler(void)
{
  s_apTIMIrqCntr[DRV_TIM_IRQ_TIMER2]++;
  DRV_TIM_TIMER2_Callback();
  DRV_TIMER_DEVICE->timer_irq_raw = timer_app_timer_irq_masked_timer2_irq_Msk;
}

/*!
 * It is called from the Interrupt service routine. And calls the function attached to the interrupt by
 * \ref DRV_TIM_IRQAttach() , if there is one. Because the implementation has the attribute weak it
 * is possible to implement the callback inside the application context.
 * EOI-handling and source clearing not necessary.
 */
__WEAK void DRV_TIM_TIMER2_Callback(void)
{
  if(s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER2] != 0)
  {
    s_apTIMCallbackTable[DRV_TIM_IRQ_TIMER2](s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_TIMER2], s_apTIMCallbackHandleTable[DRV_TIM_IRQ_TIMER2]);
  }
}

/*!
 * \brief  This function handles interrupts requests.
 * \return None
 */
void SYSTIME_IRQHandler(void)
{
  s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTIME_COMPARE]++;
  DRV_TIM_SYSTIME_COMPARE_Callback();
  DRV_TIMER_DEVICE->timer_irq_raw = timer_app_timer_irq_masked_systime_s_irq_Msk;
}

/*!
 * It is called from the Interrupt service routine. And calls the function attached to the interrupt by
 * \ref DRV_TIM_IRQAttach() , if there is one. Because the implementation has the attribute weak it
 * is possible to implement the callback inside the application context.
 * EOI-handling and source clearing not necessary.
 */
__WEAK void DRV_TIM_SYSTIME_COMPARE_Callback(void)
{
  if(s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTIME_COMPARE] != 0)
  {
    s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTIME_COMPARE](s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_SYSTIME_COMPARE],
      s_apTIMCallbackHandleTable[DRV_TIM_IRQ_SYSTIME_COMPARE]);
  }
}

/*!
 * \brief  This function handles interrupts requests.
 * \return None
 */
void DRVSysTick_Handler(void)
{
  s_apTIMIrqCntr[DRV_TIM_IRQ_SYSTICK]++;
  DRV_TIM_ARMSYSTICK_Callback();
}

/*!
 * It is called from the Interrupt service routine. And calls the function attached to the interrupt by
 * \ref DRV_TIM_IRQAttach() , if there is one. Because the implementation has the attribute weak it
 * is possible to implement the callback inside the application context.
 * EOI-handling and source clearing not necessary.
 */
__WEAK void DRV_TIM_ARMSYSTICK_Callback(void)
{
  if(s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTICK] != 0)
  {
    s_apTIMCallbackTable[DRV_TIM_IRQ_SYSTICK](s_apTIMCallbackDriverHandleTable[DRV_TIM_IRQ_SYSTICK], s_apTIMCallbackHandleTable[DRV_TIM_IRQ_SYSTICK]);
  }
}

/*! \} *//* End of group TIM */

/*lint -restore */
/*lint -restore */

#endif /* DRV_TIM_MODULE_DISABLED */
