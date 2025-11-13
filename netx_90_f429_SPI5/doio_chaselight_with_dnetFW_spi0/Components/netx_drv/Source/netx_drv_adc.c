/*!************************************************************************//*!
 * \file    netx_drv_adc.c
 * \brief   ADC peripheral module driver.
 *          This file provides firmware functions related to the netX90 ADC
 *          e.g.DRV_ADC_Driver_Init(), DRV_ADC_Seq_Init(), DRV_ADC_Seq_Start()
 *              DRV_ADC_Seq_GetState(), DRV_ADC_Seq_GetSample()
 * $Revision: 10737 $
 * $Date: 2023-08-18 16:36:12 +0300 (Fri, 18 Aug 2023) $
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
#ifdef DRV_ADC_MODULE_ENABLED   /* NOTE: needs 'PREDEFINED = DRV_ADC_MODULE_ENABLED' in Doxygen-Config-File if not enabled */

/*lint -save -e685   Relational operator '>' always evaluates to 'false'  */

/*! \defgroup ADC_lable  ADC
 * \{
 * \brief The ADC driver controls the NetX90 Analog to Digital Converter.
 *
 * \details
 * The ADC driver can be used to acquire digital-samples of NetX90 Analog-Signal-Inputs.
 *
 * The driver controls the NetX90-ADC-Module which consists of 4-sub-units. It provides functions for e.g.
 * - Get an actual single value (e.g. in polling mode).
 * - Get continuous sampling to automate the sampling process.
 * - Get a set of samples one after another.
 * - Get a set of samples at the same time.
 *
 * Each ADC-sub-unit is associated with an analog input-multiplexer (MUX) for the various input-channels.
 * Some configurable 'Sequencer-HW-Logic' is basically able to switch this MUX automatically
 *  when multiple channels are enabled.
 *
 * As every other driver component of the driver-package, a context- or handle-object has to be created first.
 * (one for the ADC-Block, and one for each Sequencer to be used)
 * These handles can be initialized using 'DRV_ADC_INITIALIZER' and 'DRV_ADC_SEQ_INITIALIZER'
 * but certain parameters usually need to be modified depending on application-requirements
 * before calling the initialize functions.
 * Note that when using 'polling mode' basically just one-channel should be active/enabled
 * because the sequencer will provide just the 'final-result' of all active channels via 'DRV_ADC_Seq_GetSample()'.
 *
 * Note1: Thus most practical for applications is probably the 'DRV_OPERATION_MODE_DMA' where conversion results
 * are stored by the Sequencer via 'memory-pointers' for each channel.
 * This is especially the case when multiple channels shall be converted at once,
 * because in polling- or IRQ-mode, the register used by DRV_ADC_Seq_GetSample() is updated so fast by the seq.controller
 * such that reading it by SW is not fast enough.
 *
 * Note2: make sure the destination/store-memory-address for the DMA-operation is 'internal RAM'
 * since the DMA might not reliable store data in 'external SDRAM' !
 * \note Also check out the ADC-example-applications in 'Examples_netX90_app_drv_V.x.x.x.x' found via the Hilscher web-page.
 */

/*!
 * \brief Table of the device addresses.
 *
 * Used to identify the device addresses by the device id.
 */
static DRV_ADC_DEVICE_T * const s_apDeviceAddressTable[DRV_ADC_DEVICE_COUNT] = DRV_ADC_DEVICE_LIST;

/*!
 * \brief Table of the sequencer addresses.
 *
 * Used to identify the sequencer addresses by the sequencer id.
 */
static DRV_ADC_SEQ_DEVICE_T * const s_apSubDeviceAddressTable[DRV_ADC_SEQ_DEVICE_COUNT] = DRV_ADC_SEQ_DEVICE_LIST;

/*!
 * \brief Table of the IRQ vector numbers.
 *
 * Used to identify the interrupt channels by the device id.
 */
static IRQn_Type const s_apHandleIRQnTable[DRV_ADC_SEQ_DEVICE_COUNT] = DRV_ADC_SEQ_DEVICE_IRQ_LIST;

/*!
 * \brief Used for mapping the handle to an interrupt.
 *
 * Threadsafe and reentrant because its is only written in normal context an used in interrupt context of the specific interrupt.
 */
static DRV_ADC_SEQ_HANDLE_T * s_apHandleAddressTable[DRV_ADC_SEQ_DEVICE_COUNT] = { 0 };

/*!
 * \brief The enumeration for the run/stop commands of the adc sequencer.
 */
typedef enum DRV_ADC_SEQ_COMMAND_Etag
{
  DRV_ADC_SEQ_RUN = 0x1u,  /*!< Start measurement sequence */
  DRV_ADC_SEQ_STOP = 0x0u, /*!< Stop measurement sequence  */
} DRV_ADC_SEQ_COMMAND_E;

