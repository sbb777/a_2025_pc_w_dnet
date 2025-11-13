/*!************************************************************************//*!
 * \file     netx_drv_i2c.c
 * \brief    I2C peripheral module driver.
 * $Revision: 11617 $
 * $Date: 2024-10-07 15:35:35 +0300 (Mon, 07 Oct 2024) $
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
#include <stdbool.h>
#include "assert.h"
#ifdef DRV_I2C_MODULE_ENABLED

#define DRV_I2C_CLEAR_IRQSR_FLAGS               (0xFFFFFFFFul)
#define DRV_I2C_CLEAR_IRQSR_FLAGS_EXCEPT_CMD_OK (0xFFFFFFFEul)
#define DRV_I2C_ACK_POLL_MAX                    (255)
#define DRV_I2C_NWR_SAMPLING_ATTEMPTS           (3)

/* Allowed addresses according to Philips specification . */
#define DRV_I2C_LOWEST_ALLOWED_7BIT_ADDR        (0x08)
#define DRV_I2C_HIGHEST_ALLOWED_7BIT_ADDR       (0x77)

__STATIC_INLINE DRV_STATUS_E
drv_I2C_VerifyParams(DRV_I2C_HANDLE_T* const ptDriver,
                     DRV_I2C_ADDRESS_T* ptTargetAddress,
                     const uint8_t* const pbData,
                     uint32_t ulSize,
                     DRV_I2C_CONT_E eCont);

__STATIC_INLINE void
drv_I2C_LoadTxParams(DRV_I2C_HANDLE_T* const ptDriver,
                     DRV_I2C_ADDRESS_T* ptTargetAddress,
                     const uint8_t* const pbData,
                     uint32_t ulSize,
                     DRV_I2C_CONT_E eCont);

__STATIC_INLINE DRV_STATUS_E
drv_I2C_LoadRxParams(DRV_I2C_HANDLE_T* const ptDriver,
                     DRV_I2C_ADDRESS_T* ptTargetAddress,
                     uint8_t* const pbData,
                     uint32_t ulSize,
                     DRV_I2C_CONT_E eCont);

__STATIC_INLINE uint32_t
drv_I2C_ComposeTxCommand(DRV_I2C_HANDLE_T* const ptDriver,
                         uint32_t ulSize,
                         DRV_I2C_CONT_E eCont);

__STATIC_INLINE uint32_t
drv_I2C_ComposeRxCommand(DRV_I2C_HANDLE_T* const ptDriver,
                         uint32_t ulSize,
                         DRV_I2C_CONT_E eCont);

