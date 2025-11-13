/**************************************************************************//**
 * \file     netx_drv_dmac.c
 * \brief    DMA controller module driver.
 * $Revision: 11368 $
 * $Date: 2024-06-04 18:17:06 +0300 (Tue, 04 Jun 2024) $
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
#ifdef DRV_DMAC_MODULE_ENABLED
/*! \addtogroup DMAC DMAC
 * \{
 * \brief This is the  DMAC driver, defined by DRV_DMAC_HANDLE_T.
 *
 * \details
 * The DMAC is a device, which is a master in the internal bus interface thats only purpose is
 * to read from one memory address and write to an other. This device of course has some more
 * options like iterating the source and/or destination address and if a word, byte or half
 * word is read or written. This will be configured in the configuration part of the context
 * object. This handles values shall not be modified other than the configuration. In any case,
 * after the device was initialized by its handle, one should only use the api functions to
 * interact with the device.
 *
 * The device
 */

/*!
 * \brief Table of the device addresses.
 *
 * Used to identify the device addresses by the device id.
 */
static DRV_DMAC_CH_DEVICE_T * const s_apChannelDeviceAddressTable[DRV_DMAC_CH_DEVICE_COUNT] = DRV_DMAC_CH_DEVICE_LIST;
/*!
 * \brief Used for mapping the handle to an interrupt.
 *
 * Threadsafe and reentrant because its is only written in normal context an used in interrupt context of the specific interrupt.
 */
static DRV_DMAC_HANDLE_T * s_apChannelHandleAddressTable[DRV_DMAC_CH_DEVICE_COUNT] = { 0 };
void * s_apDeviceChannelCallbackHandleTable[DRV_DMAC_CH_DEVICE_COUNT] = { 0 };

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
  }\
  if(ptSequencer->tConfiguration.eTransferWidthSource > DRV_DMAC_TRANSFER_WIDTH_MAX)\
  {\
    return DRV_ERROR_PARAM;\
  }
#else
#define DRV_HANDLE_CHECK(handle) ;
#endif

#define DRV_DMAC_LINKED_LIST_TRANSFER_SIZE        (4092u)

/*!
 * \brief This function initializes the dmac context object.
 *
 * The init function takes a context object of the dmac as input. The DRV_DMAC_HANDLE_T is this object,
 * containing the devices configuration and some private attributes of the driver.
 * Those attributes are initialized by the given configuration parameters.
 *
 * One should not interact (change values) with the context object itself after initializing it.
 * However it is possible in some cases to wait until the device is not busy anymore, modify
 * the configuration and reinitialize the context again.
 *
 * In case of an error during initialization, the function returns an error value, but the lock for the handle, will not be released.
 * Together with the return value, this will ensure that the driver can only be interacted with if it has been correctly initialized.
 * Since the lock is initialized when this function is called, this function can still be called again with other parameters after a failed initialization.
 *
 * \memberof DRV_DMAC_HANDLE_T
 * \param[in,out] ptSequencer The handle of the driver
 * \return DRV_OK If the device is initialized correctly
 *         DRV_ERROR_PARAM If a combination or a single parameter is not correct
 */
DRV_STATUS_E DRV_DMAC_Init(DRV_DMAC_HANDLE_T * const ptSequencer)
{
  if(ptSequencer == 0)
  {
    return DRV_ERROR_PARAM;
  }
  ptSequencer->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptSequencer);
#if (DRV_DMAC_LIST_LENGTH < 1)
  return DRV_ERROR_PARAM;
#endif
  // activate clock for app dmac
  DRV_ASIC_CTRL_DEVICE->asic_ctrl_access_key = DRV_ASIC_CTRL_DEVICE->asic_ctrl_access_key;
#ifdef DRV_DEVICE_NETX_90
  DRV_ASIC_CTRL_DEVICE->clock_enable1 = asic_ctrl_clock_enable1_dma_app_Msk | asic_ctrl_clock_enable1_dma_app_wm_Msk;
#else
#ifdef DRV_DEVICE_NETX_90_MPW
  DRV_ASIC_CTRL_DEVICE->clock_enable1 = asic_ctrl_clock_enable1_dma_app_Msk;