/*!
 * The function takes a DRV_SPI_HANDLE_T pointer which contains a DRV_ADC_ATTRIBUTES_T structure.
 *
 * In case of an error during initialization, the function returns an error value, but the lock for the handle, will not be released.
 * Together with the return value, this will ensure that the driver can only be interacted with if it has been correctly initialized.
 * Since the lock is initialized when this function is called, this function can still be called again with other parameters after a failed initialization.
 *
 * \public
 * \memberof DRV_ADC_HANDLE_T
 *
 * \param[in] ptDriver Handle of ADC driver
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Driver_Init(DRV_ADC_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRet = DRV_OK;

  if(ptDriver == NULL)
  {
    return DRV_ERROR_PARAM;
  }
  ptDriver->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptDriver);
  if((ptDriver->tConfiguration.eAdcClckPeriod > DRV_ADC_CLK_PERIOD_MAX) || (ptDriver->tConfiguration.eAdcClckPeriod < DRV_ADC_CLK_PERIOD_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptDriver->tConfiguration.eVrefBufferEnable > DRV_ADC_VREF_BUFFER_MAX) || (ptDriver->tConfiguration.eVrefBufferEnable < DRV_ADC_VREF_BUFFER_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptDriver->tConfiguration.tStaticCfg01.eSoftReset > DRV_ADC_SOFT_RESET_MAX)
    || (ptDriver->tConfiguration.tStaticCfg01.eSoftReset < DRV_ADC_SOFT_RESET_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptDriver->tConfiguration.tStaticCfg01.ePowerDown > DRV_ADC_POWER_DOWN_MAX)
    || (ptDriver->tConfiguration.tStaticCfg01.ePowerDown < DRV_ADC_POWER_DOWN_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptDriver->tConfiguration.tStaticCfg23.eSoftReset > DRV_ADC_SOFT_RESET_MAX)
    || (ptDriver->tConfiguration.tStaticCfg23.eSoftReset < DRV_ADC_SOFT_RESET_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptDriver->tConfiguration.tStaticCfg23.ePowerDown > DRV_ADC_POWER_DOWN_MAX)
    || (ptDriver->tConfiguration.tStaticCfg23.ePowerDown < DRV_ADC_POWER_DOWN_MIN))
  {
    return DRV_ERROR_PARAM;
  }

  ptDriver->ptDevice = s_apDeviceAddressTable[0];

  /* madc_cfg */
  ptDriver->ptDevice->madc_cfg_b.adcclk_period = ptDriver->tConfiguration.eAdcClckPeriod;

  /* madc_adc01_static_cfg */
  ptDriver->ptDevice->madc_adc01_static_cfg_b.vref_buffer_enable = ptDriver->tConfiguration.eVrefBufferEnable;
  ptDriver->ptDevice->madc_adc01_static_cfg_b.adc01_reset_n = ptDriver->tConfiguration.tStaticCfg01.eSoftReset;
  ptDriver->ptDevice->madc_adc01_static_cfg_b.adc01_enable = ptDriver->tConfiguration.tStaticCfg01.ePowerDown;

  /* madc_adc23_static_cfg */
  ptDriver->ptDevice->madc_adc23_static_cfg_b.adc23_reset_n = ptDriver->tConfiguration.tStaticCfg23.eSoftReset;
  ptDriver->ptDevice->madc_adc23_static_cfg_b.adc23_enable = ptDriver->tConfiguration.tStaticCfg23.ePowerDown;

  DRV_UNLOCK(ptDriver);
  return eRet;
}

/*!
 * Deinitializes the adc driver device and handle.
 *
 * \public
 * \memberof DRV_ADC_HANDLE_T
 * \param[in] ptDriver Handle of ADC driver
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Driver_DeInit(DRV_ADC_HANDLE_T * const ptDriver)
{
  if(ptDriver == NULL)
  {
    return DRV_ERROR_PARAM;
  }
  DRV_LOCK(ptDriver);
  size_t iterator;
  for(iterator = 0; iterator < DRV_ADC_DEVICE_COUNT; iterator++)
  {
    if(ptDriver->aptAdcSequencer[iterator] != NULL)
    {
      return DRV_BUSY;
    }
  }
  *ptDriver = (DRV_ADC_HANDLE_T ) { 0 };
  return DRV_OK;
}

/*!
 * Starts the sequencers given by the mask (software trigger).
 * Not configured sequencers will be ignored.
 * \public
 * \memberof DRV_ADC_HANDLE_T
 * \param[in] ptDriver Handle of ADC driver
 * \param[in] eSequencerMask Mask of sequencers to start
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 * \sa DRV_ADC_Seq_Start
 */