__STATIC_INLINE DRV_STATUS_E
drv_I2C_WaitTxStartSeqAcknowledged(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE DRV_STATUS_E
drv_I2C_WaitRxStartSeqAcknowledged(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE bool
drv_I2C_IsTxStartSeqAcknowledged(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE bool
drv_I2C_IsRxStartSeqAcknowledged(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE uint32_t
drv_I2C_SelectTxCommand(DRV_I2C_HANDLE_T* const ptDriver,
                        DRV_I2C_CONT_E eCont);

__STATIC_INLINE uint32_t
drv_I2C_SelectRxCommand(DRV_I2C_HANDLE_T* const ptDriver,
                        DRV_I2C_CONT_E eCont);

__STATIC_INLINE DRV_STATUS_E
drv_I2C_TransferData(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE DRV_STATUS_E
drv_I2C_WaitTransferEnd(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE DRV_STATUS_E
drv_I2C_WaitIdle(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE DRV_STATUS_E
drv_I2C_CheckForError(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_CallEndTransferCallback(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE bool
drv_I2C_IsTransferDirectionChanged(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE bool
drv_I2C_IsNwrMasterWrite(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE bool
drv_I2C_IsSlaveTransferPrematurelyStopped(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_HandleSlaveTransferDirectionChange(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_HandleSlaveTransferPrematureStop(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateUninitialized(DRV_I2C_HANDLE_T* const ptDriver,
                                      DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateXferError(DRV_I2C_HANDLE_T* const ptDriver,
                                  DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateIdle(DRV_I2C_HANDLE_T* const ptDriver,
                             DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateWaitAddressing(DRV_I2C_HANDLE_T* const ptDriver,
                                       DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateWaitStopOrRepeatedAddressing(DRV_I2C_HANDLE_T* const ptDriver,
                                                     DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateXferToBeReversed(DRV_I2C_HANDLE_T* const ptDriver,
                                         DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateAddrAckToBeEnabled(DRV_I2C_HANDLE_T* const ptDriver,
                                           DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateAddrAckEnabled(DRV_I2C_HANDLE_T* const ptDriver,
                                       DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateAddrAckAndSkipDirectionCheck(DRV_I2C_HANDLE_T* const ptDriver,
                                                     DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateXferStopped(DRV_I2C_HANDLE_T* const ptDriver,
                                    DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_UpdateSlaveState(DRV_I2C_HANDLE_T* const ptDriver,
                         DRV_I2C_SLAVE_STATE_E eRequestedState);

__STATIC_INLINE void
drv_I2C_FillMasterTxFifo(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_ReadMasterRxFifo(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_FillSlaveTxFifo(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_ReadSlaveRxFifo(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE bool
drv_I2c_IsTransferFinishedCorrectly(DRV_I2C_HANDLE_T* const ptDriver,
                                    uint32_t ulCmdOk);

__STATIC_INLINE void
drv_I2C_IRQ_HandleMasterTransferEnd(DRV_I2C_HANDLE_T* const ptDriver,
                                    uint32_t ulCmdOk);

__STATIC_INLINE void
drv_I2C_IRQ_CheckForCmdError(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_IRQ_CheckForFifoError(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_IRQ_HandleSlaveTransferEnd(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_IRQ_HandleSlaveBufferNotPresent(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_AdjustSlaveFifoWatermark(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_AdjustMasterFifoWatermark(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_ResetMstBuffer(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_ResetSlvBuffer(DRV_I2C_HANDLE_T* const ptDriver);

__STATIC_INLINE void
drv_I2C_GenerateConditionalStop(DRV_I2C_HANDLE_T* const ptDriver);

/*! \addtogroup I2C I2C
 * \{
 * \brief The I2C driver, defined by DRV_I2C_HANDLE_T
 *
 * \details
 */

/*!
 * \brief Table of the device addresses.
 *
 * Used to identify the device addresses by the device ID.
 */
static DRV_I2C_DEVICE_T* const s_apDeviceAddressTable[DRV_I2C_DEVICE_COUNT] = DRV_I2C_DEVICE_LIST;
/*!
 * \brief Table of the IRQ vector numbers.
 *
 * Used to identify the interrupt channels by the device ID.
 */
static IRQn_Type const s_apHandleIRQnTable[DRV_I2C_DEVICE_COUNT] = DRV_I2C_DEVICE_IRQ_LIST;
/*!
 * \brief Used for mapping the handle to an interrupt.
 *
 * Threadsafe and reentrant because its is only written in normal context an used in interrupt context of the specific interrupt.
 */
static DRV_I2C_HANDLE_T* s_apHandleAddressTable[DRV_I2C_DEVICE_COUNT] = { 0 };

/*!
 * Define for checking the consistency of the handle or static representation of the driver.
 */
#ifndef DRV_HANDLE_CHECK_INACTIVE
#define DRV_HANDLE_CHECK(handle)\
  if((handle)==0){ \
    return DRV_ERROR_PARAM; \
  }\
  if((handle)->ptDevice==0){ \
    return DRV_ERROR_PARAM; \
  }
#else
#define DRV_HANDLE_CHECK(handle)
#endif

/*!
 * This method takes an I2C context object of the type DRV_I2C_HANDLE_T and uses the configuration given within to initialize the
 * device. This driver context may be used further on in the I2C API functions to interact with the given I2C device. This context
 * object shall not be modified after initialization. It is only valid to modify it before initializing the device again. The
 * behavior between modification and the performed reinitialization is unknown.
 *
 * In case of an error during initialization, the function returns an error value, but the lock for the handle, will not be released.
 * Together with the return value, this will ensure that the driver can only be interacted with if it has been correctly initialized.
 * Since the lock is initialized when this function is called, this function can still be called again with other parameters after a failed initialization.
 *
 * Given by the operating mode, the interrupts will be modified and the DMA controllers will be initialized.
 *
 * \note The address of the own slave has to be specified even if the slave is not used actively.
 *
 * \public
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return DRV_OK Everything is fine.
 *         DRV_ERROR_PARAM A given parameter is not correct.
 */
DRV_STATUS_E DRV_I2C_Init(DRV_I2C_HANDLE_T* const ptDriver)
{
  if(ptDriver == 0)
  {
    return DRV_ERROR_PARAM;
  }

  ptDriver->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptDriver);

  if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
  {
    return DRV_NSUPP;
  }

  if(ptDriver->tConfiguration.eDeviceID >= DRV_I2C_DEVICE_ID_MIN && ptDriver->tConfiguration.eDeviceID <= DRV_I2C_DEVICE_ID_MAX)
  {
    ptDriver->ptDevice = s_apDeviceAddressTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN];
  }
  else
  {
    return DRV_ERROR_PARAM;
  }

  if(DRV_I2C_ACK_POLL_MAX < ptDriver->tConfiguration.sAckPollMaximum)
  {
    return DRV_ERROR_PARAM;
  }

  if(DRV_I2C_SPEED_MODE_MAX < ptDriver->tConfiguration.eSpeedMode)
  {
    return DRV_ERROR_PARAM;
  }

  if(DRV_I2C_ADDRESS_10_BIT == ptDriver->tConfiguration.tSlaveAddress.eAddressingScheme)
  {
    ptDriver->ptDevice->i2c_scr_b.sid10 = 1;
    ptDriver->ptDevice->i2c_scr_b.sid = ptDriver->tConfiguration.tSlaveAddress.uDeviceAddress;
  }
  else if(DRV_I2C_ADDRESS_7_BIT == ptDriver->tConfiguration.tSlaveAddress.eAddressingScheme)
  {
    if((DRV_I2C_LOWEST_ALLOWED_7BIT_ADDR > ptDriver->tConfiguration.tSlaveAddress.uDeviceAddress) ||
       (DRV_I2C_HIGHEST_ALLOWED_7BIT_ADDR < ptDriver->tConfiguration.tSlaveAddress.uDeviceAddress))
    {
      return DRV_ERROR_PARAM;
    }
    ptDriver->ptDevice->i2c_scr_b.sid10 = 0;
    ptDriver->ptDevice->i2c_scr_b.sid = ptDriver->tConfiguration.tSlaveAddress.uDeviceAddress;
  }
  else
  {
    return DRV_ERROR_PARAM;
  }

  if((uint32_t)DRV_I2C_TIMEOUT_DETECTION_MAX < (uint32_t)ptDriver->tConfiguration.eTimeoutDetection)
  {
    return DRV_ERROR_PARAM;
  }

  if(DRV_I2C_WATERMARK_UNINITIALIZED == ptDriver->tConfiguration.eMstRxFifoWatermark)
  {
    ptDriver->tConfiguration.eMstRxFifoWatermark = DRV_I2C_WATERMARK_DEFAULT;
  }
  if(DRV_I2C_WATERMARK_UNINITIALIZED == ptDriver->tConfiguration.eMstTxFifoWatermark)
  {
    ptDriver->tConfiguration.eMstTxFifoWatermark = DRV_I2C_WATERMARK_DEFAULT;
  }
  if(DRV_I2C_WATERMARK_UNINITIALIZED == ptDriver->tConfiguration.eMstTxFifoRefillLevel)
  {
    ptDriver->tConfiguration.eMstTxFifoRefillLevel = DRV_I2C_WATERMARK_MAX;
  }
  if(DRV_I2C_WATERMARK_UNINITIALIZED == ptDriver->tConfiguration.eSlvRxFifoWatermark)
  {
    ptDriver->tConfiguration.eSlvRxFifoWatermark = DRV_I2C_WATERMARK_DEFAULT;
  }
  if(DRV_I2C_WATERMARK_UNINITIALIZED == ptDriver->tConfiguration.eSlvTxFifoWatermark)
  {
    ptDriver->tConfiguration.eSlvTxFifoWatermark = DRV_I2C_WATERMARK_DEFAULT;
  }
  if(DRV_I2C_WATERMARK_UNINITIALIZED == ptDriver->tConfiguration.eSlvTxFifoRefillLevel)
  {
    ptDriver->tConfiguration.eSlvTxFifoRefillLevel = DRV_I2C_WATERMARK_MAX;
  }

  ptDriver->ptDevice->i2c_mcr = 0;
  ptDriver->ptDevice->i2c_mcr_b.en_i2c = 0;
  ptDriver->ptDevice->i2c_mcr_b.rst_i2c = 1;
  ptDriver->ptDevice->i2c_mcr_b.pio_mode = 0;
  ptDriver->ptDevice->i2c_mcr_b.sadr = 0;
  ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_clr = 1;
  ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_clr = 1;
  ptDriver->ptDevice->i2c_irqsr = 0xFFFFFFFF;
  ptDriver->ptDevice->i2c_scr_b.autoreset_ac_start = 1;
  ptDriver->ptDevice->i2c_scr_b.ac_start = 0;
  ptDriver->ptDevice->i2c_scr_b.ac_srx = 0;
  ptDriver->ptDevice->i2c_scr_b.ac_gcall = 0;

  s_apHandleAddressTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN] = ptDriver;

  if(DRV_I2C_TIMEOUT_DETECTION_DISABLED == ptDriver->tConfiguration.eTimeoutDetection)
  {
    ptDriver->ptDevice->i2c_mcr_b.en_timeout = 0;
  }
  else
  {
    ptDriver->ptDevice->i2c_mcr_b.en_timeout = 1;
  }

  ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;

  drv_I2C_ResetMstBuffer(ptDriver);

  ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_UNKNOWN;

  drv_I2C_ResetSlvBuffer(ptDriver);

  (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_IDLE);

  if(ptDriver->tConfiguration.ulDriverTimeout == 0)
  {
    ptDriver->tConfiguration.ulDriverTimeout = 0xFFFFFFFFul;
  }
  /* Configure NVIC (activate IRQ, define priority and so on) */
  ptDriver->ptDevice->i2c_mcr_b.mode = ptDriver->tConfiguration.eSpeedMode;
  ptDriver->ptDevice->i2c_mcr_b.rst_i2c = 0;
  ptDriver->ptDevice->i2c_mcr_b.en_i2c = 1;
  if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN]);
    DRV_NVIC_EnableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN]);
  }
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * It tries to get a lock on the handle so that it is able to be reset.
 * Then it will reset the NVIC, the device states and the context given.
 *
 * \public
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The handle of the driver
 * \return DRV_OK Everything is ok.
 *         DRV_LOCKED The API is locked and not accessible.
 *         DRV_ERROR_PARAM A given parameter is not correct.
 */
DRV_STATUS_E DRV_I2C_DeInit(DRV_I2C_HANDLE_T* const ptDriver)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN]);
  DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN]);
  ptDriver->ptDevice->i2c_irqsr = DRV_I2C_CLEAR_IRQSR_FLAGS;
  ptDriver->ptDevice->i2c_irqmsk = 0u;
  ptDriver->ptDevice->i2c_mcr_b.rst_i2c = 1u;
  ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_clr = 1u;
  ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_clr = 1u;
  ptDriver->ptDevice->i2c_mcr_b.en_i2c = 0u;
  *ptDriver = (DRV_I2C_HANDLE_T ) { 0 };
  return DRV_OK;
}

/*!
 * \brief The flush function is performing the task of equalizing the hardware and software buffers.
 *
 * The flush function checks if there is data to transmit or receive in the given FIFOs and
 * depending on the direction emptying the hardware buffer or filling it up. Because there is only one
 * interrupt source for master and slave, both buffers are serviced.
 * When the function is called from slave receive function, additional check is performed,
 * asserting read of entire slave FIFO contents.
 *
 * \private
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return void Because there is no margin for errors here.
 */
__INLINE static void DRV_I2C_Flush_Buffers(DRV_I2C_HANDLE_T* const ptDriver)
{
  /* Slave Event Detection */
  if(NULL != ptDriver->SlvBuffer)
  {
    /* When slave is addressed, check for direction change. */
    if((1u == ptDriver->ptDevice->i2c_sr_b.slave_access) &&
       (1u == ptDriver->ptDevice->i2c_irqsr_b.sreq)      &&
        ((DRV_I2C_SLAVE_WAIT_ADDRESSING == ptDriver->eSlaveState)            ||
         (DRV_I2C_SLAVE_WAIT_STOP_OR_REPEATED_ADDRESSING == ptDriver->eSlaveState)))
    {
      if(drv_I2C_IsTransferDirectionChanged(ptDriver))
      {
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_XFER_TO_BE_REVERSED);
      }
      else
      {
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_ADDR_ACK_TO_BE_ENABLED);
      }
    }

    if(DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode)
    {
      /* In polling mode, clear sreq irq flag, in order to check for direction change
      *  only once after slave addressing. Otherwise false-positive change detection events occur. */
      ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_sreq_Msk;

     /* 1. Check for Premature Stop gives false positive result, if made before addressing.
      * 2. If Direction change is detected, Premature Stop can not occur in the same cycle. */
      if((DRV_I2C_SLAVE_WAIT_STOP_OR_REPEATED_ADDRESSING == ptDriver->eSlaveState) ||
         (DRV_I2C_SLAVE_ADDR_ACK_ENABLED == ptDriver->eSlaveState))
      {
        if(drv_I2C_IsSlaveTransferPrematurelyStopped(ptDriver))
        {
          (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_XFER_STOPPED);
        }
      }
    }

    if((DRV_I2C_SLAVE_XFER_TO_BE_REVERSED == ptDriver->eSlaveState))
    {
      drv_I2C_HandleSlaveTransferDirectionChange(ptDriver);

      /* Switch to Slave Reverse Buffer */
      if(NULL != ptDriver->SlvRvrsBuffer)
      {
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_ADDR_ACK_TO_BE_ENABLED);
      }
      else
      {
        /* If there is no Reverse buffer, force the slave to end the transfer prematurely. */
        ptDriver->SlvBufferSize = ptDriver->SlvBufferCounter;
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_XFER_STOPPED);
      }
    }

    if(DRV_I2C_RW_FLAG_READ == ptDriver->eRwFlagSlave)
    {
      /* Slave receive */
      drv_I2C_ReadSlaveRxFifo(ptDriver);
    }
    else if(DRV_I2C_RW_FLAG_WRITE == ptDriver->eRwFlagSlave)
    {
      /* If slave FIFO level is above slave refill level, it is an error condition. */
      if(ptDriver->tConfiguration.eSlvTxFifoRefillLevel < ptDriver->ptDevice->i2c_sr_b.sfifo_level)
      {
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_XFER_ERROR);
      }
      else
      {
        /* Slave transmit */
        drv_I2C_FillSlaveTxFifo(ptDriver);
      }
    }

    /* If buffer counter is above buffer size, it is an error condition. */
    if(ptDriver->SlvBufferCounter > ptDriver->SlvBufferSize)
    {
      (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_XFER_ERROR);
    }

    if(DRV_I2C_SLAVE_XFER_STOPPED == ptDriver->eSlaveState)
    {
      /* Slave operation not complete */
      if(ptDriver->SlvBufferCounter < ptDriver->SlvBufferSize)
      {
        drv_I2C_HandleSlaveTransferPrematureStop(ptDriver);

        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_WAIT_ADDRESSING);
      }
      else
      {
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_IDLE);
      }
    }
    else
    {
      /* If address is acknowledged, enable check for stop. */
      if((DRV_I2C_SLAVE_ADDR_ACK_ENABLED == ptDriver->eSlaveState) &&
         (0 == ptDriver->ptDevice->i2c_irqsr_b.sreq)               &&
         (1 == ptDriver->ptDevice->i2c_sr_b.last_ac))
      {
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_WAIT_STOP_OR_REPEATED_ADDRESSING);
      }
    }

    /* Specific checks for IRQ mode */
    if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
    {
      /* In IRQ mode direction must be checked only on first addressing byte */
      if(DRV_I2C_SLAVE_ADDR_ACK_AND_SKIP_DIRECTION_CHECK == ptDriver->eSlaveState)
      {
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_WAIT_STOP_OR_REPEATED_ADDRESSING);
      }

      if(DRV_I2C_SLAVE_ADDR_ACK_ENABLED == ptDriver->eSlaveState)
      {
        (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_ADDR_ACK_AND_SKIP_DIRECTION_CHECK);
      }
    }

    /* Enable Slave Acknowledge of the addressing. */
    if(DRV_I2C_SLAVE_ADDR_ACK_TO_BE_ENABLED == ptDriver->eSlaveState)
    {
      ptDriver->ptDevice->i2c_scr_b.ac_start = 1;

      (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_ADDR_ACK_ENABLED);
    }
  }

  /* Master operation not complete */
  if(ptDriver->MstBufferCounter != ptDriver->MstBufferSize)
  {
    if(DRV_I2C_RW_FLAG_WRITE == ptDriver->eRwFlagMaster)
    {
      /* Master Transmit */
      drv_I2C_FillMasterTxFifo(ptDriver);
    }
    else if(DRV_I2C_RW_FLAG_READ == ptDriver->eRwFlagMaster)
    {
      /* Master Receive */
      drv_I2C_ReadMasterRxFifo(ptDriver);
    }
  }
}

/*!
 * \note The master code transmission function has to be used in front of every high speed arbitration context.
 *
 * Because of the I2C protocol, after a start in high speed mode, the master code has to be transmitted in
 * standard mode or fast mode. This function performs this master code transmission and switches the high speed
 * mode on. The driver is afterwards in the continuous mode and a transmission must be performed.
 *
 * \public
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] tTargetAddress as described in \ref i2c_addressing_scheme
 * \param[in] eSpeed The low or standard speed mode in which the master code will be transmitted
 * \return DRV_OK Everything is fine.
 *         DRV_LOCKED The API is locked and not accessible.
 *         DRV_ERROR_PARAM A given parameter is not correct.
 */
DRV_STATUS_E DRV_I2C_HS_Master_Code(DRV_I2C_HANDLE_T* const ptDriver,
                                    DRV_I2C_ADDRESS_T tTargetAddress,
                                    DRV_I2C_SPEED_MODE_E eSpeed)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_I2C_ADDRESS_U uTargetAddress;
  uTargetAddress.bf = tTargetAddress;

  if(DRV_I2C_ADDRESS_MASTER_CODE != tTargetAddress.eAddressingScheme)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  if((DRV_I2C_SPEED_MODE_HS_800k > ptDriver->tConfiguration.eSpeedMode) ||
     (DRV_I2C_SPEED_MODE_FS_FAST < eSpeed))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  DRV_STATUS_E ret = DRV_ERROR;
  if((NULL != ptDriver->MstBuffer)      ||
     (0 != ptDriver->MstBufferSize)     ||
     (0 != ptDriver->MstBufferCounter))
  {
    ret = DRV_BUSY;
  }
  else
  {
    if((DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode) ||
       (DRV_OPERATION_MODE_IRQ  == ptDriver->tConfiguration.eOperationMode) ||
       (DRV_OPERATION_MODE_DMA  == ptDriver->tConfiguration.eOperationMode))
    {
      ptDriver->eCT = DRV_I2C_CONT_CONTINUOUS;
      uint32_t tCMD = 0;
      ptDriver->ptDevice->i2c_mcr_b.sadr = uTargetAddress.array[1];
      tCMD |= ((uTargetAddress.array[0] >> 7) << i2c_app_i2c_cmd_nwr_Pos) & i2c_app_i2c_cmd_nwr_Msk;
      tCMD |= (0 << i2c_app_i2c_cmd_tsize_Pos) & i2c_app_i2c_cmd_tsize_Msk;
      tCMD |= (DRV_I2C_COMMAND_S_AC_TC << i2c_app_i2c_cmd_cmd_Pos) & i2c_app_i2c_cmd_cmd_Msk;
      tCMD |= (0 << i2c_app_i2c_cmd_acpollmax_Pos) & i2c_app_i2c_cmd_acpollmax_Msk;
      ptDriver->ptDevice->i2c_mcr_b.mode = eSpeed;
      ptDriver->ptDevice->i2c_irqmsk_b.cmd_err = 0;
      ptDriver->ptDevice->i2c_cmd = tCMD;

      while((DRV_I2C_COMMAND_CT   != ptDriver->ptDevice->i2c_cmd_b.cmd) &&
            (DRV_I2C_COMMAND_CTC  != ptDriver->ptDevice->i2c_cmd_b.cmd) &&
            (DRV_I2C_COMMAND_IDLE != ptDriver->ptDevice->i2c_cmd_b.cmd) &&
            (DRV_I2C_COMMAND_STOP != ptDriver->ptDevice->i2c_cmd_b.cmd))
      {
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          ret = DRV_TOUT;
          DRV_UNLOCK(ptDriver);
          return ret;
        }
      }

      ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_cmd_err_Msk;
      ptDriver->ptDevice->i2c_mcr_b.sadr = uTargetAddress.array[0];
      ptDriver->ptDevice->i2c_mcr_b.mode = ptDriver->tConfiguration.eSpeedMode;

      if(0 == ptDriver->ptDevice->i2c_sr_b.bus_master)
      {
        if(ptDriver->tConfiguration.fnArbitrationLostCallback != 0)
        {
          ptDriver->tConfiguration.fnArbitrationLostCallback(ptDriver,
                                                             ptDriver->tConfiguration.pArbitrationLostCallbackHandle);
        }
        ret = DRV_ERROR;
      }
      else
      {
        if(ptDriver->tConfiguration.fnMasterCompleteCallback != 0)
        {
          ptDriver->tConfiguration.fnMasterCompleteCallback(ptDriver,
                                                            ptDriver->tConfiguration.pMasterCompleteCallbackHandle);
        }
        ret = DRV_OK;
      }

      ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;
      ptDriver->ptDevice->i2c_irqmsk_b.cmd_err = 1;

      drv_I2C_ResetMstBuffer(ptDriver);
    }
    else
    {
      ret = DRV_ERROR_PARAM;
    }
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The clear function puts out 9 clock pulses on the I2C bus in the defined speed.
 *
 * Because there is no hardware support for it in the netX 90, this call uses the PIO mode
 * of the device.
 * It is blocking and should only be used directly after initialization.
 * This functionality is not conform to the original specification of I2C and contradicts
 * with a lot of the available implementations of I2C in the field.
 *
 * The duration of the high and low phase loops have to be provided as a parameter and may
 * differ widely between builds and hardware.
 *
 * Example calculation: 10ns * 100.000 = 1kHz / 100 = 100kHz with two phases and
 * 23 or 11 instructions result in 100.000 / 23 as loop frequency / 100 / 2 as loop
 * iteration counter.
 * In debug builds: 100000 / 23 / 100 / 2 approx. 100kHz.
 * In release builds: 100000 / 11 / 100 / 2 approx. 100kHz.
 *
 * \public
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] ulLoops Amount of iterations the IO of SClk is put low and high
 * \return DRV_OK if everything is fine
 *         DRV_LOCKED The API is locked and not accessible.
 *         DRV_ERROR_PARAM A given parameter is not correct.
 */
DRV_STATUS_E DRV_I2C_Clear(DRV_I2C_HANDLE_T* const ptDriver,
                           const size_t ulLoops)
{
  DRV_HANDLE_CHECK(ptDriver);
  if(ulLoops == 0)
  {
    return DRV_ERROR_PARAM;
  }

  (void) DRV_IRQ_Disable();
  DRV_I2C_STATE_E tState;
  DRV_STATUS_E ret;
  if((ret = DRV_I2C_GetState(ptDriver, &tState)) != DRV_OK)
  {
    (void) DRV_IRQ_Enable();
    return ret;
  }

  ptDriver->ptDevice->i2c_mcr_b.pio_mode = 1;
  ptDriver->ptDevice->i2c_pio_b.scl_oe = 1;

  for(size_t i = 0; i < ulLoops; i++)
  {
    ptDriver->ptDevice->i2c_pio_b.scl_out = 0;
  }

  for(size_t j = 0; j < 9; j++)
  {
    for(size_t i = 0; i < ulLoops; i++)
    {
      ptDriver->ptDevice->i2c_pio_b.scl_out = 1;
    }
    for(size_t i = 0; i < ulLoops; i++)
    {
      ptDriver->ptDevice->i2c_pio_b.scl_out = 0;
    }
  }

  ptDriver->ptDevice->i2c_pio_b.scl_oe = 0;
  ptDriver->ptDevice->i2c_mcr_b.pio_mode = 0;

  (void) DRV_IRQ_Enable();

  return DRV_OK;
}

/*!
 * The master transmit function will initiate a transmit of the given data of the given size to the given slave address.
 *
 * If high speed mode is active, the master code has to be transmitted first.
 *
 * The eCont argument defines if the Master shall keep arbitration context after the transmission or not.
 * If the master keeps the arbitration, its behavior is different depending on the previous command:
 *
 * If the previous operation was also master transmit (no direction changing),
 *   next transmission starts directly with clock pulses on SClk line, without (r)Start, Addressing, Ack
 *
 * If the previous operation was master receive (direction changing),
 *   next transmission starts with (r)Start, Addressing, R/W bit, Ack/NAck from Slave.
 *
 * Caution: When changing direction from continued master receive to master transmit, there is risk of
 * SDa line stalling, depending on the most significant bit of the first byte in the slave FIFO.
 * In this specific case it is recommended the last master receive call to be in not continued mode
 * (even with dummy call and discarding the received data) in order to guarantee the SDa line will be at high level
 * before the (r)Start command.
 *
 * If the function returns error, a deinit and new init call are recommended. One of the reasons is that the
 * FIFOs are the same for transmit and receive operations and parts of data, remaining in FIFO from previous transfers,
 * would cause incorrect behavior.
 *
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] tTargetAddress as described in \ref i2c_addressing_scheme
 * \param[in] pbData Pointer to the data to transmit
 * \param[in] size Amount of chars to transmit. Must be greater than 0 and not greater than 1024
 * \param[in] eCont Determines if the Master will keep bus arbitration after the transmission.
 * \return DRV_OK if everything is fine.
 */
DRV_STATUS_E
DRV_I2C_MasterTransmit(DRV_I2C_HANDLE_T* const ptDriver,
                       DRV_I2C_ADDRESS_T tTargetAddress,
                       const uint8_t* const pbData,
                       size_t size,
                       DRV_I2C_CONT_E eCont)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  DRV_STATUS_E eStatus = DRV_OK;
  ptDriver->ullFrameStartTick = 0;
  uint32_t ulCommand = 0;

  eStatus = drv_I2C_VerifyParams(ptDriver, &tTargetAddress, pbData, size, eCont);
  if (DRV_OK != eStatus)
  {
    DRV_UNLOCK(ptDriver);
    return eStatus;
  }

  drv_I2C_LoadTxParams(ptDriver, &tTargetAddress, pbData, size, eCont);

  ulCommand = drv_I2C_ComposeTxCommand(ptDriver, size, eCont);

  if(DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode)
  {
    ptDriver->ptDevice->i2c_cmd = ulCommand;

    /* Wait hardware to change state */
    eStatus = drv_I2C_WaitTxStartSeqAcknowledged(ptDriver);
    if(DRV_TOUT == eStatus)
    {
      DRV_UNLOCK(ptDriver);
      return eStatus;
    }

    /* Master transmits data */
    eStatus = drv_I2C_TransferData(ptDriver);
    if(DRV_TOUT == eStatus)
    {
      DRV_UNLOCK(ptDriver);
      return eStatus;
    }

    /* Wait hardware to change state */
    eStatus = drv_I2C_WaitTransferEnd(ptDriver);
    if(DRV_TOUT == eStatus)
    {
      DRV_UNLOCK(ptDriver);
      return eStatus;
    }

    /* Check if the transfer is not to be continued */
    if(DRV_I2C_CONT_END == eCont)
    {
      ptDriver->ptDevice->i2c_cmd_b.cmd = DRV_I2C_COMMAND_STOP;
    }

    /* Wait hardware to change state */
    eStatus = drv_I2C_WaitIdle(ptDriver);
    if(DRV_TOUT == eStatus)
    {
      DRV_UNLOCK(ptDriver);
      return eStatus;
    }

    /* Check for errors and call the error callbacks accordingly */
    eStatus = drv_I2C_CheckForError(ptDriver);

    if((DRV_OK == eStatus) &&
       (NULL != ptDriver->tConfiguration.fnMasterCompleteCallback))
    {
      ptDriver->tConfiguration.fnMasterCompleteCallback(ptDriver,
                                                        ptDriver->tConfiguration.pMasterCompleteCallbackHandle);
    }

    ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;

    drv_I2C_ResetMstBuffer(ptDriver);
  }
  else if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
  {
    (void) DRV_IRQ_Disable();

    ptDriver->ptDevice->i2c_cmd = ulCommand;

    ptDriver->ptDevice->i2c_irqmsk_b.cmd_ok = 1;
    ptDriver->ptDevice->i2c_irqmsk_b.cmd_err = 1;
    ptDriver->ptDevice->i2c_irqmsk_b.fifo_err = 1;
    ptDriver->ptDevice->i2c_irqmsk_b.mfifo_req = 1;

    (void) DRV_IRQ_Enable();

    eStatus = DRV_OK;
  }
  else if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
  {
    ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;

    drv_I2C_ResetMstBuffer(ptDriver);

    eStatus = DRV_NSUPP;
  }
  else
  {
    ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;

    drv_I2C_ResetMstBuffer(ptDriver);

    eStatus = DRV_ERROR_PARAM;
  }

  DRV_UNLOCK(ptDriver);

  return eStatus;
}

/*!
 * The master receive function will initiate a receive of data with the given size from the addressed slave.
 *
 * If high speed mode is active, the master code has to be transmitted first.
 *
 * The eCont argument defines if the Master shall keep arbitration context after the transmission or not.
 * If the master keeps the arbitration, its behavior is different depending on the previous command:
 *
 * If the previous operation was also master receive (no direction changing),
 *   next transmission starts directly with clock pulses on SClk line, without (r)Start, Addressing, Ack
 *
 * If the previous operation was master transmit (direction changing),
 *   next transmission starts with (r)Start, Addressing, R/W bit, Ack/NAck from Slave.
 *
 * If the function returns error, a deinit and new init call is recommended. One of the reasons is that the
 * FIFOs are the same for transmit and receive operations and parts of data, remaining in FIFO from previous transfers,
 * would cause incorrect behavior.
 *
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] tTargetAddress as described in \ref i2c_addressing_scheme
 * \param[out] pbData Pointer to the data, that will be received
 * \param[in] size Amount of chars to be received. Must be greater than 0 and not greater than 1024
 * \param[in] eCont Determines if the Master will keep bus arbitration after the transmission
 * \return DRV_OK if everything is fine
 */
DRV_STATUS_E DRV_I2C_MasterReceive(DRV_I2C_HANDLE_T* const ptDriver,
                                   DRV_I2C_ADDRESS_T tTargetAddress,
                                   uint8_t* const pbData,
                                   size_t size,
                                   DRV_I2C_CONT_E eCont)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  DRV_STATUS_E eStatus = DRV_OK;
  ptDriver->ullFrameStartTick = 0;
  uint32_t ulCommand = 0;

  eStatus = drv_I2C_VerifyParams(ptDriver, &tTargetAddress, pbData, size, eCont);

  if (DRV_OK != eStatus)
  {
    DRV_UNLOCK(ptDriver);
    return eStatus;
  }

  eStatus = drv_I2C_LoadRxParams(ptDriver, &tTargetAddress, pbData, size, eCont);
  if(DRV_TOUT == eStatus)
  {
    DRV_UNLOCK(ptDriver);
    return eStatus;
  }

  ulCommand = drv_I2C_ComposeRxCommand(ptDriver, size, eCont);

  if(DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode)
  {
    ptDriver->ptDevice->i2c_cmd = ulCommand;

    /* Wait hardware to change state */
    eStatus = drv_I2C_WaitRxStartSeqAcknowledged(ptDriver);
    if(DRV_TOUT == eStatus)
    {
      DRV_UNLOCK(ptDriver);
      return eStatus;
    }

    /* Master receives data */
    eStatus = drv_I2C_TransferData(ptDriver);
    if(DRV_TOUT == eStatus)
    {
      DRV_UNLOCK(ptDriver);
      return eStatus;
    }

    /* Wait hardware to change state */
    eStatus = drv_I2C_WaitTransferEnd(ptDriver);
    if(DRV_TOUT == eStatus)
    {
      DRV_UNLOCK(ptDriver);
      return eStatus;
    }

    /* Check if the transfer is not to be continued */
    if(DRV_I2C_CONT_END == eCont)
    {
      ptDriver->ptDevice->i2c_cmd_b.cmd = DRV_I2C_COMMAND_STOP;
    }

    /* Wait hardware to change state */
    eStatus = drv_I2C_WaitIdle(ptDriver);
    if(DRV_TOUT == eStatus)
    {
      DRV_UNLOCK(ptDriver);
      return eStatus;
    }

    /* Check for errors and call the error callbacks accordingly */
    eStatus = drv_I2C_CheckForError(ptDriver);

    if((DRV_OK == eStatus) &&
       (NULL != ptDriver->tConfiguration.fnMasterCompleteCallback))
    {
      ptDriver->tConfiguration.fnMasterCompleteCallback(ptDriver,
                                                        ptDriver->tConfiguration.pMasterCompleteCallbackHandle);
    }

    ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;

    drv_I2C_ResetMstBuffer(ptDriver);
  }
  else if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
  {
    (void) DRV_IRQ_Disable();

    ptDriver->ptDevice->i2c_cmd = ulCommand;

    ptDriver->ptDevice->i2c_irqmsk_b.cmd_ok = 1;
    ptDriver->ptDevice->i2c_irqmsk_b.cmd_err = 1;
    ptDriver->ptDevice->i2c_irqmsk_b.fifo_err = 1;
    ptDriver->ptDevice->i2c_irqmsk_b.mfifo_req = 1;

    (void) DRV_IRQ_Enable();

    eStatus = DRV_OK;
  }
  else if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
  {
    ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;

    drv_I2C_ResetMstBuffer(ptDriver);

    eStatus = DRV_NSUPP;
  }
  else
  {
    ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;

    drv_I2C_ResetMstBuffer(ptDriver);

    eStatus = DRV_ERROR_PARAM;
  }

  DRV_UNLOCK(ptDriver);

  return eStatus;
}

/*!
 * The slave transmit prepares the data, that the master will read later.
 *
 * A slave transaction has to be queried before the start on the bus is generated or an
 * FIFO underrun will occur.
 *
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] pbData Pointer to the data to transmit
 * \param[in] size Amount of chars to transmit. Must be greater than zero and not greater than 1024
 * \return DRV_OK if everything is fine
 */
DRV_STATUS_E DRV_I2C_SlaveTransmit(DRV_I2C_HANDLE_T* const ptDriver,
                                   uint8_t const * const pbData,
                                   size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_STATUS_E eRslt = DRV_ERROR;

  if((NULL == pbData) ||
     (1024 < size)    ||
     (0 == size))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  ptDriver->ullFrameStartTick = 0;

  eRslt = drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_WAIT_ADDRESSING);

  /* Check if there were no previous unfinished slave operations. */
  if(DRV_OK == eRslt)
  {
    if((size <= ptDriver->tConfiguration.eSlvTxFifoWatermark) && (size > 0))
    {
      ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_wm = size;
    }
    else
    {
      ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_wm = ptDriver->tConfiguration.eSlvTxFifoWatermark;
    }
    ptDriver->SlvBuffer = (void*) pbData;
    ptDriver->SlvBufferCounter = 0;
    ptDriver->SlvBufferSize = size;
    ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_WRITE;

    /* Start is not acknowledged initially, but after direction check or full fit into the FIFO. */
    ptDriver->ptDevice->i2c_scr_b.ac_start = 0;

    /* This bit is also related to the slave transmit function and must be set before transmitting data */
    ptDriver->ptDevice->i2c_scr_b.ac_srx = 1;

    if(DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode)
    {
      /* The data should be in the slave FIFO before the start of the read is acknowledged, otherwise an error occurs. */
      drv_I2C_FillSlaveTxFifo(ptDriver);

      /* Slave does not delay addressing acknowledge, if all the data fit in the FIFO and are ready for transfer. */
      if(ptDriver->SlvBufferCounter == ptDriver->SlvBufferSize)
      {
        ptDriver->ptDevice->i2c_scr_b.ac_start = 1;
      }
      while(ptDriver->SlvBufferCounter < ptDriver->SlvBufferSize)
      {
        /* In polling mode, if the transfer size is greater than the slave FIFO size, the function is blocking.
         * If the transfer size fits in the slave FIFO, the function returns result,
         *  even if the transmission is not finished. */
        DRV_I2C_Flush_Buffers(ptDriver);
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          DRV_UNLOCK(ptDriver);
          return DRV_TOUT;
        }
      }

      drv_I2C_CallEndTransferCallback(ptDriver);

      ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_UNKNOWN;

      drv_I2C_ResetSlvBuffer(ptDriver);

      (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_IDLE);

      eRslt = DRV_OK;
    }
    else if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
    {
      (void) DRV_IRQ_Disable();
      ptDriver->ptDevice->i2c_irqmsk_b.sfifo_req = 0;
      ptDriver->ptDevice->i2c_irqmsk_b.sreq = 1;

      /* The data should be in the slave FIFO before the start of the read is acknowledged, otherwise an error occurs. */
      drv_I2C_FillSlaveTxFifo(ptDriver);

      (void) DRV_IRQ_Enable();
      eRslt = DRV_OK;
    }
    else if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
    {
      ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_UNKNOWN;

      drv_I2C_ResetSlvBuffer(ptDriver);

      eRslt = DRV_NSUPP;
    }
    else
    {
      ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_UNKNOWN;

      drv_I2C_ResetSlvBuffer(ptDriver);

      eRslt = DRV_ERROR_PARAM;
    }
  }

  DRV_UNLOCK(ptDriver);

  return eRslt;
}

/*!
 * The slave receive function will prepare the hardware to receive data, transmitted later by the I2C master.
 *
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] pbData Pointer to the data, that will be received
 * \param[in] size Amount of chars to be received. Must be greater than 0 and not greater than 1024
 * \return DRV_OK if everything is fine
 */
DRV_STATUS_E DRV_I2C_SlaveReceive(DRV_I2C_HANDLE_T* const ptDriver,
                                  uint8_t* const pbData,
                                  size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  DRV_STATUS_E eRslt = DRV_ERROR;

  if((NULL == pbData) ||
     (1024 < size)    ||
     (0 == size))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  ptDriver->ullFrameStartTick = 0;

  eRslt = drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_WAIT_ADDRESSING);

  /* Check if there were no previous unfinished slave operations. */
  if(DRV_OK == eRslt)
  {
    /* Adjust Slave FIFO watermark */
    if((size <= ptDriver->tConfiguration.eSlvRxFifoWatermark) && (size > 0))
    {
      ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_wm = size - 1;
    }
    else
    {
      ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_wm = ptDriver->tConfiguration.eSlvRxFifoWatermark - 1;
    }

    /* Prepare Slave Buffer */
    ptDriver->SlvBuffer = (void*) pbData;
    ptDriver->SlvBufferCounter = 0;
    ptDriver->SlvBufferSize = size;

    /* Slave does not acknowledge addressing until the direction is checked. */
    ptDriver->ptDevice->i2c_scr_b.ac_start = 0;

    ptDriver->ptDevice->i2c_scr_b.ac_srx = 1;

    ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_READ;

    if(DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode)
    {
      while(ptDriver->SlvBufferCounter < ptDriver->SlvBufferSize)
      {
        /* In polling mode, if the transfer size is greater than the slave FIFO size, the function is blocking.
         * If the transfer size fits in the slave FIFO, the function returns result,
         * even if the transmission is not finished. */
        DRV_I2C_Flush_Buffers(ptDriver);
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          DRV_UNLOCK(ptDriver);
          return DRV_TOUT;
        }
      }

      drv_I2C_CallEndTransferCallback(ptDriver);

      ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_UNKNOWN;

      (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_IDLE);

      drv_I2C_ResetSlvBuffer(ptDriver);

      eRslt = DRV_OK;
    }
    else if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
    {
      (void) DRV_IRQ_Disable();
      ptDriver->ptDevice->i2c_irqmsk_b.sfifo_req = 0;
      ptDriver->ptDevice->i2c_irqmsk_b.sreq = 1;

      (void) DRV_IRQ_Enable();
      eRslt = DRV_OK;
    }
    else if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
    {
      ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_UNKNOWN;

      drv_I2C_ResetSlvBuffer(ptDriver);

      eRslt = DRV_NSUPP;
    }
    else
    {
      ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_UNKNOWN;

      drv_I2C_ResetSlvBuffer(ptDriver);

      eRslt = DRV_ERROR_PARAM;
    }
  }

  DRV_UNLOCK(ptDriver);

  return eRslt;
}

/*!
 * This function aborts any transmissions on the given I2C.
 * A reinitialize afterwards might be necessary to attach back to the bus in a valid state.
 *
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return DRV_OK if everything is fine
 */
DRV_STATUS_E DRV_I2C_Abort(DRV_I2C_HANDLE_T* const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;

  DRV_HANDLE_CHECK(ptDriver);

  DRV_LOCK(ptDriver);

  switch(ptDriver->tConfiguration.eOperationMode)
  {
    case DRV_OPERATION_MODE_POLL:
    {
      /* Disable slave acknowledgments */
      ptDriver->ptDevice->i2c_scr_b.ac_start = 0;
      ptDriver->ptDevice->i2c_scr_b.ac_srx = 0;

      /* Clear FIFOs */
      ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_clr = 1;
      ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_clr = 1;

      /* Reset buffers */
      drv_I2C_ResetMstBuffer(ptDriver);
      drv_I2C_ResetSlvBuffer(ptDriver);

      /* Generate Stop if Bus is occupied, Device is Bus Master and Start was acknowledged */
      drv_I2C_GenerateConditionalStop(ptDriver);

      break;
    }

    case DRV_OPERATION_MODE_IRQ:
    {
      DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN]);

      DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN]);

      /* Disable acknowledge of start sequence and received data from slave */
      ptDriver->ptDevice->i2c_scr_b.ac_start = 0;
      ptDriver->ptDevice->i2c_scr_b.ac_srx = 0;

      /* Clear FIFOs */
      ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_clr = 1;
      ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_clr = 1;

      /* Clear Interrupt Flags and Mask */
      ptDriver->ptDevice->i2c_irqsr  = DRV_I2C_CLEAR_IRQSR_FLAGS;
      ptDriver->ptDevice->i2c_irqmsk = 0u;

      /* Reset buffers */
      drv_I2C_ResetMstBuffer(ptDriver);
      drv_I2C_ResetSlvBuffer(ptDriver);

      /* Generate Stop if Bus is occupied, Device is Bus Master and Start was acknowledged */
      drv_I2C_GenerateConditionalStop(ptDriver);

      DRV_NVIC_EnableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN]);

      break;
    }

    case DRV_OPERATION_MODE_DMA:
      eRslt = DRV_NSUPP;
      break;

    default:
      eRslt = DRV_ERROR_PARAM;
      break;
  }

  (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_IDLE);

  DRV_UNLOCK(ptDriver);

  return eRslt;
}

/*!
 * The get state function shall modify the value, pointed by ptState, in accordance with the device internal state,
 * return the states of the driver and indicate an error.
 *
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] ptState The pointer to where the state of the device given will be stored
 * \return DRV_OK if everything is fine
 */
DRV_STATUS_E DRV_I2C_GetState(DRV_I2C_HANDLE_T* const ptDriver,
                              DRV_I2C_STATE_E* const ptState)
{
  DRV_HANDLE_CHECK(ptDriver);

  if(ptState == NULL)
  {
    return DRV_ERROR_PARAM;
  }

  *ptState = (DRV_I2C_STATE_E) 0ull;
  *ptState = (DRV_I2C_STATE_E) ptDriver->ptDevice->i2c_sr;

  if(ptDriver->ptDevice->i2c_cmd_b.nwr)
  {
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_READ);
  }

  switch (ptDriver->ptDevice->i2c_cmd_b.cmd)
  {
  case DRV_I2C_COMMAND_START:
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_START);
    break;
  case DRV_I2C_COMMAND_S_AC:
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_S_AC);
    break;
  case DRV_I2C_COMMAND_S_AC_T:
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_S_AC_T);
    break;
  case DRV_I2C_COMMAND_S_AC_TC:
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_S_AC_TC);
    break;
  case DRV_I2C_COMMAND_CT:
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_CT);
    break;
  case DRV_I2C_COMMAND_CTC:
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_CTC);
    break;
  case DRV_I2C_COMMAND_STOP:
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_STOP);
    break;
  case DRV_I2C_COMMAND_IDLE:
    *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (uint64_t) DRV_I2C_STATE_MASTER_CMD_IDLE);
    break;
  default:
    break;
  }

  uint64_t fill = ptDriver->ptDevice->i2c_cmd_b.tsize;
  *ptState = (DRV_I2C_STATE_E) (((uint64_t) *ptState) | (fill << 48));

  DRV_STATUS_E ret = DRV_OK;

  if((!((uint64_t) *ptState & (uint64_t) DRV_I2C_STATE_MASTER_CMD_IDLE) ||
       ((uint64_t) *ptState & (uint64_t) DRV_I2C_STATE_MASTER_CMD_CTC))         ||
     (NULL != ptDriver->SlvBuffer)                                              ||
     (ptDriver->SlvBufferCounter > 0)                                           ||
     (NULL != ptDriver->MstBuffer)                                              ||
     (ptDriver->MstBufferCounter != ptDriver->MstBufferSize))
  {
    ret = DRV_BUSY;
  }

  if(ptDriver->ptDevice->i2c_irqsr_b.cmd_err)
  {
    ret = DRV_ERROR;
  }

  return ret;
}