#endif
#endif
  // reset the callback and enable irq to cope with irq clearing of pending state at repeated startup
  s_apChannelHandleAddressTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN] = 0;
  NVIC_ClearPendingIRQ(DRV_DMAC_REG_IRQ);
  NVIC_EnableIRQ(DRV_DMAC_REG_IRQ);
  s_apChannelHandleAddressTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN] = ptSequencer;
  DMAC_APP_REG_DEVICE->dmac_config = 1;
  ptSequencer->ptDevice = s_apChannelDeviceAddressTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN];
  if(ptSequencer->tConfiguration.fCallbackComplete != 0)
  {
    ptSequencer->ptDevice->dmac_chcfg_b.ITC = 1;
  }
  else
  {
    ptSequencer->ptDevice->dmac_chcfg_b.ITC = 0;
  }
  ptSequencer->ptDevice->dmac_chcfg_b.SrcPeripheral = 0u + 2 * ((uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN);
  ptSequencer->ptDevice->dmac_chcfg_b.DestPeripheral = 1u
    + 2 * ((uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN);
  if(ptSequencer->tConfiguration.ePeripheralSource == DRV_DMAC_PERIPHERAL_MEMORY
    && ptSequencer->tConfiguration.ePeripheralDest == DRV_DMAC_PERIPHERAL_MEMORY)
  {
    ptSequencer->ptDevice->dmac_chcfg_b.FlowCntrl = DRV_DMAC_FLOW_MEM_TO_MEM_BY_DMAC;
  }
  else if(ptSequencer->tConfiguration.ePeripheralSource == DRV_DMAC_PERIPHERAL_MEMORY)
  {
    if(ptSequencer->tConfiguration.eFlowController == DRV_DMAC_FLOW_CONTROLLER_DMAC)
    {
      ptSequencer->ptDevice->dmac_chcfg_b.FlowCntrl = DRV_DMAC_FLOW_MEM_TO_PER_BY_DMAC;
    }
    else if(ptSequencer->tConfiguration.eFlowController == DRV_DMAC_FLOW_CONTROLLER_PER)
    {
      ptSequencer->ptDevice->dmac_chcfg_b.FlowCntrl = DRV_DMAC_FLOW_MEM_TO_PER_BY_PER;
    }
    else
    {
      return DRV_ERROR_PARAM;
    }
    (&DMAC_MUX_APP_DEVICE->dmac_mux_peripheral_input_sel0)[0
      + 2 * ((uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN)] = ptSequencer->tConfiguration.ePeripheralDest;
    (&DMAC_MUX_APP_DEVICE->dmac_mux_peripheral_input_sel0)[1
      + 2 * ((uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN)] = ptSequencer->tConfiguration.ePeripheralDest;
  }
  else if(ptSequencer->tConfiguration.ePeripheralDest == DRV_DMAC_PERIPHERAL_MEMORY)
  {
    ptSequencer->ptDevice->dmac_chcfg_b.FlowCntrl = DRV_DMAC_FLOW_PER_TO_MEM_BY_DMAC;
    if(ptSequencer->tConfiguration.eFlowController == DRV_DMAC_FLOW_CONTROLLER_DMAC)
    {
      ptSequencer->ptDevice->dmac_chcfg_b.FlowCntrl = DRV_DMAC_FLOW_PER_TO_MEM_BY_DMAC;
    }
    else if(ptSequencer->tConfiguration.eFlowController == DRV_DMAC_FLOW_CONTROLLER_PER)
    {
      ptSequencer->ptDevice->dmac_chcfg_b.FlowCntrl = DRV_DMAC_FLOW_PER_TO_MEM_BY_PER;
    }
    else
    {
      return DRV_ERROR_PARAM;
    }
    (&(DMAC_MUX_APP_DEVICE->dmac_mux_peripheral_input_sel0))[0
      + 2 * ((uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN)] = ptSequencer->tConfiguration.ePeripheralSource;
    (&(DMAC_MUX_APP_DEVICE->dmac_mux_peripheral_input_sel0))[1
      + 2 * ((uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_DMAC_DEVICE_ID_MIN)] = ptSequencer->tConfiguration.ePeripheralSource;
  }
  else
  {
    return DRV_ERROR_PARAM;
  }
  ptSequencer->ptDevice->dmac_chcfg_b.E = 0;
  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 *
 * It trys to get a lock on the handle so that it is able to be reset.
 * Then it will reset the nvic, the device states and the context given.
 *
 * \memberof DRV_DMAC_HANDLE_T
 * \param[in,out] ptSequencer The handle of the driver
 * \return DRV_OK Everything is ok.
 *         DRV_LOCKED The api is locked and not accessible.
 *         DRV_ERROR_PARAM A given parameter is not correct.
 */
DRV_STATUS_E DRV_DMAC_DeInit(DRV_DMAC_HANDLE_T * const ptSequencer)
{
  DRV_HANDLE_CHECK(ptSequencer);
  DRV_LOCK(ptSequencer);
  NVIC_ClearPendingIRQ(DRV_DMAC_REG_IRQ);
  NVIC_DisableIRQ(DRV_DMAC_REG_IRQ);
  ptSequencer->ptDevice->dmac_chcfg_b.H = 1;
  ptSequencer->ptDevice->dmac_chcfg_b.ITC = 0;
  ptSequencer->ptDevice->dmac_chcfg_b.E = 0;
  ptSequencer->ptDevice->dmac_chcfg_b.IE = 0;
  *ptSequencer = (DRV_DMAC_HANDLE_T ) { 0 };
  return DRV_OK;
}

/*!
 * \memberof DRV_DMAC_HANDLE_T
 * \param[in,out] ptSequencer The handle of the driver
 * \return DRV_OK Everything is ok.
 *         DRV_LOCKED The api is locked and not accessible.
 *         DRV_ERROR_PARAM A given parameter is not correct.
 */
DRV_STATUS_E DRV_DMAC_Start(DRV_DMAC_HANDLE_T * const ptSequencer, void * const ptBufferSource, void * const ptBufferDestination, size_t size)
{
  DRV_HANDLE_CHECK(ptSequencer);
  DRV_LOCK(ptSequencer);
  if(ptSequencer->ptDevice->dmac_chcfg_b.A != 0)
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_BUSY;
  }
  uint32_t ulWidth = 0x1ul << (uint32_t) ptSequencer->tConfiguration.eTransferWidthSource;
  if(((size % ulWidth) != 0) || (size / ulWidth) > DRV_DMAC_LINKED_LIST_TRANSFER_SIZE * DRV_DMAC_LIST_LENGTH)
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_ERROR_PARAM;
  }

  uint32_t i = 0;
  for(i = 0; i < DRV_DMAC_LIST_LENGTH; i++)
  {
    ptSequencer->atLinkedList[i].tControl.bf.eGenerateTerminalInterrupt = DRV_DMAC_GENERATE_TERMINAL_INTERRUPT_INACTIVE;
    ptSequencer->atLinkedList[i].tControl.bf.eArmEq = DRV_DMAC_ARM_EQ_DEFAULT;
    ptSequencer->atLinkedList[i].tControl.bf.eDestBurstSize = DRV_DMAC_BURST_TRANSFER_SIZE_1;
    ptSequencer->atLinkedList[i].tControl.bf.eSourceBurstSize = DRV_DMAC_BURST_TRANSFER_SIZE_1;
    ptSequencer->atLinkedList[i].tControl.bf.eDestIncrementation = ptSequencer->tConfiguration.eIncrementationDest;
    ptSequencer->atLinkedList[i].tControl.bf.eSourceIncrementation = ptSequencer->tConfiguration.eIncrementationSource;
    ptSequencer->atLinkedList[i].tControl.bf.eDestTransferWidth = ptSequencer->tConfiguration.eTransferWidthDest;
    ptSequencer->atLinkedList[i].tControl.bf.eSourceTransferWidth = ptSequencer->tConfiguration.eTransferWidthSource;

    if(DRV_DMAC_INCREMENTATION_ACTIVE == ptSequencer->tConfiguration.eIncrementationDest)
    {
      /*lint -save -e124 */
      ptSequencer->atLinkedList[i].ptDest = ptBufferDestination + (DRV_DMAC_LINKED_LIST_TRANSFER_SIZE * ulWidth * i);
      /*lint -restore */
    }
    else
    {
      ptSequencer->atLinkedList[i].ptDest = ptBufferDestination;
    }

    if(DRV_DMAC_INCREMENTATION_ACTIVE == ptSequencer->tConfiguration.eIncrementationSource)
    {
      /*lint -save -e124 */
      ptSequencer->atLinkedList[i].ptSource = ptBufferSource + (DRV_DMAC_LINKED_LIST_TRANSFER_SIZE * ulWidth * i);
      /*lint -restore */
    }
    else
    {
      ptSequencer->atLinkedList[i].ptSource = ptBufferSource;
    }

    if(size / ulWidth > DRV_DMAC_LINKED_LIST_TRANSFER_SIZE)
    {
      ptSequencer->atLinkedList[i].tControl.bf.ulTransferSize = DRV_DMAC_LINKED_LIST_TRANSFER_SIZE;
      ptSequencer->atLinkedList[i].ptNextElement = &(ptSequencer->atLinkedList[i + 1]);

      size -= DRV_DMAC_LINKED_LIST_TRANSFER_SIZE * ulWidth;
    }
    else
    {
      ptSequencer->atLinkedList[i].tControl.bf.ulTransferSize = size / ulWidth;
      ptSequencer->atLinkedList[i].ptNextElement = 0;
      ptSequencer->atLinkedList[i].tControl.bf.eGenerateTerminalInterrupt = DRV_DMAC_GENERATE_TERMINAL_INTERRUPT_ACTIVE;
      break;
    }
  }

  // copy first element
  ptSequencer->ptDevice->dmac_chsrc_ad = ptSequencer->atLinkedList[0].ptSource;
  ptSequencer->ptDevice->dmac_chdest_ad = ptSequencer->atLinkedList[0].ptDest;
  ptSequencer->ptDevice->dmac_chlink = (volatile void *) ptSequencer->atLinkedList[0].ptNextElement;
  ptSequencer->ptDevice->dmac_chctrl = ptSequencer->atLinkedList[0].tControl.val;
  // enable channel
  ptSequencer->ptDevice->dmac_chcfg |= dmac_app_ch_dmac_chcfg_E_Msk;
  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 * \memberof DRV_DMAC_HANDLE_T
 * \param[in,out] ptSequencer The handle of the driver
 * \return DRV_OK Everything is ok.
 *         DRV_LOCKED The api is locked and not accessible.
 *         DRV_ERROR_PARAM A given parameter is not correct.
 */
DRV_STATUS_E DRV_DMAC_Abort(DRV_DMAC_HANDLE_T * const ptSequencer)
{
  DRV_HANDLE_CHECK(ptSequencer);
  DRV_LOCK(ptSequencer);
  ptSequencer->ptDevice->dmac_chcfg_b.H = 1;
  ptSequencer->ptDevice->dmac_chcfg_b.E = 0;
  while(ptSequencer->ptDevice->dmac_chcfg_b.A != 0)
  {
  }
  ptSequencer->ptDevice->dmac_chcfg_b.H = 0;
  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 * \brief The state of the dmac is returned.
 *
 * The dmac has no exclusive device states to return. The sole state possible to return is the
 * busy flag, which is a combined state of the device and/or API beeing busy.
 *
 * \memberof DRV_DMAC_HANDLE_T
 * \param[in,out] ptSequencer The handle of the driver
 * \return DRV_OK Everything is ok.
 *         DRV_BUSY It is not possible to interact with the device or api
 */
DRV_STATUS_E DRV_DMAC_GetState(DRV_DMAC_HANDLE_T * const ptSequencer, DRV_DMAC_STATE_E * const ptState)
{
  DRV_HANDLE_CHECK(ptSequencer);
  if(ptSequencer->ptDevice->dmac_chcfg_b.E != 0)
  {
    return DRV_BUSY;
  }
  return DRV_OK;
}

/*!
 * \brief The interrupt handler of the dma controller
 *
 * Here, the interrupt is analyzed and the associated callback is executed.
 *
 * \memberof DRV_DMAC_HANDLE_T
 * \params[in,out] ptSequencer The handle of the driver
 */
#define DRV_DMAC_IRQHandler_Generator(id, _) \
if(irq & 0x1u << id )\
{\
  if(s_apChannelHandleAddressTable[id] != 0)\
  {\
    DRV_CALLBACK_F fClbk = s_apChannelHandleAddressTable[id]->tConfiguration.fCallbackComplete;\
    if(fClbk != 0)\
    {\
      void * ptDriverHandle = s_apChannelHandleAddressTable[id];\
      void * ptUserHandle = s_apChannelHandleAddressTable[id]->tConfiguration.ptCallbackHandleComplete;\
      fClbk(ptDriverHandle, ptUserHandle);\
    }\
  }\
  DMAC_APP_REG_DEVICE->dmac_inttc_clear = 0x01u << id;\
}

void DMAC_IRQHandler(void)
{
  size_t irq = DMAC_APP_REG_DEVICE->dmac_inttc_status;
  /*lint -save -e123 */
  DRV_DEF_REPEAT_EVAL(DRV_DMAC_CH_DEVICE_COUNT, DRV_DMAC_IRQHandler_Generator, ~)
  /*lint -restore */
}

/*! \} *//* End of group DMAC */

#endif /* DRV_DMAC_MODULE_ENABLED */