DRV_STATUS_E DRV_ADC_Start(DRV_ADC_HANDLE_T * const ptDriver, DRV_ADC_SEQ_DEVICE_MSK_E eSequencerMask)
{
  uint32_t ulSequencerMask = (uint32_t)eSequencerMask;
  if(ptDriver == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptDriver);

  if((ulSequencerMask > DRV_ADC_SEQ_DEVICE_MSK_MAX) || (ulSequencerMask < DRV_ADC_SEQ_DEVICE_MSK_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  //TODO: Make use of DRV_ADC_SEQ_DEVICE_COUNT or DRV_ADC_SEQ_DEVICE_ID_MAX/MIN
  if(ulSequencerMask & DRV_ADC_SEQ_DEVICE_MSK_0)
  {
    if(ptDriver->aptAdcSequencer[0] != NULL)
    {
      if(ptDriver->aptAdcSequencer[0]->tConfiguration.eDeviceID == DRV_ADC_SEQ_DEVICE_ID_ADC0)
      {
        if(ptDriver->aptAdcSequencer[0]->tBuffer.fEnable == true)
        {
          ptDriver->aptAdcSequencer[0]->tBuffer.ulPosition = 0;
        }
        if(ptDriver->aptAdcSequencer[0]->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
        {
          ptDriver->aptAdcSequencer[0]->ptDevice->madc_seq_irq_raw = 0x1FFul;
          ptDriver->aptAdcSequencer[0]->ptDevice->madc_seq_irq_mask_set = 0x1FFul;
        }
        else
        {
          ptDriver->aptAdcSequencer[0]->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
        }
      }
      else
      {
        ulSequencerMask &= ~DRV_ADC_SEQ_DEVICE_MSK_0;
      }
    }
    else
    {
      ulSequencerMask &= ~DRV_ADC_SEQ_DEVICE_MSK_0;
    }
  }

  if(ulSequencerMask & DRV_ADC_SEQ_DEVICE_MSK_1)
  {
    if(ptDriver->aptAdcSequencer[1] != NULL)
    {
      if(ptDriver->aptAdcSequencer[1]->tConfiguration.eDeviceID == DRV_ADC_SEQ_DEVICE_ID_ADC1)
      {
        if(ptDriver->aptAdcSequencer[1]->tBuffer.fEnable == true)
        {
          ptDriver->aptAdcSequencer[1]->tBuffer.ulPosition = 0;
        }
        if(ptDriver->aptAdcSequencer[1]->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
        {
          ptDriver->aptAdcSequencer[1]->ptDevice->madc_seq_irq_raw = 0x1FFul;
          ptDriver->aptAdcSequencer[1]->ptDevice->madc_seq_irq_mask_set = 0x1FFul;
        }
        else
        {
          ptDriver->aptAdcSequencer[1]->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
        }
      }
      else
      {
        ulSequencerMask &= ~DRV_ADC_SEQ_DEVICE_MSK_1;
      }
    }
    else
    {
      ulSequencerMask &= ~DRV_ADC_SEQ_DEVICE_MSK_1;
    }
  }

  if(ulSequencerMask & DRV_ADC_SEQ_DEVICE_MSK_2)
  {
    if(ptDriver->aptAdcSequencer[2] != NULL)
    {
      if(ptDriver->aptAdcSequencer[2]->tConfiguration.eDeviceID == DRV_ADC_SEQ_DEVICE_ID_ADC2)
      {
        if(ptDriver->aptAdcSequencer[2]->tBuffer.fEnable == true)
        {
          ptDriver->aptAdcSequencer[2]->tBuffer.ulPosition = 0;
        }
        if(ptDriver->aptAdcSequencer[2]->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
        {
          ptDriver->aptAdcSequencer[2]->ptDevice->madc_seq_irq_raw = 0x1FFul;
          ptDriver->aptAdcSequencer[2]->ptDevice->madc_seq_irq_mask_set = 0x1FFul;
        }
        else
        {
          ptDriver->aptAdcSequencer[2]->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
        }
      }
      else
      {
        ulSequencerMask &= ~DRV_ADC_SEQ_DEVICE_MSK_2;
      }
    }
    else
    {
      ulSequencerMask &= ~DRV_ADC_SEQ_DEVICE_MSK_2;
    }
  }

  if(ulSequencerMask & DRV_ADC_SEQ_DEVICE_MSK_3)
  {
    if(ptDriver->aptAdcSequencer[3] != NULL)
    {
      if(ptDriver->aptAdcSequencer[3]->tConfiguration.eDeviceID == DRV_ADC_SEQ_DEVICE_ID_ADC3)
      {
        if(ptDriver->aptAdcSequencer[3]->tBuffer.fEnable == true)
        {
          ptDriver->aptAdcSequencer[3]->tBuffer.ulPosition = 0;
        }
        if(ptDriver->aptAdcSequencer[3]->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
        {
          ptDriver->aptAdcSequencer[3]->ptDevice->madc_seq_irq_raw = 0x1FFul;
          ptDriver->aptAdcSequencer[3]->ptDevice->madc_seq_irq_mask_set = 0x1FFul;
        }
        else
        {
          ptDriver->aptAdcSequencer[3]->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
        }
      }
      else
      {
        ulSequencerMask &= ~DRV_ADC_SEQ_DEVICE_MSK_3;
      }
    }
    else
    {
      ulSequencerMask &= ~DRV_ADC_SEQ_DEVICE_MSK_3;
    }
  }

  /* start together */
  ptDriver->ptDevice->madc_start = ulSequencerMask;

  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * Stops the sequencers given by the mask.
 *
 * \public
 * \memberof DRV_ADC_HANDLE_T
 * \param[in] ptDriver Handle of ADC driver
 * \param[in] eSequencerMask Mask of sequencers to stop
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Stop(DRV_ADC_HANDLE_T * const ptDriver, DRV_ADC_SEQ_DEVICE_MSK_E eSequencerMask)
{
  uint32_t ulSequencerMask = (uint32_t)eSequencerMask;

  if(ptDriver == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptDriver);

  if((ulSequencerMask > DRV_ADC_SEQ_DEVICE_MSK_MAX) || (ulSequencerMask < DRV_ADC_SEQ_DEVICE_MSK_MIN))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  if(ulSequencerMask & DRV_ADC_SEQ_DEVICE_MSK_0)
  {
    ptDriver->aptAdcSequencer[0]->ptDevice->madc_seq_cmd &= ~madc_seq0_madc_seq_cmd_run_Msk;

    if(ptDriver->aptAdcSequencer[0]->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ptDriver->aptAdcSequencer[0]->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
    }
  }

  if(ulSequencerMask & DRV_ADC_SEQ_DEVICE_MSK_1)
  {
    ptDriver->aptAdcSequencer[1]->ptDevice->madc_seq_cmd &= ~madc_seq0_madc_seq_cmd_run_Msk;

    if(ptDriver->aptAdcSequencer[1]->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ptDriver->aptAdcSequencer[1]->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
    }
  }

  if(ulSequencerMask & DRV_ADC_SEQ_DEVICE_MSK_2)
  {
    ptDriver->aptAdcSequencer[2]->ptDevice->madc_seq_cmd &= ~madc_seq0_madc_seq_cmd_run_Msk;

    if(ptDriver->aptAdcSequencer[2]->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ptDriver->aptAdcSequencer[2]->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
    }
  }

  if(ulSequencerMask & DRV_ADC_SEQ_DEVICE_MSK_3)
  {
    ptDriver->aptAdcSequencer[3]->ptDevice->madc_seq_cmd &= ~madc_seq0_madc_seq_cmd_run_Msk;

    if(ptDriver->aptAdcSequencer[3]->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ptDriver->aptAdcSequencer[3]->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
    }
  }

  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * Initializes the adc sequencer and its handle by the given configuration..
 *
 * In case of an error during initialization, the function returns an error value, but the lock for the handle, will not be released.
 * Together with the return value, this will ensure that the driver can only be interacted with if it has been correctly initialized.
 * Since the lock is initialized when this function is called, this function can still be called again with other parameters after a failed initialization.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 *
 * \param[in] ptSequencer Handle of the ADC sequencer
 * \param[in] ptDriver Handle of ADC driver
 * \return  DRV_OK
 */
DRV_STATUS_E DRV_ADC_Seq_Init(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRet = DRV_OK;
  uint32_t ulIndexLoop;

  if((ptDriver == NULL) || (ptSequencer == NULL))
  {
    return DRV_ERROR_PARAM;
  }
  ptSequencer->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptSequencer);

  if((ptSequencer->tConfiguration.eDeviceID > DRV_ADC_SEQ_DEVICE_ID_MAX) || (ptSequencer->tConfiguration.eDeviceID < DRV_ADC_SEQ_DEVICE_ID_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  else
  {
    ptSequencer->ptDevice = s_apSubDeviceAddressTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_ADC_SEQ_DEVICE_ID_MIN];
    s_apHandleAddressTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_ADC_SEQ_DEVICE_ID_MIN] = ptSequencer;
    ptDriver->aptAdcSequencer[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_ADC_SEQ_DEVICE_ID_MIN] = ptSequencer;
    ptSequencer->ptParent = ptDriver;
  }
  if((ptSequencer->tConfiguration.eOperationMode > DRV_OPERATION_MODE_MAX) || (ptSequencer->tConfiguration.eOperationMode < DRV_OPERATION_MODE_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptSequencer->tConfiguration.eDmaModeDisable > DRV_ADC_SEQ_DMA_MODE_MAX)
    || (ptSequencer->tConfiguration.eDmaModeDisable < DRV_ADC_SEQ_DMA_MODE_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptSequencer->tConfiguration.e32BitMode > DRV_ADC_SEQ_32Bit_MODE_MAX) || (ptSequencer->tConfiguration.e32BitMode < DRV_ADC_SEQ_32Bit_MODE_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptSequencer->tConfiguration.eVrefVdd3 > DRV_ADC_SEQ_VREF_VDD3_MAX) || (ptSequencer->tConfiguration.eVrefVdd3 < DRV_ADC_SEQ_VREF_VDD3_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptSequencer->tConfiguration.eClkSync > DRV_ADC_SEQ_CLK_SYNC_MAX) || (ptSequencer->tConfiguration.eClkSync < DRV_ADC_SEQ_CLK_SYNC_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptSequencer->tConfiguration.eClkPhase > DRV_ADC_SEQ_CLK_PHASE_MAX) || (ptSequencer->tConfiguration.eClkPhase < DRV_ADC_SEQ_CLK_PHASE_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  if((ptSequencer->tConfiguration.eClkPeriod > DRV_ADC_SEQ_CLK_PERIOD_MAX) || (ptSequencer->tConfiguration.eClkPeriod < DRV_ADC_SEQ_CLK_PERIOD_MIN))
  {
    return DRV_ERROR_PARAM;
  }
  for(ulIndexLoop = 0; ulIndexLoop < DRV_ADC_INPUTS_MAX; ulIndexLoop++)
  {
    if((ptSequencer->tConfiguration.eChannelTrackingTime[ulIndexLoop] > DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_MAX)
      || (ptSequencer->tConfiguration.eChannelTrackingTime[ulIndexLoop] < DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_MIN))
    {
      return DRV_ERROR_PARAM;
    }
  }
  if((ptSequencer->tConfiguration.pvBaseAdr > DRV_ADC_SEQ_BASE_ADDRESS_MAX) || (ptSequencer->tConfiguration.pvBaseAdr < DRV_ADC_SEQ_BASE_ADDRESS_MIN)) //lint !e413
  {
    return DRV_ERROR_PARAM;
  }
  for(ulIndexLoop = 0; ulIndexLoop < DRV_ADC_MEASUREMENTS_MAX; ulIndexLoop++) /* NOTE: DRV_ADC_MEASUREMENTS_MAX == 8 */
  {
    if((ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eEnable > DRV_ADC_SEQ_MEAS_ENABLE_MAX)
      || (ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eEnable < DRV_ADC_SEQ_MEAS_ENABLE_MIN))
    {
      return DRV_ERROR_PARAM;
    }
    if((ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eAdrOffset > DRV_ADC_SEQ_BASE_ADR_OFFSET_MAX)
      || (ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eAdrOffset < DRV_ADC_SEQ_BASE_ADR_OFFSET_MIN))
    {
      return DRV_ERROR_PARAM;
    }
    if((ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eInputChannel > DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_MAX)
      || (ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eInputChannel < DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_MIN))
    {
      return DRV_ERROR_PARAM;
    }
    if((ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eOversample > DRV_ADC_SEQ_MEAS_OVERSAMPLING_MAX)
      || (ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eOversample < DRV_ADC_SEQ_MEAS_OVERSAMPLING_MIN))
    {
      return DRV_ERROR_PARAM;
    }
    if((ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eTrigger > DRV_ADC_SEQ_MEAS_TRIGGER_MAX)
      || (ptSequencer->tConfiguration.tMeasurement[ulIndexLoop].eTrigger < DRV_ADC_SEQ_MEAS_TRIGGER_MIN))
    {
      return DRV_ERROR_PARAM;
    }
  }

  /* madc_seq_cfg */
  if(ptSequencer->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL) /* Pooling mode, DMA is always disable */
  {
    ptSequencer->tConfiguration.eDmaModeDisable = DRV_ADC_SEQ_DMA_MODE_DISABLED;
  }
  else if(ptSequencer->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA) /* DMA mode, DMA is always enable */
  {
    ptSequencer->tConfiguration.eDmaModeDisable = DRV_ADC_SEQ_DMA_MODE_ENABLED;
  }

  ptSequencer->ptDevice->madc_seq_cfg_b.dma_disable = ptSequencer->tConfiguration.eDmaModeDisable;
  ptSequencer->ptDevice->madc_seq_cfg_b.dma_32bit_adr = ptSequencer->tConfiguration.e32BitMode;
  ptSequencer->ptDevice->madc_seq_cfg_b.vref_vdd3 = ptSequencer->tConfiguration.eVrefVdd3;
  ptSequencer->ptDevice->madc_seq_cfg_b.adcclk_sync = ptSequencer->tConfiguration.eClkSync;
  ptSequencer->ptDevice->madc_seq_cfg_b.adcclk_phase = ptSequencer->tConfiguration.eClkPhase;
  ptSequencer->ptDevice->madc_seq_cfg_b.adcclk_period = ptSequencer->tConfiguration.eClkPeriod;

  /* madc_seq_ms_en */
  ptSequencer->ptDevice->madc_seq_ms_en_b.m7 = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_7].eEnable;
  ptSequencer->ptDevice->madc_seq_ms_en_b.m6 = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_6].eEnable;
  ptSequencer->ptDevice->madc_seq_ms_en_b.m5 = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_5].eEnable;
  ptSequencer->ptDevice->madc_seq_ms_en_b.m4 = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_4].eEnable;
  ptSequencer->ptDevice->madc_seq_ms_en_b.m3 = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_3].eEnable;
  ptSequencer->ptDevice->madc_seq_ms_en_b.m2 = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_2].eEnable;
  ptSequencer->ptDevice->madc_seq_ms_en_b.m1 = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_1].eEnable;
  ptSequencer->ptDevice->madc_seq_ms_en_b.m0 = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_0].eEnable;

  /* madc_seq_m0 */
  ptSequencer->ptDevice->madc_seq_m0_b.adr_offset = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_0].eAdrOffset;
  ptSequencer->ptDevice->madc_seq_m0_b.mux = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_0].eInputChannel;
  ptSequencer->ptDevice->madc_seq_m0_b.oversample = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_0].eOversample;
  ptSequencer->ptDevice->madc_seq_m0_b.trigger = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_0].eTrigger;

  /* madc_seq_m1 */
  ptSequencer->ptDevice->madc_seq_m1_b.adr_offset = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_1].eAdrOffset;
  ptSequencer->ptDevice->madc_seq_m1_b.mux = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_1].eInputChannel;
  ptSequencer->ptDevice->madc_seq_m1_b.oversample = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_1].eOversample;
  ptSequencer->ptDevice->madc_seq_m1_b.trigger = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_1].eTrigger;

  /* madc_seq_m2 */
  ptSequencer->ptDevice->madc_seq_m2_b.adr_offset = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_2].eAdrOffset;
  ptSequencer->ptDevice->madc_seq_m2_b.mux = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_2].eInputChannel;
  ptSequencer->ptDevice->madc_seq_m2_b.oversample = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_2].eOversample;
  ptSequencer->ptDevice->madc_seq_m2_b.trigger = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_2].eTrigger;

  /* madc_seq_m3 */
  ptSequencer->ptDevice->madc_seq_m3_b.adr_offset = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_3].eAdrOffset;
  ptSequencer->ptDevice->madc_seq_m3_b.mux = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_3].eInputChannel;
  ptSequencer->ptDevice->madc_seq_m3_b.oversample = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_3].eOversample;
  ptSequencer->ptDevice->madc_seq_m3_b.trigger = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_3].eTrigger;

  /* madc_seq_m4 */
  ptSequencer->ptDevice->madc_seq_m4_b.adr_offset = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_4].eAdrOffset;
  ptSequencer->ptDevice->madc_seq_m4_b.mux = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_4].eInputChannel;
  ptSequencer->ptDevice->madc_seq_m4_b.oversample = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_4].eOversample;
  ptSequencer->ptDevice->madc_seq_m4_b.trigger = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_4].eTrigger;

  /* madc_seq_m5 */
  ptSequencer->ptDevice->madc_seq_m5_b.adr_offset = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_5].eAdrOffset;
  ptSequencer->ptDevice->madc_seq_m5_b.mux = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_5].eInputChannel;
  ptSequencer->ptDevice->madc_seq_m5_b.oversample = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_5].eOversample;
  ptSequencer->ptDevice->madc_seq_m5_b.trigger = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_5].eTrigger;

  /* madc_seq_m6 */
  ptSequencer->ptDevice->madc_seq_m6_b.adr_offset = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_6].eAdrOffset;
  ptSequencer->ptDevice->madc_seq_m6_b.mux = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_6].eInputChannel;
  ptSequencer->ptDevice->madc_seq_m6_b.oversample = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_6].eOversample;
  ptSequencer->ptDevice->madc_seq_m6_b.trigger = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_6].eTrigger;

  /* madc_seq_m7 */
  ptSequencer->ptDevice->madc_seq_m7_b.adr_offset = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_7].eAdrOffset;
  ptSequencer->ptDevice->madc_seq_m7_b.mux = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_7].eInputChannel;
  ptSequencer->ptDevice->madc_seq_m7_b.oversample = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_7].eOversample;
  ptSequencer->ptDevice->madc_seq_m7_b.trigger = ptSequencer->tConfiguration.tMeasurement[DRV_ADC_SEQ_MEASUREMENT_7].eTrigger;

  /* madc_seq_tracking_time_muxx */
  ptSequencer->ptDevice->madc_seq_tracking_time_mux0 = (uint32_t) ptSequencer->tConfiguration.eChannelTrackingTime[DRV_ADC_SEQ_MEASUREMENT_0];
  ptSequencer->ptDevice->madc_seq_tracking_time_mux1 = (uint32_t) ptSequencer->tConfiguration.eChannelTrackingTime[DRV_ADC_SEQ_MEASUREMENT_1];
  ptSequencer->ptDevice->madc_seq_tracking_time_mux2 = (uint32_t) ptSequencer->tConfiguration.eChannelTrackingTime[DRV_ADC_SEQ_MEASUREMENT_2];
  ptSequencer->ptDevice->madc_seq_tracking_time_mux3 = (uint32_t) ptSequencer->tConfiguration.eChannelTrackingTime[DRV_ADC_SEQ_MEASUREMENT_3];
  ptSequencer->ptDevice->madc_seq_tracking_time_mux4 = (uint32_t) ptSequencer->tConfiguration.eChannelTrackingTime[DRV_ADC_SEQ_MEASUREMENT_4];
  ptSequencer->ptDevice->madc_seq_tracking_time_mux5 = (uint32_t) ptSequencer->tConfiguration.eChannelTrackingTime[DRV_ADC_SEQ_MEASUREMENT_5];
  ptSequencer->ptDevice->madc_seq_tracking_time_mux6 = (uint32_t) ptSequencer->tConfiguration.eChannelTrackingTime[DRV_ADC_SEQ_MEASUREMENT_6];
  ptSequencer->ptDevice->madc_seq_tracking_time_mux7 = (uint32_t) ptSequencer->tConfiguration.eChannelTrackingTime[DRV_ADC_SEQ_MEASUREMENT_7];

  /* madc_seq_ms_baseadr */
  ptSequencer->ptDevice->madc_seq_ms_baseadr = (uint32_t) ptSequencer->tConfiguration.pvBaseAdr;

  if(ptSequencer->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_ADC_SEQ_DEVICE_ID_MIN]);
    DRV_NVIC_EnableIRQ(s_apHandleIRQnTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_ADC_SEQ_DEVICE_ID_MIN]);

    ptSequencer->ptDevice->madc_seq_irq_raw = 0x1FFul;
    ptSequencer->ptDevice->madc_seq_irq_mask_set = 0x1FFul;
  }

  DRV_UNLOCK(ptSequencer);
  return eRet;
}