/*!
 * \brief Fills the master FIFO with data, based on the current master FIFO level, refill level and master buffer state.
 */
__STATIC_INLINE void
drv_I2C_FillMasterTxFifo(DRV_I2C_HANDLE_T* const ptDriver)
{
  uint32_t ulBorder = ptDriver->tConfiguration.eMstTxFifoRefillLevel - ptDriver->ptDevice->i2c_sr_b.mfifo_level;
  uint32_t ulIdx = 0;

  if(ptDriver->MstBufferCounter + ulBorder >= ptDriver->MstBufferSize)
  {
    ulBorder = ptDriver->MstBufferSize - ptDriver->MstBufferCounter;
  }

  for(ulIdx = 0; ulIdx < ulBorder; ulIdx++)
  {
    ptDriver->ptDevice->i2c_mdr = ((uint8_t*) ptDriver->MstBuffer)[ptDriver->MstBufferCounter + ulIdx];
  }

  ptDriver->MstBufferCounter += ulBorder;
}

/*!
 * \brief In master receive function, reads data from the master FIFO
 */
__STATIC_INLINE void
drv_I2C_ReadMasterRxFifo(DRV_I2C_HANDLE_T* const ptDriver)
{
  uint32_t ulBorder = ptDriver->ptDevice->i2c_sr_b.mfifo_level;
  uint32_t ulIdx = 0;

  if(ptDriver->MstBufferCounter + ulBorder >= ptDriver->MstBufferSize)
  {
    ulBorder = ptDriver->MstBufferSize - ptDriver->MstBufferCounter;
  }

  for(ulIdx = 0; ulIdx < ulBorder; ulIdx++)
  {
    ((uint8_t*) ptDriver->MstBuffer)[ptDriver->MstBufferCounter + ulIdx] = ptDriver->ptDevice->i2c_mdr;
  }

  ptDriver->MstBufferCounter += ulBorder;
}