/*!
 * Deinitializes the adc sequencer device and handle.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_DeInit(DRV_ADC_SEQ_HANDLE_T * const ptSequencer)
{
  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }
  DRV_LOCK(ptSequencer);
  if(ptSequencer->ptParent->aptAdcSequencer[ptSequencer->tConfiguration.eDeviceID - (unsigned int) DRV_ADC_SEQ_DEVICE_ID_MIN] != ptSequencer)
  {
    return DRV_ERROR;
  }
  else
  {
    ptSequencer->ptParent->aptAdcSequencer[ptSequencer->tConfiguration.eDeviceID - (unsigned int) DRV_ADC_SEQ_DEVICE_ID_MIN] = NULL;
  }
  if(ptSequencer->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_ADC_SEQ_DEVICE_ID_MIN]);
    ptSequencer->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
    DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptSequencer->tConfiguration.eDeviceID - (uint32_t) DRV_ADC_SEQ_DEVICE_ID_MIN]);
    ptSequencer->ptDevice->madc_seq_irq_raw = 0x1FFul;
  }
  *ptSequencer = (DRV_ADC_SEQ_HANDLE_T ) { 0 };
  return DRV_OK;
}

/*!
 * set mode of the sequencer.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \param[in] eMode Sequencer mode
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_SetMode(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_SEQ_CONTINUOUS_MODE_E eMode)
{
  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptSequencer);

  if((eMode > DRV_ADC_SEQ_CONTINUOUS_MODE_MAX) || (eMode < DRV_ADC_SEQ_CONTINUOUS_MODE_MIN))
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_ERROR_PARAM;
  }

  ptSequencer->tConfiguration.eContinuousMode = eMode;

  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 * Starts the given adc sequencer.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_Start(DRV_ADC_SEQ_HANDLE_T * const ptSequencer)
{
  uint32_t ulSequencerCmdRegValue = 0;

  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  if(ptSequencer->ptDevice == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptSequencer);

  if(ptSequencer->tBuffer.fEnable)
  {
    ptSequencer->tBuffer.ulPosition = 0ul;
  }

  if(ptSequencer->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    ptSequencer->ptDevice->madc_seq_irq_raw = 0x1FFul;
    ptSequencer->ptDevice->madc_seq_irq_mask_set = 0x1FFul;
  }
  else
  {
    ptSequencer->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
  }

  ulSequencerCmdRegValue = ptSequencer->tConfiguration.eContinuousMode;
  ulSequencerCmdRegValue = ulSequencerCmdRegValue << madc_seq0_madc_seq_cmd_continuous_Pos;
  ulSequencerCmdRegValue |= DRV_ADC_SEQ_RUN;
  ptSequencer->ptDevice->madc_seq_cmd = ulSequencerCmdRegValue;

  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 * Stops the given adc sequencer.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_Stop(DRV_ADC_SEQ_HANDLE_T * const ptSequencer)
{
  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptSequencer);

  ptSequencer->ptDevice->madc_seq_cmd_b.run = DRV_ADC_SEQ_STOP;

  if(ptSequencer->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    ptSequencer->ptDevice->madc_seq_irq_mask_reset = 0x1FFul;
  }

  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 * Reset the given adc sequencer.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_Reset(DRV_ADC_SEQ_HANDLE_T * const ptSequencer)
{
  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptSequencer);

  ptSequencer->tBuffer.fEnable = false;
  ptSequencer->ptDevice->madc_seq_cmd_b.reset = 1u;

  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 * Changes the parameters of the given adc sequencer measurement.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \param[in] eMeasurement Measurement
 * \param[in] eInputChannel Input channel multiplexer setting
 * \param[in] eOversampling Number of samples minus one to sum for this measurement
 * \param[in] eTrigger Trigger condition for measurement
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_Meas_ChangeConfig(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_SEQ_MEASUREMENT_E eMeasurement,
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_E eInputChannel, DRV_ADC_SEQ_MEAS_OVERSAMPLING_E eOversampling, DRV_ADC_SEQ_MEAS_TRIGGER_E eTrigger)
{
  uint32_t ulMadc_Seq_Mx = 0ul;

  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptSequencer);

  if((eMeasurement > DRV_ADC_SEQ_MEASUREMENT_MAX) || (eMeasurement < DRV_ADC_SEQ_MEASUREMENT_MIN))
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_ERROR_PARAM;
  }

  if((eInputChannel > DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_MAX) || (eInputChannel < DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_MIN))
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_ERROR_PARAM;
  }

  if((eOversampling > DRV_ADC_SEQ_MEAS_OVERSAMPLING_MAX) || (eOversampling < DRV_ADC_SEQ_MEAS_OVERSAMPLING_MIN))
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_ERROR_PARAM;
  }

  if((eTrigger > DRV_ADC_SEQ_MEAS_TRIGGER_MAX) || (eTrigger < DRV_ADC_SEQ_MEAS_TRIGGER_MIN))
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_ERROR_PARAM;
  }

  ulMadc_Seq_Mx = ((eInputChannel << madc_seq0_madc_seq_m0_mux_Pos) & madc_seq0_madc_seq_m0_mux_Msk)
    | ((eOversampling << madc_seq0_madc_seq_m0_oversample_Pos) & madc_seq0_madc_seq_m0_oversample_Msk)
    | ((eTrigger << madc_seq0_madc_seq_m0_trigger_Pos) & madc_seq0_madc_seq_m0_trigger_Msk);

  switch (eMeasurement)
  {
  case DRV_ADC_SEQ_MEASUREMENT_0:
    ulMadc_Seq_Mx |= (ptSequencer->ptDevice->madc_seq_m0 & madc_seq0_madc_seq_m0_adr_offset_Msk);
    ptSequencer->ptDevice->madc_seq_m0 = ulMadc_Seq_Mx;
    break;
  case DRV_ADC_SEQ_MEASUREMENT_1:
    ulMadc_Seq_Mx |= (ptSequencer->ptDevice->madc_seq_m1 & madc_seq0_madc_seq_m1_adr_offset_Msk);
    ptSequencer->ptDevice->madc_seq_m1 = ulMadc_Seq_Mx;
    break;
  case DRV_ADC_SEQ_MEASUREMENT_2:
    ulMadc_Seq_Mx |= (ptSequencer->ptDevice->madc_seq_m2 & madc_seq0_madc_seq_m2_adr_offset_Msk);
    ptSequencer->ptDevice->madc_seq_m2 = ulMadc_Seq_Mx;
    break;
  case DRV_ADC_SEQ_MEASUREMENT_3:
    ulMadc_Seq_Mx |= (ptSequencer->ptDevice->madc_seq_m3 & madc_seq0_madc_seq_m3_adr_offset_Msk);
    ptSequencer->ptDevice->madc_seq_m3 = ulMadc_Seq_Mx;
    break;
  case DRV_ADC_SEQ_MEASUREMENT_4:
    ulMadc_Seq_Mx |= (ptSequencer->ptDevice->madc_seq_m4 & madc_seq0_madc_seq_m4_adr_offset_Msk);
    ptSequencer->ptDevice->madc_seq_m4 = ulMadc_Seq_Mx;
    break;
  case DRV_ADC_SEQ_MEASUREMENT_5:
    ulMadc_Seq_Mx |= (ptSequencer->ptDevice->madc_seq_m5 & madc_seq0_madc_seq_m5_adr_offset_Msk);
    ptSequencer->ptDevice->madc_seq_m5 = ulMadc_Seq_Mx;
    break;
  case DRV_ADC_SEQ_MEASUREMENT_6:
    ulMadc_Seq_Mx |= (ptSequencer->ptDevice->madc_seq_m6 & madc_seq0_madc_seq_m6_adr_offset_Msk);
    ptSequencer->ptDevice->madc_seq_m6 = ulMadc_Seq_Mx;
    break;
  case DRV_ADC_SEQ_MEASUREMENT_7:
    ulMadc_Seq_Mx |= (ptSequencer->ptDevice->madc_seq_m7 & madc_seq0_madc_seq_m7_adr_offset_Msk);
    ptSequencer->ptDevice->madc_seq_m7 = ulMadc_Seq_Mx;
    break;
  default:
    DRV_UNLOCK(ptSequencer);
    return DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 * Aquires the state of the given adc sequencer.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \param[out] ptState State of the ADC sequencer
 * \return  DRV_OK
 *          DRV_BUSY
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_GetState(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_STATE_T * const ptState)
{
  uint32_t Madc_Seq_Status = 0ul;

  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  /* important: read full sequencer status with one access */
  Madc_Seq_Status = ptSequencer->ptDevice->madc_seq_status;

  ptState->ulHalfClockCycle = (uint8_t) ((Madc_Seq_Status & madc_seq1_madc_seq_status_adc_half_clock_cycle_Msk)
    >> madc_seq1_madc_seq_status_adc_half_clock_cycle_Pos);
  ptState->eActiveMeasurement = (DRV_ADC_STATE_MEASUREMENT_E) ((Madc_Seq_Status & madc_seq1_madc_seq_status_m_nr_Msk)
    >> madc_seq1_madc_seq_status_m_nr_Pos);

  if(ptState->ulHalfClockCycle == 0)
  {
    return DRV_OK;
  }
  return DRV_BUSY;
}

/*!
 * Get the acquired sample of the  the given adc.
 * It may be that the retrieved value is not the final result, but only an intermediate value.
 * In this case the return value is DRV_BUSY instead of DRV_OK.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \param[out] pulSample Value of the current sample
 * \param[out] pbMeasNr Number of measurement
 * \return  DRV_OK
 *          DRV_BUSY
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_GetSample(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, uint32_t* const pulSample, uint8_t* const pbMeasNr)
{
  uint32_t Madc_Seq_Result_Current = 0ul;

  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptSequencer);

  /* important: read full sequencer result with one access */
  Madc_Seq_Result_Current = ptSequencer->ptDevice->madc_seq_result_current;

  if(pulSample != NULL)
  {
    *pulSample = (Madc_Seq_Result_Current & madc_seq1_madc_seq_result_current_val_Msk) >> madc_seq1_madc_seq_result_current_val_Pos;
  }

  if(pbMeasNr != NULL)
  {
    *pbMeasNr = (Madc_Seq_Result_Current & madc_seq1_madc_seq_result_current_mnr_Msk) >> madc_seq1_madc_seq_result_current_mnr_Pos;
  }

  if(Madc_Seq_Result_Current & madc_seq1_madc_seq_result_current_valid_Msk)
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_OK;
  }
  else
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_BUSY;
  }
}