/*!
 * \brief This function is used in the slave transmit function in order to provide data in the FIFO
 *  before the start of the read is acknowledged, otherwise an error would occur at higher speeds.
 */
__STATIC_INLINE void
drv_I2C_FillSlaveTxFifo(DRV_I2C_HANDLE_T* const ptDriver)
{
  uint32_t ulBorder = ptDriver->tConfiguration.eSlvTxFifoRefillLevel - ptDriver->ptDevice->i2c_sr_b.sfifo_level;
  uint32_t ulIdx = 0;

  if((ptDriver->SlvBufferCounter + ulBorder) >= ptDriver->SlvBufferSize)
  {
    ulBorder = ptDriver->SlvBufferSize - ptDriver->SlvBufferCounter;
  }

  for(ulIdx = 0; ulIdx < ulBorder; ulIdx++)
  {
    ptDriver->ptDevice->i2c_sdr = ((uint8_t*) ptDriver->SlvBuffer)[ptDriver->SlvBufferCounter + ulIdx];
  }

  ptDriver->SlvBufferCounter += ulBorder;
}

/*!
 * \brief In slave receive function, reads data from the slave FIFO
 */
__STATIC_INLINE void
drv_I2C_ReadSlaveRxFifo(DRV_I2C_HANDLE_T* const ptDriver)
{
  uint32_t ulBorder = ptDriver->ptDevice->i2c_sr_b.sfifo_level;
  uint32_t ulIdx = 0;

  if(ptDriver->SlvBufferCounter + ulBorder >= ptDriver->SlvBufferSize)
  {
    ulBorder = ptDriver->SlvBufferSize - ptDriver->SlvBufferCounter;
  }

  for(ulIdx = 0; ulIdx < ulBorder; ulIdx++)
  {
    ((uint8_t*) ptDriver->SlvBuffer)[ptDriver->SlvBufferCounter + ulIdx] = ptDriver->ptDevice->i2c_sdr;
  }

  ptDriver->SlvBufferCounter += ulBorder;
}