/*!
 * Get the last acquired sample of the  the given adc.
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \param[out] pulSample Value of the last sample
 * \param[out] pbMeasNr Number of measurement
 * \return  DRV_OK
 *          DRV_BUSY
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_ADC_Seq_GetLastSample(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, uint32_t* const pulSample, uint8_t* const pbMeasNr)
{
  uint32_t Madc_Seq_Result_Last = 0ul;

  if(ptSequencer == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  DRV_LOCK(ptSequencer);

  /* important: read full sequencer last result with one access */
  Madc_Seq_Result_Last = ptSequencer->ptDevice->madc_seq_result_last;

  if(pulSample != NULL)
  {
    *pulSample = (Madc_Seq_Result_Last & madc_seq1_madc_seq_result_last_val_Msk) >> madc_seq1_madc_seq_result_last_val_Pos;
  }

  if(pbMeasNr != NULL)
  {
    *pbMeasNr = (Madc_Seq_Result_Last & madc_seq1_madc_seq_result_last_mnr_Msk) >> madc_seq1_madc_seq_result_last_mnr_Pos;
  }

  if(Madc_Seq_Result_Last & madc_seq1_madc_seq_result_last_valid_Msk)
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_OK;
  }
  else
  {
    DRV_UNLOCK(ptSequencer);
    return DRV_BUSY;
  }
}