/*!
 * \brief In master transmit and master receive functions, verifies the input parameters and the master buffer state.
 */
__STATIC_INLINE DRV_STATUS_E
drv_I2C_VerifyParams(DRV_I2C_HANDLE_T* const ptDriver,
                     DRV_I2C_ADDRESS_T* ptTargetAddress,
                     const uint8_t* const pbData,
                     uint32_t ulSize,
                     DRV_I2C_CONT_E eCont)
{
  DRV_STATUS_E eStatus = DRV_OK;

  if((NULL == pbData) ||
     (1024 < ulSize)  ||
     (0 == ulSize))
  {
    eStatus = DRV_ERROR_PARAM;
  }
  else if((DRV_I2C_ADDRESS_10_BIT      != ptTargetAddress->eAddressingScheme) &&
          (DRV_I2C_ADDRESS_7_BIT       != ptTargetAddress->eAddressingScheme) &&
          (DRV_I2C_ADDRESS_MASTER_CODE != ptTargetAddress->eAddressingScheme))
  {
    eStatus = DRV_ERROR_PARAM;
  }
  else if(ptDriver->tConfiguration.tSlaveAddress.uDeviceAddress == ptTargetAddress->uDeviceAddress)
  {
    eStatus = DRV_ERROR_PARAM;
  }
  else if((uint32_t)DRV_I2C_CONT_MAX < (uint32_t)eCont)
  {
    eStatus = DRV_ERROR_PARAM;
  }
  else if((DRV_I2C_SPEED_MODE_HS_800k <= ptDriver->tConfiguration.eSpeedMode) &&
          (DRV_I2C_CONT_END == ptDriver->eCT))
  {
    /* Return error, if a high speed transfer is requested without sending the high speed master code first. */
    eStatus = DRV_ERROR;
  }
  else if((NULL != ptDriver->MstBuffer)  ||
          (0 != ptDriver->MstBufferSize) ||
          (0 != ptDriver->MstBufferCounter))
  {
    eStatus = DRV_BUSY;
  }

  return eStatus;
}

/*!
 * \brief In master transmit function, loads the corresponding parameters in the configuration structure
 */
__STATIC_INLINE void
drv_I2C_LoadTxParams(DRV_I2C_HANDLE_T* const ptDriver,
                     DRV_I2C_ADDRESS_T* ptTargetAddress,
                     const uint8_t* const pbData,
                     uint32_t ulSize,
                     DRV_I2C_CONT_E eCont)
{
    if((ulSize <= ptDriver->tConfiguration.eMstTxFifoWatermark) &&
       (ulSize > 0))
    {
      ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_wm = ulSize;
    }
    else
    {
      ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_wm = ptDriver->tConfiguration.eMstTxFifoWatermark;
    }

    ptDriver->MstBuffer = (void*) pbData;
    ptDriver->MstBufferCounter = 0;
    ptDriver->MstBufferSize = ulSize;
    ptDriver->eCT = eCont;
    ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_WRITE;

    if(DRV_I2C_ADDRESS_10_BIT == ptTargetAddress->eAddressingScheme)
    {
      /* The 10-bit addressing procedure steps are performed only if there is no valid previous 10-bit slave acknowledgment. */
      if(0 == ptDriver->ptDevice->i2c_sr_b.sid10_aced)
      {
        DRV_I2C_ADDRESS_U uTargetAddress;
        uTargetAddress.bf = *ptTargetAddress;
        ptDriver->ptDevice->i2c_mcr_b.sadr = (uint32_t) uTargetAddress.array[1];
        ptDriver->ptDevice->i2c_mdr = (uint32_t)uTargetAddress.array[0];
      }
    }
    else /* DRV_I2C_ADDRESS_7_BIT or DRV_I2C_ADDRESS_MASTER_CODE */
    {
      ptDriver->ptDevice->i2c_mcr_b.sadr = ptTargetAddress->uDeviceAddress;
    }
}

/*!
 * \brief In master receive function, loads the corresponding parameters in the configuration structure
 */
__STATIC_INLINE DRV_STATUS_E
drv_I2C_LoadRxParams(DRV_I2C_HANDLE_T* const ptDriver,
                     DRV_I2C_ADDRESS_T* ptTargetAddress,
                     uint8_t* const pbData,
                     uint32_t ulSize,
                     DRV_I2C_CONT_E eCont)
{
  DRV_STATUS_E eStatus = DRV_OK;

  if((ulSize <= ptDriver->tConfiguration.eMstRxFifoWatermark) &&
     (ulSize > 0))
  {
    ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_wm = ulSize;
  }
  else
  {
    ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_wm = ptDriver->tConfiguration.eMstRxFifoWatermark;
  }
  ptDriver->MstBuffer = (void*)pbData;
  ptDriver->MstBufferCounter = 0;
  ptDriver->MstBufferSize = ulSize;
  ptDriver->eCT = eCont;
  ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_READ;

  if(DRV_I2C_ADDRESS_10_BIT == ptTargetAddress->eAddressingScheme)
  {
    /* The 10-bit addressing procedure steps are performed only if there is no valid previous 10-bit slave acknowledgment. */
    if(0 == ptDriver->ptDevice->i2c_sr_b.sid10_aced)
    {
      DRV_I2C_ADDRESS_U uTargetAddress;
      uTargetAddress.bf = *ptTargetAddress;

      /* In accordance with standard THE I 2C-BUS SPECIFICATION, VERSION 2.1, JANUARY 2000 from Philips (fig.27),
       * when receiving data in 10-bit addressing mode, Master first has to send the first addressing byte
       * with direction bit Write (11110AA0), where AA are Address bits 9 and 8, then to send the second
       * addressing byte A2 (AAAAAAAA), where AAAAAAAA are address bits 7 to 0. */
      ptDriver->ptDevice->i2c_mcr_b.sadr = (uint32_t) uTargetAddress.array[1];
      ptDriver->ptDevice->i2c_mdr = (uint32_t)uTargetAddress.array[0];
      ptDriver->ptDevice->i2c_cmd_b.nwr = DRV_I2C_RW_FLAG_WRITE;
      ptDriver->ptDevice->i2c_cmd_b.cmd = DRV_I2C_COMMAND_S_AC;
      while(DRV_I2C_COMMAND_IDLE != ptDriver->ptDevice->i2c_cmd_b.cmd)
      {
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          ptDriver->ullFrameStartTick = 0;
          eStatus = DRV_TOUT;
          break;
        }
      }
    }
  }
  else /* DRV_I2C_ADDRESS_7_BIT or DRV_I2C_ADDRESS_MASTER_CODE */
  {
    ptDriver->ptDevice->i2c_mcr_b.sadr = ptTargetAddress->uDeviceAddress;
  }

  return eStatus;
}

/*!
 * \brief In master transmit function, prepares the cmd register contents, in order to be written at once.
 */
__STATIC_INLINE uint32_t
drv_I2C_ComposeTxCommand(DRV_I2C_HANDLE_T* const ptDriver,
                         uint32_t ulSize,
                         DRV_I2C_CONT_E eCont)
{
  uint32_t ulSelectedCommand = DRV_I2C_COMMAND_IDLE;
  uint32_t ulCommand = (DRV_I2C_RW_FLAG_WRITE << i2c_app_i2c_cmd_nwr_Pos) & i2c_app_i2c_cmd_nwr_Msk;

  ulCommand |= ((ulSize - 1) << i2c_app_i2c_cmd_tsize_Pos) & i2c_app_i2c_cmd_tsize_Msk;

  ulSelectedCommand = drv_I2C_SelectTxCommand(ptDriver, eCont);
  ulCommand |= (ulSelectedCommand << i2c_app_i2c_cmd_cmd_Pos) & i2c_app_i2c_cmd_cmd_Msk;

  ulCommand |= (ptDriver->tConfiguration.sAckPollMaximum << i2c_app_i2c_cmd_acpollmax_Pos) & i2c_app_i2c_cmd_acpollmax_Msk;

  return ulCommand;
}

/*!
 * \brief In master receive function, prepares the cmd register contents, in order to be written at once.
 */
__STATIC_INLINE uint32_t
drv_I2C_ComposeRxCommand(DRV_I2C_HANDLE_T* const ptDriver,
                         uint32_t ulSize,
                         DRV_I2C_CONT_E eCont)
{
  uint32_t ulSelectedCommand = DRV_I2C_COMMAND_IDLE;
  uint32_t ulCommand = (DRV_I2C_RW_FLAG_READ << i2c_app_i2c_cmd_nwr_Pos) & i2c_app_i2c_cmd_nwr_Msk;

  ulCommand |= ((ulSize - 1) << i2c_app_i2c_cmd_tsize_Pos) & i2c_app_i2c_cmd_tsize_Msk;

  ulSelectedCommand = drv_I2C_SelectRxCommand(ptDriver, eCont);
  ulCommand |= (ulSelectedCommand << i2c_app_i2c_cmd_cmd_Pos) & i2c_app_i2c_cmd_cmd_Msk;

  ulCommand |= (ptDriver->tConfiguration.sAckPollMaximum << i2c_app_i2c_cmd_acpollmax_Pos) & i2c_app_i2c_cmd_acpollmax_Msk;

  return ulCommand;
}

/*!
 * \brief In master transmit function, synchronizes the main context with the hardware context.
 */
__STATIC_INLINE DRV_STATUS_E
drv_I2C_WaitTxStartSeqAcknowledged(DRV_I2C_HANDLE_T* const ptDriver)
{
  bool fIsStartSeqAcknowledged = drv_I2C_IsTxStartSeqAcknowledged(ptDriver);

  while((false == fIsStartSeqAcknowledged) &&
        (0 == ptDriver->ptDevice->i2c_irqsr_b.cmd_err))
  {
    if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
    {
      return DRV_TOUT;
    }

    fIsStartSeqAcknowledged = drv_I2C_IsTxStartSeqAcknowledged(ptDriver);
  }

  return DRV_OK;
}

/*!
 * \brief In master receive function, synchronizes the main context with the hardware context.
 */
__STATIC_INLINE DRV_STATUS_E
drv_I2C_WaitRxStartSeqAcknowledged(DRV_I2C_HANDLE_T* const ptDriver)
{
  bool fIsStartSeqAcknowledged = drv_I2C_IsRxStartSeqAcknowledged(ptDriver);

  while((false == fIsStartSeqAcknowledged) &&
        (0 == ptDriver->ptDevice->i2c_irqsr_b.cmd_err))
  {
    if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
    {
      return DRV_TOUT;
    }

    fIsStartSeqAcknowledged = drv_I2C_IsRxStartSeqAcknowledged(ptDriver);
  }

  return DRV_OK;
}

/*!
 * \brief In master transmit function, checks for the appropriate state machine state after start sequence
 */
__STATIC_INLINE bool
drv_I2C_IsTxStartSeqAcknowledged(DRV_I2C_HANDLE_T* const ptDriver)
{
  bool fStartSeqAcknowledged = true;
  volatile uint32_t ulCmdReg = ptDriver->ptDevice->i2c_cmd;
  uint32_t ulCommand = (ulCmdReg & i2c_app_i2c_cmd_cmd_Msk) >> i2c_app_i2c_cmd_cmd_Pos;

  if((DRV_I2C_COMMAND_CT != ulCommand) &&
     (DRV_I2C_COMMAND_CTC != ulCommand))
  {
    fStartSeqAcknowledged = false;
  }

  return fStartSeqAcknowledged;
}

/*!
 * \brief In master receive function, checks for the appropriate state machine state after start sequence
 */
__STATIC_INLINE bool
drv_I2C_IsRxStartSeqAcknowledged(DRV_I2C_HANDLE_T* const ptDriver)
{
  bool fStartSeqAcknowledged = true;
  volatile uint32_t ulCmdReg = ptDriver->ptDevice->i2c_cmd;
  uint32_t ulCommand = (ulCmdReg & i2c_app_i2c_cmd_cmd_Msk) >> i2c_app_i2c_cmd_cmd_Pos;
  uint32_t ulTSize = (ulCmdReg & i2c_app_i2c_cmd_tsize_Msk) >> i2c_app_i2c_cmd_tsize_Pos;

  if((DRV_I2C_COMMAND_CT != ulCommand) &&
     (DRV_I2C_COMMAND_CTC != ulCommand) &&
     (false == ((DRV_I2C_COMMAND_IDLE == ulCommand) && (0 == ulTSize))))
  {
    fStartSeqAcknowledged = false;
  }

  return fStartSeqAcknowledged;
}

/*!
 * \brief In master transmit function, selects the appropriate command, according to the continuous/not continuous mode
 * selected and the previous command, performed on the I2C line (detecting direction change).
 */
__STATIC_INLINE uint32_t
drv_I2C_SelectTxCommand(DRV_I2C_HANDLE_T* const ptDriver,
                        DRV_I2C_CONT_E eCont)
{
  uint32_t ulSelectedCommand = DRV_I2C_COMMAND_IDLE;

 /* If transmission continues previous master transmit, commands CT/CTC should be used instead of S_AC_T/S_AC_TC */
  if((DRV_I2C_RW_FLAG_WRITE == ptDriver->ptDevice->i2c_sr_b.nwr_aced) &&
     (1 == ptDriver->ptDevice->i2c_sr_b.started)                      &&
     (1 == ptDriver->ptDevice->i2c_sr_b.bus_master)                   &&
     (1 == ptDriver->ptDevice->i2c_sr_b.last_ac))
  {
    if(DRV_I2C_CONT_END == eCont)
    {
      ulSelectedCommand = DRV_I2C_COMMAND_CT;
    }
    else
    {
      ulSelectedCommand = DRV_I2C_COMMAND_CTC;
    }
  }
  else
  {
    if(DRV_I2C_CONT_END == eCont)
    {
      ulSelectedCommand = DRV_I2C_COMMAND_S_AC_T;
    }
    else
    {
      ulSelectedCommand = DRV_I2C_COMMAND_S_AC_TC;
    }
  }

  return ulSelectedCommand;
}

/*!
 * \brief In master receive function, selects the appropriate command, according to the continuous/not continuous mode
 * selected and the previous command, performed on the I2C line (detecting direction change).
 */
__STATIC_INLINE uint32_t
drv_I2C_SelectRxCommand(DRV_I2C_HANDLE_T* const ptDriver,
                        DRV_I2C_CONT_E eCont)
{
  uint32_t ulSelectedCommand = DRV_I2C_COMMAND_IDLE;

 /* If transmission continues previous MasterReceive(), commands CT/CTC should be used instead of S_AC_T/S_AC_TC */
  if((DRV_I2C_RW_FLAG_READ == ptDriver->ptDevice->i2c_sr_b.nwr_aced) &&
     (1 == ptDriver->ptDevice->i2c_sr_b.started)                     &&
     (1 == ptDriver->ptDevice->i2c_sr_b.bus_master)                  &&
     (1 == ptDriver->ptDevice->i2c_sr_b.last_ac))
  {
    if(DRV_I2C_CONT_END == eCont)
    {
      ulSelectedCommand = DRV_I2C_COMMAND_CT;
    }
    else
    {
      ulSelectedCommand = DRV_I2C_COMMAND_CTC;
    }
  }
  else
  {
    if(DRV_I2C_CONT_END == eCont)
    {
      ulSelectedCommand = DRV_I2C_COMMAND_S_AC_T;
    }
    else
    {
      ulSelectedCommand = DRV_I2C_COMMAND_S_AC_TC;
    }
  }

  return ulSelectedCommand;
}

/*!
 * \brief In master transmit and receive functions in polling mode, keeps calling the flush buffers function
 * until the transfer is complete, with a simple timeout check implemented for unblocking.
 */
__STATIC_INLINE DRV_STATUS_E
drv_I2C_TransferData(DRV_I2C_HANDLE_T* const ptDriver)
{
  while((ptDriver->MstBufferCounter != ptDriver->MstBufferSize) &&
        (0 == ptDriver->ptDevice->i2c_irqsr_b.cmd_err))
  {
    DRV_I2C_Flush_Buffers(ptDriver);

    if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
    {
      return DRV_TOUT;
    }
  }

  return DRV_OK;
}

/*!
 * \brief In master receive and transmit functions, checks for the appropriate state machine state after data transfer.
 */
__STATIC_INLINE DRV_STATUS_E
drv_I2C_WaitTransferEnd(DRV_I2C_HANDLE_T* const ptDriver)
{
  while((DRV_I2C_COMMAND_IDLE != ptDriver->ptDevice->i2c_cmd_b.cmd) &&
        (DRV_I2C_COMMAND_CTC != ptDriver->ptDevice->i2c_cmd_b.cmd)  &&
        (0 == ptDriver->ptDevice->i2c_irqsr_b.cmd_err))
  {
    if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
    {
      return DRV_TOUT;
    }
  }

  return DRV_OK;
}

/*!
 * \brief In master transmit and receive functions, waits for idle state machine state after processing a command
 */
__STATIC_INLINE DRV_STATUS_E
drv_I2C_WaitIdle(DRV_I2C_HANDLE_T* const ptDriver)
{
  while((DRV_I2C_COMMAND_IDLE != ptDriver->ptDevice->i2c_cmd_b.cmd) &&
        (0 == ptDriver->ptDevice->i2c_irqsr_b.cmd_err))
  {
    if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
    {
      return DRV_TOUT;
    }
  }

  return DRV_OK;
}

/*!
 * \brief In master transmit and receive functions, checks for errors and calls the corresponding callbacks accordingly
 */
__STATIC_INLINE DRV_STATUS_E
drv_I2C_CheckForError(DRV_I2C_HANDLE_T* const ptDriver)
{
  DRV_STATUS_E ulRslt = DRV_OK;

  if(1 == ptDriver->ptDevice->i2c_irqsr_b.cmd_err)
  {
    if(0 == ptDriver->ptDevice->i2c_sr_b.bus_master)
    {
      if(NULL != ptDriver->tConfiguration.fnArbitrationLostCallback)
      {
        ptDriver->tConfiguration.fnArbitrationLostCallback(ptDriver,
                                                           ptDriver->tConfiguration.pArbitrationLostCallbackHandle);
      }
    }
    else
    {
      if(NULL != ptDriver->tConfiguration.fnErrorCallback)
      {
        ptDriver->tConfiguration.fnErrorCallback(ptDriver,
                                                 ptDriver->tConfiguration.pErrorCallbackHandle);
      }
    }

    ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_cmd_err_Msk;

    ulRslt = DRV_ERROR;
  }

  return ulRslt;
}

__STATIC_INLINE bool
drv_I2c_IsTransferFinishedCorrectly(DRV_I2C_HANDLE_T* const ptDriver,
                                    uint32_t ulCmdOk)
{
  bool fTransferFinishedCorrectly = false;

  if((1 == ulCmdOk) &&
     ((DRV_I2C_COMMAND_CT   == ptDriver->ptDevice->i2c_cmd_b.cmd) ||
      (DRV_I2C_COMMAND_CTC  == ptDriver->ptDevice->i2c_cmd_b.cmd) ||
      (DRV_I2C_COMMAND_IDLE == ptDriver->ptDevice->i2c_cmd_b.cmd)))
  {
    fTransferFinishedCorrectly = true;
  }

  return fTransferFinishedCorrectly;
}