/*!
 * Set a buffer memory to record a series of samples
 * This function do automatically enable the buffer and after next start of the sequencer, every recored sample will
 * stored continuously into the buffer area. The buffer works like a ring buffer.
 * After every round of filling the buffer, it is possible to execute a callback.
 * Reseting the sequencer will deactivate the buffer. The buffer only works together with the  interrupt mode.
 * The buffer memory is word based. (2bytes need for every sample)
 *
 * \public
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer Handle of ADC sequencer
 * \param[out] pulSampleSeries Pointer to the sample buffer
 * \param[in] tSize Size of buffer in byte
 * \param[in] fnCallback the users callback function
 * \param[in] pUser pointer to user resources to pass through
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 *          DRV_NSUPP
 */
DRV_STATUS_E DRV_ADC_Seq_SetSampleSeriesBuffer(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, uint16_t* pulSampleSeries, size_t tSize,
  DRV_CALLBACK_F fnCallback, void* pUser)
{
  if((ptSequencer == NULL) || (pulSampleSeries == NULL) || (tSize < 2))
  {
    return DRV_ERROR_PARAM;
  }

  if(ptSequencer->tConfiguration.eOperationMode != DRV_OPERATION_MODE_IRQ)
  {
    return DRV_NSUPP; /* buffer functions only supported in interrupt mode */
  }
  DRV_LOCK(ptSequencer);

  ptSequencer->tBuffer.fEnable = true;
  ptSequencer->tBuffer.pvBufferAdr = (void*) pulSampleSeries;
  ptSequencer->tBuffer.ulPosition = 0;
  ptSequencer->tBuffer.ulMaxCount = (uint32_t) (tSize / 2);
  ptSequencer->tBuffer.fnWrapCallback = fnCallback;
  ptSequencer->tBuffer.pWrapCallbackkHandle = pUser;

  DRV_UNLOCK(ptSequencer);
  return DRV_OK;
}

/*!
 * In-line function for save the result of a single measurement and if necessary, execute the corresponding callback.
 *
 * \private
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in] ptSequencer ADC sequencer
 * \param[in] eMeasurement The number of completed measurement
 * \return void
 */