__STATIC_INLINE void
drv_I2C_IRQ_HandleMasterTransferEnd(DRV_I2C_HANDLE_T* const ptDriver,
                                    uint32_t ulCmdOk)
{
  if(drv_I2c_IsTransferFinishedCorrectly(ptDriver,
                                         ulCmdOk))
  {
    if(DRV_I2C_CONT_END == ptDriver->eCT)
    {
      ptDriver->ptDevice->i2c_cmd_b.cmd = DRV_I2C_COMMAND_STOP;
    }
    if(NULL != ptDriver->tConfiguration.fnMasterCompleteCallback)
    {
      ptDriver->tConfiguration.fnMasterCompleteCallback(ptDriver,
                                                        ptDriver->tConfiguration.pMasterCompleteCallbackHandle);
    }

    ptDriver->eRwFlagMaster = DRV_I2C_RW_FLAG_UNKNOWN;

    ptDriver->ptDevice->i2c_irqmsk_b.cmd_ok = 0;
    ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_cmd_ok_Msk;

    drv_I2C_ResetMstBuffer(ptDriver);
  }

  ptDriver->ptDevice->i2c_irqmsk_b.mfifo_req = 0;
  ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_mfifo_req_Msk;
}

__STATIC_INLINE void
drv_I2C_IRQ_CheckForCmdError(DRV_I2C_HANDLE_T* const ptDriver)
{
  if(ptDriver->ptDevice->i2c_irqsr_b.cmd_err)
  {
    ptDriver->ptDevice->i2c_cmd_b.cmd = DRV_I2C_COMMAND_STOP;

    if((NULL != ptDriver->tConfiguration.fnArbitrationLostCallback) &&
       (0 == ptDriver->ptDevice->i2c_sr_b.bus_master))
    {
      ptDriver->tConfiguration.fnArbitrationLostCallback(ptDriver,
                                                         ptDriver->tConfiguration.pArbitrationLostCallbackHandle);
    }
    else if(NULL != ptDriver->tConfiguration.fnErrorCallback)
    {
      ptDriver->tConfiguration.fnErrorCallback(ptDriver,
                                               ptDriver->tConfiguration.pErrorCallbackHandle);
    }

    ptDriver->ptDevice->i2c_irqmsk_b.cmd_err = 0;

    ptDriver->ptDevice->i2c_irqmsk_b.mfifo_req = 0;
    ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_cmd_err_Msk;
  }
}

__STATIC_INLINE void
drv_I2C_IRQ_CheckForFifoError(DRV_I2C_HANDLE_T* const ptDriver)
{
  if(ptDriver->ptDevice->i2c_irqsr_b.fifo_err)
  {
    if(NULL != ptDriver->tConfiguration.fnErrorCallback)
    {
      ptDriver->tConfiguration.fnErrorCallback(ptDriver,
                                               ptDriver->tConfiguration.pErrorCallbackHandle);
    }

    ptDriver->ptDevice->i2c_irqmsk_b.fifo_err = 0;
    ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_fifo_err_Msk;
  }
}

__STATIC_INLINE void
drv_I2C_IRQ_HandleSlaveTransferEnd(DRV_I2C_HANDLE_T* const ptDriver)
{
  drv_I2C_CallEndTransferCallback(ptDriver);

  ptDriver->ptDevice->i2c_irqmsk_b.sfifo_req = 0;
  ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_sfifo_req_Msk;

  ptDriver->ptDevice->i2c_irqmsk_b.sreq = 0;
  ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_sreq_Msk;

  ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_UNKNOWN;

  (void)drv_I2C_UpdateSlaveState(ptDriver, DRV_I2C_SLAVE_IDLE);

  /* If slave transmit was completed and slave FIFO is not empty, address acknowledging should be enabled, in order to
   * give the master opportunity to read the remaining data from the slave FIFO.
   * Master must read all the remaining data at one step before the last stop, because after stop condition the slave
   * address acknowledging is automatically reset and the slave can no longer be addressed. */
  if(0 < ptDriver->ptDevice->i2c_sr_b.sfifo_level)
  {
   ptDriver->ptDevice->i2c_scr_b.ac_start = 1;
  }

  drv_I2C_ResetSlvBuffer(ptDriver);
}

__STATIC_INLINE void
drv_I2C_CallEndTransferCallback(DRV_I2C_HANDLE_T* const ptDriver)
{
  if(DRV_I2C_SLAVE_XFER_ERROR == ptDriver->eSlaveState)
  {
    if(NULL != ptDriver->tConfiguration.fnErrorCallback)
    {
      ptDriver->tConfiguration.fnErrorCallback(ptDriver,
                                               ptDriver->tConfiguration.pErrorCallbackHandle);
    }
  }
  else
  {
    if(NULL != ptDriver->tConfiguration.fnSlaveCompleteCallback)
    {
      ptDriver->tConfiguration.fnSlaveCompleteCallback(ptDriver,
                                                       ptDriver->tConfiguration.pSlaveCompleteCallbackHandle);
    }
  }
}

/* Direction flag nwr is checked against the previously set slave transfer direction flag. */
__STATIC_INLINE bool
drv_I2C_IsTransferDirectionChanged(DRV_I2C_HANDLE_T* const ptDriver)
{
  bool fIsTransferDirectionChanged = false;
  /* The nwr flag is set from Master's point of view. */
  DRV_I2C_RW_FLAG_E eNwrDirection = DRV_I2C_RW_FLAG_READ;

  if(drv_I2C_IsNwrMasterWrite(ptDriver))
  {
    eNwrDirection = DRV_I2C_RW_FLAG_WRITE;
  }
  /* Direction is changing from Master Transmit Slave Receive to Master Receive Slave Transmit. */
  if((DRV_I2C_RW_FLAG_READ == ptDriver->eRwFlagSlave) &&
     (DRV_I2C_RW_FLAG_READ == eNwrDirection))
  {
    fIsTransferDirectionChanged = true;
  }
  /* Direction is changing from Master Receive Slave Transmit to Master Transmit Slave Receive. */
  else if((DRV_I2C_RW_FLAG_WRITE == ptDriver->eRwFlagSlave) &&
          (DRV_I2C_RW_FLAG_WRITE == eNwrDirection))
  {
    fIsTransferDirectionChanged = true;
  }

  return fIsTransferDirectionChanged;
}

/* Direction flag nwr is checked during a number of addressing cycles.*/
__STATIC_INLINE bool
drv_I2C_IsNwrMasterWrite(DRV_I2C_HANDLE_T* const ptDriver)
{
  bool fNwrMasterWrite = false;
  bool fAddressAcknowledged = false;
  uint32_t ulAddrAttemptCounter = 0;

  /* Clear the Slave Request flag. */
  ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_sreq_Msk;

  /* Read the nwr flag within a number of consecutive addressing attempts. If acknowledge is issued meanwhile,
   * when Master Tx, Slave Rx, direction is considered determined and the check is canceled. */
  while((DRV_I2C_RW_FLAG_WRITE == ptDriver->ptDevice->i2c_sr_b.nwr) &&
        (ulAddrAttemptCounter <= DRV_I2C_NWR_SAMPLING_ATTEMPTS) &&
        (false == fAddressAcknowledged))
  {
    /* Slave is addressed, counter is incremented and flag is reset. */
    if(1 == ptDriver->ptDevice->i2c_irqsr_b.sreq)
    {
      ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_sreq_Msk;
      ulAddrAttemptCounter++;
    }

    /* If slave has already acknowledged address, check must be canceled. */
    if(1 == ptDriver->ptDevice->i2c_sr_b.last_ac)
    {
      fAddressAcknowledged = true;
    }
  }
  if(((ulAddrAttemptCounter > DRV_I2C_NWR_SAMPLING_ATTEMPTS)||
      (fAddressAcknowledged))&&
     (DRV_I2C_RW_FLAG_WRITE == ptDriver->ptDevice->i2c_sr_b.nwr))
  {
    fNwrMasterWrite = true;
  }

  return fNwrMasterWrite;
}

__STATIC_INLINE void
drv_I2C_HandleSlaveTransferDirectionChange(DRV_I2C_HANDLE_T* const ptDriver)
{
  /* Save the size of transferred data before the direction change. */
  ptDriver->ulSlvTransferCounter = ptDriver->SlvBufferCounter - ptDriver->ptDevice->i2c_sr_b.sfifo_level;

  /* Clear slave FIFO */
  while(0 == ptDriver->ptDevice->i2c_sr_b.sfifo_empty)
  {
    ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_clr = 1;
  }

  if(DRV_I2C_RW_FLAG_WRITE == ptDriver->eRwFlagSlave)
  {
    /* Change slave direction flag */
    ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_READ;

    /* Update slave FIFO watermark according to the new transfer direction. */
    /* It will be adjusted additionally at the end of the IRQ inline handler */
    ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_wm = ptDriver->tConfiguration.eSlvRxFifoWatermark - 1;

    /* Call callback, if assigned. */
    if(NULL != ptDriver->tConfiguration.fnSlaveEventCallback)
    {
      ptDriver->tConfiguration.fnSlaveEventCallback(ptDriver,
                                                    ptDriver->tConfiguration.pSlaveEventCallbackHandle,
                                                    DRV_I2C_SLAVE_DIRECTION_CHANGE_TX_TO_RX);
    }
  }
  else
  {
    /* Change slave direction flag */
    ptDriver->eRwFlagSlave = DRV_I2C_RW_FLAG_WRITE;

    /* Update slave FIFO watermark according to the new transfer direction. */
    ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_wm = ptDriver->tConfiguration.eSlvTxFifoWatermark;

    /* Call callback, if assigned. */
    if(NULL != ptDriver->tConfiguration.fnSlaveEventCallback)
    {
      ptDriver->tConfiguration.fnSlaveEventCallback(ptDriver,
                                                    ptDriver->tConfiguration.pSlaveEventCallbackHandle,
                                                    DRV_I2C_SLAVE_DIRECTION_CHANGE_RX_TO_TX);
    }
  }

  if(NULL != ptDriver->SlvRvrsBuffer)
  {
    ptDriver->SlvBuffer = ptDriver->SlvRvrsBuffer;
    ptDriver->SlvBufferCounter = 0;
    ptDriver->SlvBufferSize = ptDriver->SlvRvrsBufferSize;
  }
}

__STATIC_INLINE bool
drv_I2C_IsSlaveTransferPrematurelyStopped(DRV_I2C_HANDLE_T* const ptDriver)
{
  bool fSlaveTransferPrematurelyStopped = false;

  /* bus_master flag is set, when the master is no more addressing the slave, e.g. Stop condition is generated. */
  if((1 == ptDriver->ptDevice->i2c_sr_b.bus_master)          &&
     (NULL != ptDriver->SlvBuffer)                           &&
     (ptDriver->SlvBufferCounter < ptDriver->SlvBufferSize))
  {
    fSlaveTransferPrematurelyStopped = true;
  }

  return fSlaveTransferPrematurelyStopped;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateUninitialized(DRV_I2C_HANDLE_T* const ptDriver,
                                      DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    /* Intentional fall through */
    case DRV_I2C_SLAVE_IDLE:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateIdle(DRV_I2C_HANDLE_T* const ptDriver,
                             DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    /* Intentional fall through */
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    case DRV_I2C_SLAVE_IDLE:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}
__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateXferError(DRV_I2C_HANDLE_T* const ptDriver,
                                     DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    {
      eRslt = DRV_BUSY;
      break;
    }
    case DRV_I2C_SLAVE_IDLE:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateWaitAddressing(DRV_I2C_HANDLE_T* const ptDriver,
                                       DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    {
      eRslt = DRV_BUSY;
      break;
    }
    /* Intentional fall through */
    case DRV_I2C_SLAVE_XFER_TO_BE_REVERSED:
    case DRV_I2C_SLAVE_ADDR_ACK_TO_BE_ENABLED:
    case DRV_I2C_SLAVE_IDLE:
    case DRV_I2C_SLAVE_XFER_ERROR:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateWaitStopOrRepeatedAddressing(DRV_I2C_HANDLE_T* const ptDriver,
                                                     DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    {
      eRslt = DRV_BUSY;
      break;
    }
    /* Intentional fall through */
    case DRV_I2C_SLAVE_XFER_TO_BE_REVERSED:
    case DRV_I2C_SLAVE_ADDR_ACK_TO_BE_ENABLED:
    case DRV_I2C_SLAVE_XFER_STOPPED:
    case DRV_I2C_SLAVE_IDLE:
    case DRV_I2C_SLAVE_XFER_ERROR:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateXferToBeReversed(DRV_I2C_HANDLE_T* const ptDriver,
                                         DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    {
      eRslt = DRV_BUSY;
      break;
    }
    /* Intentional fall through */
    case DRV_I2C_SLAVE_ADDR_ACK_TO_BE_ENABLED:
    case DRV_I2C_SLAVE_XFER_STOPPED:
    case DRV_I2C_SLAVE_IDLE:
    case DRV_I2C_SLAVE_XFER_ERROR:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateAddrAckToBeEnabled(DRV_I2C_HANDLE_T* const ptDriver,
                                           DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    {
      eRslt = DRV_BUSY;
      break;
    }
    /* Intentional fall through */
    case DRV_I2C_SLAVE_WAIT_STOP_OR_REPEATED_ADDRESSING:
    case DRV_I2C_SLAVE_ADDR_ACK_ENABLED:
    case DRV_I2C_SLAVE_IDLE:
    case DRV_I2C_SLAVE_XFER_ERROR:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateAddrAckEnabled(DRV_I2C_HANDLE_T* const ptDriver,
                                       DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    {
      eRslt = DRV_BUSY;
      break;
    }
    /* Intentional fall through */
    case DRV_I2C_SLAVE_ADDR_ACK_AND_SKIP_DIRECTION_CHECK:
    case DRV_I2C_SLAVE_WAIT_STOP_OR_REPEATED_ADDRESSING:
    case DRV_I2C_SLAVE_XFER_STOPPED:
    case DRV_I2C_SLAVE_IDLE:
    case DRV_I2C_SLAVE_XFER_ERROR:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateXferStopped(DRV_I2C_HANDLE_T* const ptDriver,
                                    DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    /* Intentional fall through */
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    case DRV_I2C_SLAVE_XFER_ERROR:
    case DRV_I2C_SLAVE_IDLE:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }
  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_HandleSlaveStateAddrAckAndSkipDirectionCheck(DRV_I2C_HANDLE_T* const ptDriver,
                                                     DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(eRequestedState)
  {
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    {
      eRslt = DRV_BUSY;
      break;
    }
    /* Intentional fall through */
    case DRV_I2C_SLAVE_WAIT_STOP_OR_REPEATED_ADDRESSING:
    case DRV_I2C_SLAVE_IDLE:
    case DRV_I2C_SLAVE_XFER_ERROR:
    {
      ptDriver->eSlaveState = eRequestedState;
      eRslt = DRV_OK;
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }
  return eRslt;
}

__STATIC_FORCEINLINE DRV_STATUS_E
drv_I2C_UpdateSlaveState(DRV_I2C_HANDLE_T* const ptDriver,
                         DRV_I2C_SLAVE_STATE_E eRequestedState)
{
  DRV_STATUS_E eRslt = DRV_OK;

  switch(ptDriver->eSlaveState)
  {
    case DRV_I2C_SLAVE_ADDR_ACK_AND_SKIP_DIRECTION_CHECK:
    {
      eRslt = drv_I2C_HandleSlaveStateAddrAckAndSkipDirectionCheck(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_ADDR_ACK_ENABLED:
    {
      eRslt = drv_I2C_HandleSlaveStateAddrAckEnabled(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_WAIT_ADDRESSING:
    {
      eRslt = drv_I2C_HandleSlaveStateWaitAddressing(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_WAIT_STOP_OR_REPEATED_ADDRESSING:
    {
      eRslt = drv_I2C_HandleSlaveStateWaitStopOrRepeatedAddressing(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_XFER_TO_BE_REVERSED:
    {
      eRslt = drv_I2C_HandleSlaveStateXferToBeReversed(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_ADDR_ACK_TO_BE_ENABLED:
    {
      eRslt = drv_I2C_HandleSlaveStateAddrAckToBeEnabled(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_XFER_STOPPED:
    {
      eRslt = drv_I2C_HandleSlaveStateXferStopped(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_IDLE:
    {
      eRslt = drv_I2C_HandleSlaveStateIdle(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_UNINITIALIZED:
    {
      eRslt = drv_I2C_HandleSlaveStateUninitialized(ptDriver, eRequestedState);
      break;
    }
    case DRV_I2C_SLAVE_XFER_ERROR:
    {
      eRslt = drv_I2C_HandleSlaveStateXferError(ptDriver, eRequestedState);
      break;
    }
    default:
    {
      eRslt = DRV_ERROR;
      break;
    }
  }

  return eRslt;
}

__STATIC_INLINE void
drv_I2C_HandleSlaveTransferPrematureStop(DRV_I2C_HANDLE_T* const ptDriver)
{
  /* Save the size of transferred data before the premature stop detection. */
  ptDriver->ulSlvTransferCounter = ptDriver->SlvBufferCounter - ptDriver->ptDevice->i2c_sr_b.sfifo_level;
  ptDriver->ptDevice->i2c_scr_b.ac_start = 0;

  if(DRV_I2C_RW_FLAG_WRITE == ptDriver->eRwFlagSlave)
  {
    if(NULL != ptDriver->tConfiguration.fnSlaveEventCallback)
    {
      ptDriver->tConfiguration.fnSlaveEventCallback(ptDriver,
                                                    ptDriver->tConfiguration.pSlaveEventCallbackHandle,
                                                    DRV_I2C_SLAVE_TX_PREMATURELY_STOPPED);
    }
  }
  else if(DRV_I2C_RW_FLAG_READ == ptDriver->eRwFlagSlave)
  {
    if(NULL != ptDriver->tConfiguration.fnSlaveEventCallback)
    {
      ptDriver->tConfiguration.fnSlaveEventCallback(ptDriver,
                                                    ptDriver->tConfiguration.pSlaveEventCallbackHandle,
                                                    DRV_I2C_SLAVE_RX_PREMATURELY_STOPPED);
    }
  }
}

__STATIC_INLINE void
drv_I2C_IRQ_HandleSlaveBufferNotPresent(DRV_I2C_HANDLE_T* const ptDriver)
{
  if(NULL != ptDriver->tConfiguration.fnErrorCallback)
  {
    ptDriver->tConfiguration.fnErrorCallback(ptDriver,
                                             ptDriver->tConfiguration.pErrorCallbackHandle);
  }

  ptDriver->ptDevice->i2c_irqmsk_b.sfifo_req = 0;
  ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_sfifo_req_Msk;

  ptDriver->ptDevice->i2c_irqmsk_b.sreq = 0;
  ptDriver->ptDevice->i2c_irqsr = i2c_app_i2c_irqsr_sreq_Msk;
}

__STATIC_INLINE void
drv_I2C_AdjustSlaveFifoWatermark(DRV_I2C_HANDLE_T* const ptDriver)
{
  uint32_t ulDiff = ptDriver->SlvBufferSize - ptDriver->SlvBufferCounter;

  if((ulDiff <= ptDriver->tConfiguration.eSlvRxFifoWatermark) &&
     (ulDiff > 0)                                             &&
     (DRV_I2C_RW_FLAG_READ == ptDriver->eRwFlagSlave))
  {
    ptDriver->ptDevice->i2c_sfifo_cr_b.sfifo_wm = ulDiff - 1;
  }
}

__STATIC_INLINE void
drv_I2C_AdjustMasterFifoWatermark(DRV_I2C_HANDLE_T* const ptDriver)
{
  uint32_t ulDiff = ptDriver->MstBufferSize - ptDriver->MstBufferCounter;

  if((ulDiff <= ptDriver->tConfiguration.eMstRxFifoWatermark) &&
     (ulDiff > 0)                                             &&
     (DRV_I2C_RW_FLAG_READ == ptDriver->eRwFlagMaster))
  {
    ptDriver->ptDevice->i2c_mfifo_cr_b.mfifo_wm = ulDiff - 1;
  }
}

__STATIC_INLINE void
drv_I2C_ResetMstBuffer(DRV_I2C_HANDLE_T* const ptDriver)
{
  ptDriver->MstBuffer = NULL;
  ptDriver->MstBufferSize = 0;
  ptDriver->MstBufferCounter = 0;
}

__STATIC_INLINE void
drv_I2C_ResetSlvBuffer(DRV_I2C_HANDLE_T* const ptDriver)
{
  ptDriver->SlvBuffer = NULL;
  ptDriver->SlvBufferSize = 0;
  ptDriver->SlvBufferCounter = 0;
}

__STATIC_INLINE void
drv_I2C_GenerateConditionalStop(DRV_I2C_HANDLE_T* const ptDriver)
{
  if((0 == ptDriver->ptDevice->i2c_sr_b.scl_state)  &&
     (1 == ptDriver->ptDevice->i2c_sr_b.bus_master) &&
     (1 == ptDriver->ptDevice->i2c_sr_b.last_ac))
  {
    ptDriver->ptDevice->i2c_cmd_b.cmd = DRV_I2C_COMMAND_STOP;
  }
}

/*!
 * This is the inline handler of the I2C driver. It is called by the generated ISRs.
 *
 * \memberof DRV_I2C_HANDLE_T
 * \param[in,out] eDeviceID The given handle of the drivers class
 */
__INLINE static void DRV_I2C_IRQ_Inline_Handler(DRV_I2C_DEVICE_ID_E const eDeviceID)
{
  uint32_t ulCmdOk = 0;
  uint32_t ulDeviceOffset = (uint32_t) eDeviceID - (uint32_t) DRV_I2C_DEVICE_ID_MIN;
  DRV_I2C_HANDLE_T* ptDriver = s_apHandleAddressTable[ulDeviceOffset];

#ifndef NDEBUG
  if(ptDriver == 0 || ptDriver->ptDevice != s_apDeviceAddressTable[ulDeviceOffset])
  {
    ((DRV_I2C_DEVICE_T* const) s_apDeviceAddressTable[ulDeviceOffset])->i2c_mcr = 0;
    DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[ulDeviceOffset]);
    return;
  }
#endif

  DRV_I2C_Flush_Buffers(ptDriver);

  if(NULL != ptDriver->MstBuffer)
  {
    ulCmdOk = ptDriver->ptDevice->i2c_irqsr_b.cmd_ok;

    if(ptDriver->MstBufferCounter == ptDriver->MstBufferSize)
    {
      drv_I2C_IRQ_HandleMasterTransferEnd(ptDriver,
                                          ulCmdOk);
    }
  }
  else
  {
    if(1 == ptDriver->ptDevice->i2c_irqmsk_b.cmd_ok)
    {
      ptDriver->ptDevice->i2c_irqmsk_b.cmd_ok = 0;
    }
  }

  drv_I2C_IRQ_CheckForCmdError(ptDriver);

  drv_I2C_IRQ_CheckForFifoError(ptDriver);

  if(NULL != ptDriver->SlvBuffer)
  {
    /* Enable slave FIFO request Interrupts only after the slave has been addressed */
    if(1 == ptDriver->ptDevice->i2c_irqsr_b.sreq)
    {
      ptDriver->ptDevice->i2c_irqmsk_b.sfifo_req = 1;
    }

    if(ptDriver->SlvBufferCounter >= ptDriver->SlvBufferSize)
    {
      drv_I2C_IRQ_HandleSlaveTransferEnd(ptDriver);
    }
  }
  else
  {
    if((1 == ptDriver->ptDevice->i2c_irqsr_b.sfifo_req) ||
       (1 == ptDriver->ptDevice->i2c_irqsr_b.sreq))
    {
      drv_I2C_IRQ_HandleSlaveBufferNotPresent(ptDriver);
    }
  }

  drv_I2C_AdjustSlaveFifoWatermark(ptDriver);

  drv_I2C_AdjustMasterFifoWatermark(ptDriver);

  /* If ISR is interrupted by higher priority IRQ, it is possible for cmd_ok to preempt the last mfifo_req,
   * and if cleared in the last ISR line, would remain 0 in next ISR calls and never been processed.
   * For this reason, cmd_ok flag is excluded from the flag clearing below. */
  ptDriver->ptDevice->i2c_irqsr = DRV_I2C_CLEAR_IRQSR_FLAGS_EXCEPT_CMD_OK;
}

/*!
 * The generator define for generating IRQ handler source code.
 */
#define  DRV_I2C_IRQHandler_Generator(id, _) DRV_Default_IRQHandler_Function_Generator(DRV_I2C_IRQ_HANDLER ## id ,DRV_I2C_IRQ_Inline_Handler,DRV_I2C_DEVICE_ID_I2C ## id)

/*!
 * The define to where the IRQ handler code shall be generated.
 */
/*lint -save -e123 */
DRV_DEF_REPEAT_EVAL(DRV_I2C_DEVICE_COUNT, DRV_I2C_IRQHandler_Generator, ~)
/*lint -restore */

/*! \} *//* End of group I2C */

#endif /* DRV_I2C_MODULE_ENABLED */