__STATIC_INLINE void DRV_ACD_IRQ_SaveResult(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_SEQ_MEASUREMENT_E const eMeasurement)
{
  uint32_t ulRawValue;
  uint16_t* pusSample;

  /* If buffer mode active, try to save the result */
  if(ptSequencer->tBuffer.fEnable)
  {
    ulRawValue = ptSequencer->ptDevice->madc_seq_result_current;
    pusSample = (uint16_t*) ptSequencer->tBuffer.pvBufferAdr + ptSequencer->tBuffer.ulPosition;

    /* Check if the result valid and assigned to this measurement.
     * Only the case if this measurement be the last in the sequence or the next is not be triggered yet */
    if(ulRawValue & (0x80000000 + (eMeasurement << madc_seq0_madc_seq_result_current_mnr_Pos)))
    {
      *pusSample = (uint16_t) ulRawValue & madc_seq0_madc_seq_result_current_val_Msk;
    }
    else
    {
      /* Get the last latched result */
      ulRawValue = ptSequencer->ptDevice->madc_seq_result_last;

      /* Check if the last result valid and assigned to this measurement. */
      if(ulRawValue & (0x80000000 + (eMeasurement << madc_seq0_madc_seq_result_last_mnr_Pos)))
      {
        *pusSample = (uint16_t) ptSequencer->ptDevice->madc_seq_result_last & madc_seq0_madc_seq_result_last_val_Msk;
      }
      else
      {
        /* Possible to late and the correct result is overwritten. Possible solution is to activate the DMA mode */
        *pusSample = DRV_ADC_SEQ_CATCHED_NO_RESULT;
      }
    }
    ptSequencer->tBuffer.ulPosition++;
    if(ptSequencer->tBuffer.ulPosition == ptSequencer->tBuffer.ulMaxCount)
    {
      /* Last value to the buffer in this round, chance to execute a callback */
      if(ptSequencer->tBuffer.fnWrapCallback != 0)
      {
        ptSequencer->tBuffer.fnWrapCallback((void*) ptSequencer, ptSequencer->tBuffer.pWrapCallbackkHandle);
      }
      ptSequencer->tBuffer.ulPosition = 0; /* next round for the ring buffer */
    }
  }
  if(ptSequencer->tConfiguration.fnMeasurementCompleteCallback[eMeasurement] != 0)
  {
    ptSequencer->tConfiguration.fnMeasurementCompleteCallback[eMeasurement]((void*) ptSequencer,
      ptSequencer->tConfiguration.pMeasurementCompleteCallbackHandle[eMeasurement]);
  }
}

/*!
 * This function is the true interrupt service routine or interrupt handler of the ADC device interrupts. The interrupt handlers called by the
 * controller should inline this handler and the constant eDeviceID should define the specific device in compile time. For debug and development
 * purposes a check of the pointers is performed in front that is not necessary for release. It is necessary because a debugger stopped controller
 * does not perform a reset of the interrupt vector and so the ISR shall cope with the not initialized handle pointer.
 *
 * \private
 * \memberof DRV_ADC_SEQ_HANDLE_T
 * \param[in,out] eDeviceID The given id of the drivers class
 * \return void
 */
__STATIC_INLINE void DRV_ACD_IRQ_Inline_Handler(DRV_ADC_SEQ_DEVICE_ID_E const eDeviceID)
{
  uint32_t ulDeviceOffset = (uint32_t) eDeviceID - (uint32_t) DRV_ADC_SEQ_DEVICE_ID_MIN;

  DRV_ADC_SEQ_HANDLE_T* ptSequencer = s_apHandleAddressTable[ulDeviceOffset];
#ifndef NDEBUG
  if(ptSequencer == 0 || ptSequencer->ptDevice != s_apSubDeviceAddressTable[ulDeviceOffset])
  {
    ((DRV_ADC_SEQ_DEVICE_T * const ) s_apSubDeviceAddressTable[ulDeviceOffset])->madc_seq_ms_en = 0ul;
    DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[ulDeviceOffset]);
    return;
  }
#endif

  uint32_t ulIrqMasked = ptSequencer->ptDevice->madc_seq_irq_mask_reset; /* shows bit number of the lowest active bit in IRQ_MASKED */

  switch (ulIrqMasked)
  { /* fall through conditions */
  case madc_seq0_madc_seq_irq_masked_m0_cmpl_Pos:
  case madc_seq0_madc_seq_irq_masked_m1_cmpl_Pos:
  case madc_seq0_madc_seq_irq_masked_m2_cmpl_Pos:
  case madc_seq0_madc_seq_irq_masked_m3_cmpl_Pos:
  case madc_seq0_madc_seq_irq_masked_m4_cmpl_Pos:
  case madc_seq0_madc_seq_irq_masked_m5_cmpl_Pos:
  case madc_seq0_madc_seq_irq_masked_m6_cmpl_Pos:
  case madc_seq0_madc_seq_irq_masked_m7_cmpl_Pos:
    (void) DRV_ACD_IRQ_SaveResult(ptSequencer, (DRV_ADC_SEQ_MEASUREMENT_E) ulIrqMasked);
    break;
  case madc_seq0_madc_seq_irq_masked_seq_cmpl_Pos:
    if(ptSequencer->tConfiguration.fnSequenceCompleteCallback != 0)
    {
      ptSequencer->tConfiguration.fnSequenceCompleteCallback((void*) ptSequencer, ptSequencer->tConfiguration.pSequenceCompleteCallbackHandle);
    }
    break;
  default:
    break;
  }
  /* Resets the appropriate IRQ */
  ptSequencer->ptDevice->madc_seq_irq_raw = 1ul << ulIrqMasked;
}

/*!
 * \brief Macro for automatic ADC-IRQ-Function generation.
 */
#define  DRV_ACD_IRQHandler_Generator(id, _) DRV_Default_IRQHandler_Function_Generator(MADC ## id ## _IRQHandler ,DRV_ACD_IRQ_Inline_Handler,DRV_ADC_SEQ_DEVICE_ID_ADC ## id)

/*lint -save -e123 The name of a macro defined with arguments was subsequently used without a following '(' */
DRV_DEF_REPEAT_EVAL(DRV_ADC_SEQ_DEVICE_COUNT, DRV_ACD_IRQHandler_Generator, ~)
/*lint -restore */

/*! \} *//* End of group ADC */

/*lint -restore */

#endif /* DRV_ADC_MODULE_DISABLE */
