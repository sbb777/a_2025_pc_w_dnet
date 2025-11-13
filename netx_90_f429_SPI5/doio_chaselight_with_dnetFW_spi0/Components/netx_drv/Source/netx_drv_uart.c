/*!************************************************************************//*!
 * \file    netx_drv_uart.c
 * \brief   UART peripheral module driver.
 * $Revision: 11353 $
 * $Date: 2024-05-20 18:54:32 +0300 (Mon, 20 May 2024) $
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

/*!***************************************************************************/
/*  Includes                                                                 */
/*!***************************************************************************/
#include "netx_drv.h"
#ifdef DRV_UART_MODULE_ENABLED /* NOTE: needs 'PREDEFINED = DRV_UART_MODULE_ENABLED' in Doxygen-Config-File if not enabled */
#include <string.h>

/*!
 * \addtogroup UART
 * \{
 * \brief The UART driver, defined by DRV_UART_HANDLE_T
 */

/*!
 * \brief Table of the device addresses.
 *
 * Used to identify the device addresses by the device id.
 */
static DRV_UART_DEVICE_T * const s_apDeviceAddressTable[DRV_UART_DEVICE_COUNT] = DRV_UART_DEVICE_LIST;
/*!
 * \brief Table of the device associated dmac channels.
 *
 * Used to identify the device dmac channels by the device id.
 */
static DRV_DMAC_PERIPHERAL_E const s_apDeviceDmacTable[DRV_UART_DEVICE_COUNT] = DRV_UART_DEVICE_DMA_LIST;
/*!
 * \brief Table of the IRQ vector numbers.
 *
 * Used to identify the interrupt channels by the device id.
 */
static IRQn_Type const s_apHandleIRQnTable[DRV_UART_DEVICE_COUNT] = DRV_UART_DEVICE_IRQ_LIST;
/*!
 * \brief Used for mapping the handle to an interrupt.
 *
 * Threadsafe and reentrant because its is only written in normal context an used in interrupt context of the specific interrupt.
 */
static DRV_UART_HANDLE_T * s_apHandleAddressTable[DRV_UART_DEVICE_COUNT] = { 0 };

/*!***************************************************************************/
/*  enum Definitions                                                         */
/*!***************************************************************************/

/*!
 * \brief Baudrate limit value for oversampling 16
 *
 * Baudrates up to 3.125 MBaud are achieved with oversampling 16
 * Baudrates over 3.125 MBaud need oversampling 8 */
#define DRV_UART_MAX_BAUDRATE_AT_OVERSAMPL16 DRV_UART_BAUDRATE_3125000

#ifndef DRV_HANDLE_CHECK_INACTIVE
/*!
 * Define for checking the consistency of the handle or static representation of the driver.
 */
#define DRV_HANDLE_CHECK(handle)\
  if((handle)==0){ \
    return DRV_ERROR_PARAM; \
  }\
  if((handle)->ptDevice==0){ \
    return DRV_ERROR_PARAM; \
  }
#else
/*!
 * If the checking is deactivated, the source line will be replaced with void.
 */
#define DRV_HANDLE_CHECK(handle)
#endif


/*!***************************************************************************/
/*  Static function Prototypes                                               */
/*!***************************************************************************/

__STATIC_INLINE DRV_STATUS_E drv_UART_Init_VerifyParameters(DRV_UART_HANDLE_T* ptDriver);

__STATIC_INLINE void drv_UART_Init_SetUninitializedParametersToDefault(DRV_UART_HANDLE_T* ptDriver);

__STATIC_INLINE void drv_UART_Init_CalculateBaudDiv(DRV_UART_HANDLE_T* ptDriver);

__STATIC_INLINE void drv_UART_Init_SetLineControlParameters(DRV_UART_HANDLE_T* ptDriver);

/*!***************************************************************************/
/* Definitions                                                               */
/*!***************************************************************************/

/*!
 * This callback is used in dma operation mode. It is registered in the dmac api to get informed if
 * the dma finished copying.
 */
static void DRV_UART_Flush_DMA_Callback_Rx(void * ptDriverHandle, DRV_UART_HANDLE_T * const ptDriver)
{
  ptDriver->RxBuffer = 0;
  ptDriver->RxBufferSize = 0;
  if(ptDriver->tConfiguration.fnRxCompleteCallback != 0)
  {
    ptDriver->tConfiguration.fnRxCompleteCallback(ptDriver, ptDriver->tConfiguration.pRxCompleteCallbackHandle);
  }
}

/*!
 * This callback is used in dma operation mode. It is registered in the dmac api to get informed if
 * the dma finished copying.
 */
static void DRV_UART_Flush_DMA_Callback_Tx(void * ptDriverHandle, DRV_UART_HANDLE_T * const ptDriver)
{
  ptDriver->TxBuffer = 0;
  ptDriver->TxBufferSize = 0;
  if(ptDriver->tConfiguration.fnTxCompleteCallback != 0)
  {
    ptDriver->tConfiguration.fnTxCompleteCallback(ptDriver, ptDriver->tConfiguration.pTxCompleteCallbackHandle);
  }
}

/*!
 * The function takes a DRV_UART_HANDLE_T pointer which contains a DRV_UART_CONFIGURATION_T structure. Those structure contains the configuration/initialization parameters of the device.
 * While most attributes have valid default behavior, it is necessary to configure at least the DRV_UART_DEVICE_ID_E and some others.
 *
 * The UARTs initialize function forces the initialization of the UART peripheral and the handle.
 * The driver lock is set and the given parameters in the DRV_UART_CONFIGURATION_T structure are checked if there are parameter combinations not feasible.
 * If everything is ok, the Buffers will be reset and the configuration registers are written, regarding the given attributes. At last the lock will be released.
 *
 * In case of an error during initialization, the function returns an error value, but the lock for the handle, will not be released.
 * Together with the return value, this will ensure that the driver can only be interacted with if it has been correctly initialized.
 * Since the lock is initialized when this function is called, this function can still be called again with other parameters after a failed initialization.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[out] ptDriver The ptDriver to be
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 *          DRV_NIMPL
 */
DRV_STATUS_E DRV_UART_Init(DRV_UART_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRet = DRV_OK;
  if(ptDriver == 0)
  {
    return DRV_ERROR_PARAM;
  }
  ptDriver->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptDriver);

  eRet = drv_UART_Init_VerifyParameters(ptDriver);
  if(DRV_OK != eRet)
  {
    return eRet;
  }

  drv_UART_Init_SetUninitializedParametersToDefault(ptDriver);

  // Configure DMA (channel, IRQ, priority and so on)
  if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
  {
    if(ptDriver->tConfiguration.ptSequencerTx == 0 || ptDriver->tConfiguration.ptSequencerRx == 0)
    {
      return DRV_ERROR_PARAM;
    }
    ptDriver->ptDevice->uartrxiflsel_b.RXDMA = 1;
    ptDriver->ptDevice->uarttxiflsel_b.TXDMA = 1;

    memset(&(ptDriver->tConfiguration.ptSequencerRx->tConfiguration), 0, sizeof(ptDriver->tConfiguration.ptSequencerRx->tConfiguration));
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eDeviceID = ptDriver->tConfiguration.eDMARx;
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.ePeripheralSource =
      (DRV_DMAC_PERIPHERAL_E) ((uint32_t) s_apDeviceDmacTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN] + 0u);
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.ePeripheralDest = DRV_DMAC_PERIPHERAL_MEMORY;
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eIncrementationSource = DRV_DMAC_INCREMENTATION_INACTIVE;
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eIncrementationDest = DRV_DMAC_INCREMENTATION_ACTIVE;
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eTransferWidthSource = DRV_DMAC_TRANSFER_WIDTH_8b;
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eTransferWidthDest = DRV_DMAC_TRANSFER_WIDTH_8b;
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.fCallbackComplete = (DRV_CALLBACK_F) DRV_UART_Flush_DMA_Callback_Rx;
    ptDriver->tConfiguration.ptSequencerRx->tConfiguration.ptCallbackHandleComplete = ptDriver;

    memset(&(ptDriver->tConfiguration.ptSequencerTx->tConfiguration), 0, sizeof(ptDriver->tConfiguration.ptSequencerTx->tConfiguration));
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eDeviceID = ptDriver->tConfiguration.eDMATx;
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.ePeripheralSource = DRV_DMAC_PERIPHERAL_MEMORY;
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.ePeripheralDest =
      (DRV_DMAC_PERIPHERAL_E) ((uint32_t) s_apDeviceDmacTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN] + 1u);
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eIncrementationSource = DRV_DMAC_INCREMENTATION_ACTIVE;
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eIncrementationDest = DRV_DMAC_INCREMENTATION_INACTIVE;
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eTransferWidthSource = DRV_DMAC_TRANSFER_WIDTH_8b;
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eTransferWidthDest = DRV_DMAC_TRANSFER_WIDTH_8b;
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.fCallbackComplete = (DRV_CALLBACK_F) DRV_UART_Flush_DMA_Callback_Tx;
    ptDriver->tConfiguration.ptSequencerTx->tConfiguration.ptCallbackHandleComplete = ptDriver;

    DRV_STATUS_E ret;
    if(DRV_OK != (ret = DRV_DMAC_Init(ptDriver->tConfiguration.ptSequencerTx)))
    {
      return ret;
    }
    if(DRV_OK != (ret = DRV_DMAC_Init(ptDriver->tConfiguration.ptSequencerRx)))
    {
      return ret;
    }
  }
  ptDriver->ptDevice->uarttxiflsel_b.TXIFLSEL = ptDriver->tConfiguration.eTxFifoWatermark;
  ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->tConfiguration.eRxFifoWatermark;
  s_apHandleAddressTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN] = ptDriver;
  ptDriver->TxBuffer = 0;
  ptDriver->TxBufferSize = 0;
  ptDriver->TxBufferCounter = 0;
  ptDriver->RxBuffer = 0;
  ptDriver->RxBufferSize = 0;
  ptDriver->RxBufferCounter = 0;
  if(ptDriver->tConfiguration.ulDriverTimeout == 0)
  {
    ptDriver->tConfiguration.ulDriverTimeout = 0xFFFFFFFFul;
  }

  /* First of all disable everything */
  ptDriver->ptDevice->uartcr = 0u;

  /* Calculate the bauddiv and load the values in the corresponding uartlcr registers. */
  drv_UART_Init_CalculateBaudDiv(ptDriver);

  ptDriver->ptDevice->uartcr_b.TX_RX_LOOP = ptDriver->tConfiguration.eLoopBack;

  /* Set UART Line control parameters in the uartlcr_h register : */
  drv_UART_Init_SetLineControlParameters(ptDriver);

  /* Set TX-Driver to enabled */
  ptDriver->ptDevice->uartdrvout = (ptDriver->tConfiguration.eTxMode ^ 0x1ul) & DRV_UART_TX_MODE_MASK;

  /* Finally enable the UART */
  ptDriver->ptDevice->uartcr_b.uartEN = 1;

  /* Configure nvic (activate IRQ, define priority and so on) */
  if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    if(ptDriver->tConfiguration.fnRxCallback != 0)  // enable IRQ only if we have 'fnRxCallback' defined
    {
      ptDriver->ptDevice->uartcr_b.RIE = 1;
      ptDriver->ptDevice->uartcr_b.RTIE = 1;
    }
    NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
    NVIC_EnableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
  }
  DRV_UNLOCK(ptDriver);
  return DRV_OK;
}

/*!
 * The deinitialize function is supposed to reset the handle, the device and disable the interrupt signaling.
 * The handle might be used afterwards again for initializing a UART device driver handle.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The ptDriver to be initialized
 * \return  DRV_OK
 *          DRV_LOCKED
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_UART_DeInit(DRV_UART_HANDLE_T * const ptDriver)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
  DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
  ptDriver->ptDevice->uartcr_b.MSIE = 0u;
  ptDriver->ptDevice->uartcr_b.RIE = 0u;
  ptDriver->ptDevice->uartcr_b.RTIE = 0u;
  ptDriver->ptDevice->uartcr_b.TIE = 0u;
  ptDriver->ptDevice->uartiir = 0xFFFFFFFFul;
  *ptDriver = (DRV_UART_HANDLE_T ) { 0 };
  return DRV_OK;
}

/*!
 * \brief This function shall flush the software and hardware buffers of the device.
 *
 * This function is the hardware closest layer of the uart driver.
 * The handle contains some variables defining the software buffer state. This state shall be equalized in this function. The transmit buffer is transfered to
 * the transmit fifo of the device from the actual counter position to the given size. The receive fifo of the device is written to the current counter position
 * until the given size is reached. If there is no buffer given, the function will transmit the dummy pattern specified in the attributes, and receives what is
 * contained in the receive fifo but discard it.
 *
 *
 * \private
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return DRV_OK always, because there is no error condition occurring
 */
__STATIC_INLINE void DRV_UART_Flush_Buffers(DRV_UART_HANDLE_T * const ptDriver)
{
  if(ptDriver->RxBuffer != 0)
  {
    while(ptDriver->ptDevice->uartfr_b.RXFE != 1u)
    {
      if(ptDriver->RxBufferCounter != ptDriver->RxBufferSize)
      {
        ((uint8_t*) ptDriver->RxBuffer)[ptDriver->RxBufferCounter] = ptDriver->ptDevice->uartdr; /* Get the received byte */
        ptDriver->RxBufferCounter++;
      }
      else
      {
        break;
      }
    }
  }
  else
  { /* if there is no buffer defined */
    if(ptDriver->tConfiguration.fnRxCallback != 0) /* but a receive callback is defined */
    {
      ptDriver->RxBufferSize = 16;                      /* set to DRV_UART_WATERMARK_MAX */
      while(ptDriver->ptDevice->uartfr_b.RXFE != 1u)    /* copy into RxBufferStatic[] */
      {
        if(ptDriver->RxBufferCounter != ptDriver->RxBufferSize)
        {
          ptDriver->RxBufferStatic[ptDriver->RxBufferCounter] = ptDriver->ptDevice->uartdr; /* Get the received byte */
          ptDriver->RxBufferCounter++;
        }
        else
        {
          break;
        }
      }
      if(ptDriver->RxBufferCounter > 0)
      {
        ptDriver->tConfiguration.fnRxCallback(ptDriver, ptDriver->tConfiguration.pRxCallbackHandle);
        // only in irq mode relevant
        ptDriver->ptDevice->uartcr_b.RTIE = 1;
        ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->tConfiguration.eRxFifoWatermark;
      }
      ptDriver->RxBuffer = 0;
      ptDriver->RxBufferSize = 0;
      ptDriver->RxBufferCounter = 0;
    }
  }
  if(ptDriver->TxBuffer != 0)
  {
    while(ptDriver->ptDevice->uartfr_b.TXFF == 0u)
    {
      if(ptDriver->TxBufferCounter < ptDriver->TxBufferSize)
      {
        ptDriver->ptDevice->uartdr = ((uint8_t*) ptDriver->TxBuffer)[ptDriver->TxBufferCounter];
        ptDriver->TxBufferCounter++;
      }
      else
      {
        break;
      }
    }
  }

}

/*!
 * The transmit function is associated with the job to transmit the given data of the given size via the
 * UART initialized on the given context.
 *
 * In polling mode it is blocking and in irq and dma it is non blocking.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] data The pointer to the data to be transmitted
 * \param[in] size The size of the data buffers
 * \return  DRV_OK
 *          DRV_LOCKED
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_UART_Transmit(DRV_UART_HANDLE_T * const ptDriver, uint8_t const * const data, size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  if((NULL == data) ||
     (0 == size))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  DRV_STATUS_E ret = DRV_ERROR;
  uint32_t ulExclusiveRead = 0xFFFFFFFFul;
  ptDriver->ullFrameStartTick = 0;

  if((NULL != ptDriver->TxBuffer)  ||
     (0 != ptDriver->TxBufferSize))
  {
    ret = DRV_BUSY;
  }
  else
  {
    ptDriver->TxBufferCounter = 0;
    ptDriver->TxBufferSize = size;
    ptDriver->TxBuffer = (void*) data;
    if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      while(ptDriver->TxBufferCounter != ptDriver->TxBufferSize)
      {
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          ret = DRV_TOUT;
          break;
        }
        DRV_UART_Flush_Buffers(ptDriver);
      }

      if(DRV_TOUT != ret)
      {
        if(ptDriver->tConfiguration.fnTxCompleteCallback != 0)
        {
          ptDriver->tConfiguration.fnTxCompleteCallback(ptDriver, ptDriver->tConfiguration.pTxCompleteCallbackHandle);
        }
        ret = DRV_OK;
      }

      ptDriver->TxBuffer = NULL;
      ptDriver->TxBufferSize = 0;
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      do
      {
        ulExclusiveRead = __LDREXW((volatile uint32_t*) &ptDriver->ptDevice->uartcr);
        if(ptDriver->TxBufferSize - ptDriver->TxBufferCounter < ptDriver->tConfiguration.eTxFifoWatermark)
        {
          ptDriver->ptDevice->uarttxiflsel_b.TXIFLSEL = ptDriver->TxBufferSize - ptDriver->TxBufferCounter;
        }
        else
        {
          ptDriver->ptDevice->uarttxiflsel_b.TXIFLSEL = ptDriver->tConfiguration.eTxFifoWatermark;
        }
        ulExclusiveRead |= uart_uartcr_TIE_Msk;
      } while(__STREXW((uint32_t) ulExclusiveRead, (volatile uint32_t*) &ptDriver->ptDevice->uartcr));
      ret = DRV_OK;
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      if(ptDriver->tConfiguration.ptSequencerTx == 0 || ptDriver->tConfiguration.ptSequencerRx == 0)
      {
        ret = DRV_ERROR_PARAM;
      }
      else
      {
        ret = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerTx, ptDriver->TxBuffer, (void * const ) &ptDriver->ptDevice->uartdr,
          ptDriver->TxBufferSize);
      }
    }
    else
    {
      ptDriver->TxBuffer = NULL;
      ptDriver->TxBufferCounter = 0;
      ptDriver->TxBufferSize = 0;
      ret = DRV_ERROR_PARAM;
    }
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The receive function is associated with the job to receive data of the given size via the
 * UART initialized on the given context to the given position.
 *
 * In polling mode it is blocking and in irq and dma it is non blocking.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] data The pointer to the data to be received
 * \param[in] size The size of the data buffers
 * \return  DRV_OK
 *          DRV_LOCKED
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_UART_Receive(DRV_UART_HANDLE_T * const ptDriver, uint8_t * const data, size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  if((NULL == data) ||
     (0 == size))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  DRV_STATUS_E ret = DRV_ERROR;
  uint32_t ulExclusiveRead = 0xFFFFFFFFul;
  ptDriver->ullFrameStartTick = 0;

  if((NULL != ptDriver->RxBuffer) ||
     (0 != ptDriver->RxBufferSize))
  {
    ret = DRV_BUSY;
  }
  else
  {
    ptDriver->RxBufferCounter = 0;
    ptDriver->RxBufferSize = size;
    ptDriver->RxBuffer = data;
    ptDriver->ptDevice->uartrsr = 0x0fu;
    if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      while(ptDriver->RxBufferCounter != ptDriver->RxBufferSize)
      {
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          ret = DRV_TOUT;
          break;
        }
        DRV_UART_Flush_Buffers(ptDriver);
      }

      if(DRV_TOUT != ret)
      {
        if(ptDriver->tConfiguration.fnRxCompleteCallback != 0)
        {
          ptDriver->tConfiguration.fnRxCompleteCallback(ptDriver, ptDriver->tConfiguration.pRxCompleteCallbackHandle);
        }
        ret = DRV_OK;
      }

      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferSize = 0;
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      do
      {
        ulExclusiveRead = __LDREXW((uint32_t*) &ptDriver->ptDevice->uartcr);
        if(ptDriver->RxBufferSize - ptDriver->RxBufferCounter < ptDriver->tConfiguration.eRxFifoWatermark)
        {
          ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->RxBufferSize - ptDriver->RxBufferCounter;
        }
        else
        {
          ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->tConfiguration.eRxFifoWatermark;
        }
        ulExclusiveRead |= uart_uartcr_RIE_Msk | uart_uartcr_RTIE_Msk;
      } while(__STREXW((uint32_t) ulExclusiveRead, (uint32_t*) &ptDriver->ptDevice->uartcr));
      ret = DRV_OK;
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      if(ptDriver->tConfiguration.ptSequencerTx == 0 || ptDriver->tConfiguration.ptSequencerRx == 0)
      {
        ret = DRV_ERROR_PARAM;
      }
      else
      {
        ret = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerRx, (void * const ) &ptDriver->ptDevice->uartdr, ptDriver->RxBuffer,
          ptDriver->RxBufferSize);
      }
    }
    else
    {
      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferCounter = 0;
      ptDriver->RxBufferSize = 0;
      ret = DRV_ERROR_PARAM;
    }
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The transmit receive function is associated with the job to transmit and receive the given data of the given size via the
 * UART initialized on the given context and receive the data to the given position.
 *
 * In polling mode it is blocking and in irq and dma it is non blocking.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] txData The pointer to the data to be transmitted
 * \param[out] rxData The pointer to the data to be received
 * \param[in] size The size of the data buffers
 * \return  DRV_OK
 *          DRV_LOCKED
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_UART_TransmitReceive(DRV_UART_HANDLE_T * const ptDriver, uint8_t * const txData, uint8_t * const rxData, size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  if((NULL == txData) ||
     (NULL == rxData) ||
     (0 == size))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  DRV_STATUS_E ret = DRV_NIMPL;
  uint32_t ulExclusiveRead = 0xFFFFFFFFul;
  ptDriver->ullFrameStartTick = 0;

  if((NULL != ptDriver->TxBuffer)   ||
     (0 != ptDriver->TxBufferSize)  ||
     (NULL != ptDriver->RxBuffer)   ||
     (0 != ptDriver->RxBufferSize))
  {
    ret = DRV_BUSY;
  }
  else
  {
    ptDriver->TxBufferCounter = 0;
    ptDriver->TxBufferSize = size;
    ptDriver->TxBuffer = txData;
    ptDriver->RxBufferCounter = 0;
    ptDriver->RxBufferSize = size;
    ptDriver->RxBuffer = rxData;
    ptDriver->ptDevice->uartrsr = 0x0fu;
    if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      while(ptDriver->RxBufferCounter != ptDriver->RxBufferSize && ptDriver->TxBufferCounter != ptDriver->TxBufferSize)
      {
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          ret = DRV_TOUT;
          break;
        }
        DRV_UART_Flush_Buffers(ptDriver);
      }

      if(DRV_TOUT != ret)
      {
        if(ptDriver->tConfiguration.fnTxCompleteCallback != 0)
        {
          ptDriver->tConfiguration.fnTxCompleteCallback(ptDriver, ptDriver->tConfiguration.pTxCompleteCallbackHandle);
        }
        if(ptDriver->tConfiguration.fnRxCompleteCallback != 0)
        {
          ptDriver->tConfiguration.fnRxCompleteCallback(ptDriver, ptDriver->tConfiguration.pRxCompleteCallbackHandle);
        }
        ret = DRV_OK;
      }

      ptDriver->TxBuffer = 0;
      ptDriver->TxBufferSize = 0;
      ptDriver->RxBuffer = 0;
      ptDriver->RxBufferSize = 0;
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      do
      {
        ulExclusiveRead = __LDREXW((uint32_t*) &ptDriver->ptDevice->uartcr);
        if(ptDriver->RxBufferSize - ptDriver->RxBufferCounter < ptDriver->tConfiguration.eRxFifoWatermark)
        {
          ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->RxBufferSize - ptDriver->RxBufferCounter;
        }
        else
        {
          ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->tConfiguration.eRxFifoWatermark;
        }
        ulExclusiveRead |= uart_uartcr_TIE_Msk;
      } while(__STREXW((uint32_t) ulExclusiveRead, (uint32_t*) &ptDriver->ptDevice->uartcr));
      do
      {
        ulExclusiveRead = __LDREXW((uint32_t*) &ptDriver->ptDevice->uartcr);
        if(ptDriver->TxBufferSize - ptDriver->TxBufferCounter < ptDriver->tConfiguration.eTxFifoWatermark)
        {
          ptDriver->ptDevice->uarttxiflsel_b.TXIFLSEL = ptDriver->TxBufferSize - ptDriver->TxBufferCounter;
        }
        else
        {
          ptDriver->ptDevice->uarttxiflsel_b.TXIFLSEL = ptDriver->tConfiguration.eTxFifoWatermark;
        }
        ulExclusiveRead |= uart_uartcr_RIE_Msk | uart_uartcr_RTIE_Msk;
      } while(__STREXW((uint32_t) ulExclusiveRead, (uint32_t*) &ptDriver->ptDevice->uartcr));
      ret = DRV_OK;
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      if(ptDriver->tConfiguration.ptSequencerTx == 0 || ptDriver->tConfiguration.ptSequencerRx == 0)
      {
        ret = DRV_ERROR_PARAM;
      }
      else
      {
        if((ret = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerTx, ptDriver->TxBuffer, (void * const ) &ptDriver->ptDevice->uartdr,
          ptDriver->TxBufferSize))
          != DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerRx, (void * const ) &ptDriver->ptDevice->uartdr, ptDriver->RxBuffer,
            ptDriver->RxBufferSize))
        {
          ret = DRV_ERROR;
        }
      }
    }
    else
    {
      ptDriver->TxBuffer = 0;
      ptDriver->TxBufferCounter = 0;
      ptDriver->TxBufferSize = 0;
      ptDriver->RxBuffer = 0;
      ptDriver->RxBufferCounter = 0;
      ptDriver->RxBufferSize = 0;
      ret = DRV_ERROR_PARAM;
    }
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The put character method will put a character down to the device so that it is transmitted.
 * This is possible if there is no buffer content to be transmitted.
 *
 * This method is blocking.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] cData The character to be transmitted
 * \return  DRV_OK
 *          DRV_LOCKED
 *          DRV_ERROR_PARAM
 *          DRV_ERROR
 */
DRV_STATUS_E DRV_UART_PutChar(DRV_UART_HANDLE_T * const ptDriver, unsigned char const cData)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  DRV_STATUS_E ret = DRV_ERROR;
  ptDriver->ullFrameStartTick = 0;

  if(ptDriver->TxBufferSize > 0 && ptDriver->TxBufferCounter != ptDriver->TxBufferSize)
  {
    ret = DRV_BUSY;
  }
  else
  {
    while(ptDriver->ptDevice->uartfr_b.TXFF != 0)
    {
      if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
      {
        DRV_UNLOCK(ptDriver);
        return DRV_TOUT;
      }
    }
    ptDriver->ptDevice->uartdr = cData;
    if(ptDriver->ptDevice->uartrsr == 0)
    {
      ret = DRV_OK;
    }
    else
    {
      ret = DRV_ERROR;
    }
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The get character method will get the next character from the device.
 * This is possible if there is no buffer content to be received.
 *
 * This method is blocking.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] pcData The pointer to the character to be received
 * \return  DRV_OK
 *          DRV_LOCKED
 *          DRV_ERROR_PARAM
 *          DRV_ERROR
 */
DRV_STATUS_E DRV_UART_GetChar(DRV_UART_HANDLE_T * const ptDriver, unsigned char * const pcData)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(pcData == 0)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  DRV_STATUS_E ret = DRV_ERROR;
  ptDriver->ullFrameStartTick = 0;

  if(ptDriver->RxBufferSize > 0 && ptDriver->RxBufferCounter != ptDriver->RxBufferSize)
  {
    ret = DRV_BUSY;
  }
  else
  {
    while(ptDriver->ptDevice->uartfr_b.RXFE != 0)
    {
      if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
      {
        DRV_UNLOCK(ptDriver);
        return DRV_TOUT;
      }
    }
    *pcData = ptDriver->ptDevice->uartdr;
    if(ptDriver->ptDevice->uartrsr == 0)
    {
      ret = DRV_OK;
    }
    else
    {
      ret = DRV_ERROR;
    }
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The abort method will abort a uart transmission in progress.
 *
 * If one uses the RxCallback, the interrupt conditions will be masked out.
 * One has to actively receive again afterwards to enable it.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return  DRV_OK
 *          DRV_LOCKED
 *          DRV_ERROR_PARAM
 *          DRV_ERROR
 *          DRV_NSUPP
 */
DRV_STATUS_E DRV_UART_Abort(DRV_UART_HANDLE_T * const ptDriver)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
  {
    ret = DRV_NSUPP;
  }
  else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    ptDriver->ptDevice->uartrsr = (uint32_t) -1;
    DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
    DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
    uint32_t ulSIREN = ptDriver->ptDevice->uartcr_b.SIREN;
    ptDriver->ptDevice->uartcr_b.RTIE = 0;
    ptDriver->ptDevice->uartcr_b.RIE = 0;
    ptDriver->ptDevice->uartcr_b.TIE = 0;
    ptDriver->ptDevice->uartcr_b.MSIE = 0;
    ptDriver->ptDevice->uartcr_b.SIREN = 0;
    ptDriver->ptDevice->uartcr_b.uartEN = 0;
    ptDriver->TxBuffer = 0;
    ptDriver->TxBufferSize = 0;
    ptDriver->TxBufferCounter = 0;
    ptDriver->RxBuffer = 0;
    ptDriver->RxBufferSize = 0;
    ptDriver->RxBufferCounter = 0;
    ptDriver->ptDevice->uartiir = (uint32_t) -1;
    ptDriver->ptDevice->uartcr_b.SIREN = ulSIREN;
    ptDriver->ptDevice->uartcr_b.uartEN = 1;
    DRV_NVIC_EnableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
    ret = DRV_OK;
  }
  else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
  {
    if(DRV_DMAC_Abort(ptDriver->tConfiguration.ptSequencerRx) != (ret = DRV_DMAC_Abort(ptDriver->tConfiguration.ptSequencerTx)))
    {
      ret = DRV_ERROR;
    }
    else
    {
      ptDriver->TxBuffer = 0;
      ptDriver->TxBufferSize = 0;
      ptDriver->TxBufferCounter = 0;

      ptDriver->RxBuffer = 0;
      ptDriver->RxBufferSize = 0;
      ptDriver->RxBufferCounter = 0;
    }
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The get state method will check if the driver is busy and it will return the
 * state of the device. One is able to skip the state by providing a null pointer.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] ptState The pointer to where the state of the device given will be stored.
 * \return  DRV_OK
 *          DRV_BUSY
 */
DRV_STATUS_E DRV_UART_GetState(DRV_UART_HANDLE_T * const ptDriver, DRV_UART_STATE_E * const ptState)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_STATUS_E ret = DRV_OK;
  if((NULL != ptDriver->TxBuffer)  ||
     (0 != ptDriver->TxBufferSize) ||
     (NULL != ptDriver->RxBuffer)  ||
     (0 != ptDriver->RxBufferSize) ||
     (0 != ptDriver->ptDevice->uartfr_b.BUSY))
  {
    ret = DRV_BUSY;
  }
  if(ptState != 0)
  {
    *ptState = (DRV_UART_STATE_E) (ptDriver->ptDevice->uartrsr << 16); //0..3
    *ptState = (DRV_UART_STATE_E) ((uint32_t) *ptState | ptDriver->ptDevice->uartfr); //0..7
  }
  return ret;
}

/*!
 * The get state method will check if the driver is busy in the receive patht and it will
 * return the state of the device. One is able to skip the state by providing a null pointer.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] ptState The pointer to where the state of the device given will be stored.
 * \return  DRV_OK
 *          DRV_BUSY
 */
DRV_STATUS_E DRV_UART_GetRxState(DRV_UART_HANDLE_T * const ptDriver, DRV_UART_STATE_E * const ptState)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_STATUS_E ret = DRV_OK;

  if((NULL != ptDriver->RxBuffer) ||
     (0 != ptDriver->RxBufferSize))
  {
    ret = DRV_BUSY;
  }
  if(ptState != 0)
  {
    *ptState = (DRV_UART_STATE_E) (ptDriver->ptDevice->uartrsr << 16); //0..3
    *ptState = (DRV_UART_STATE_E) ((uint32_t) *ptState | ptDriver->ptDevice->uartfr); //0..7
  }
  return ret;
}

/*!
 * The get state method will check if the driver is busy in the transmit path and it
 * will return the state of the device. One is able to skip the state by providing a null pointer.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] ptState The pointer to where the state of the device given will be stored.
 * \return  DRV_OK
 *          DRV_BUSY
 */
DRV_STATUS_E DRV_UART_GetTxState(DRV_UART_HANDLE_T * const ptDriver, DRV_UART_STATE_E * const ptState)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_STATUS_E ret = DRV_OK;

  if((NULL != ptDriver->TxBuffer) ||
     (0 != ptDriver->TxBufferSize))
  {
    ret = DRV_BUSY;
  }
  if(ptState != 0)
  {
    *ptState = (DRV_UART_STATE_E) (ptDriver->ptDevice->uartrsr << 16); //0..3
    *ptState = (DRV_UART_STATE_E) ((uint32_t) *ptState | ptDriver->ptDevice->uartfr); //0..7
  }
  return ret;
}

/*!
 * The interrupt handler of the uart device driver.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in] eDeviceID The device calling the interrupt
 * \return  void
 */
__STATIC_INLINE void DRV_UART_IRQ_Inline_Handler(DRV_UART_DEVICE_ID_E const eDeviceID)
{
  uint32_t ulDeviceOffset = (uint32_t) eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN;
  DRV_UART_HANDLE_T * const ptDriver = s_apHandleAddressTable[ulDeviceOffset];
#ifndef NDEBUG
  if(ptDriver == 0 || ptDriver->ptDevice != s_apDeviceAddressTable[ulDeviceOffset])
  {
    ((DRV_UART_DEVICE_T * const ) s_apDeviceAddressTable[ulDeviceOffset])->uartcr = 0;
    NVIC_DisableIRQ(s_apHandleIRQnTable[ulDeviceOffset]);
    return;
  }
#endif
  size_t rtis = ptDriver->ptDevice->uartiir_b.RTIS; /* Receive Timeout Int.Stat */
  DRV_UART_Flush_Buffers(ptDriver);
  if(ptDriver->RxBufferCounter == ptDriver->RxBufferSize)
  {
    if(ptDriver->RxBuffer != 0)
    {
      if(ptDriver->tConfiguration.fnRxCompleteCallback != 0)
      {
        ptDriver->tConfiguration.fnRxCompleteCallback(ptDriver, ptDriver->tConfiguration.pRxCompleteCallbackHandle);
      }
      if(ptDriver->tConfiguration.fnRxCallback == 0)
      {
        ptDriver->ptDevice->uartcr_b.RIE = 0;
        ptDriver->ptDevice->uartcr_b.RTIE = 0;
      }
      rtis = 0;
      ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->tConfiguration.eRxFifoWatermark;
      ptDriver->RxBuffer = 0;
      ptDriver->RxBufferSize = 0;
    }
    else
    {
      if(ptDriver->tConfiguration.fnRxCallback == 0)
      {
        ptDriver->ptDevice->uartcr_b.RIE = 0;
      }
    }
  }
  else
  {
    if(ptDriver->RxBufferSize - ptDriver->RxBufferCounter < ptDriver->tConfiguration.eRxFifoWatermark)
    {
      ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->RxBufferSize - ptDriver->RxBufferCounter;
    }
  }
  if(ptDriver->TxBuffer != 0)
  {
    if(ptDriver->TxBufferCounter == ptDriver->TxBufferSize)
    {
      if(ptDriver->tConfiguration.fnTxCompleteCallback != 0)
      {
        ptDriver->tConfiguration.fnTxCompleteCallback(ptDriver, ptDriver->tConfiguration.pTxCompleteCallbackHandle);
      }
      ptDriver->ptDevice->uartcr_b.TIE = 0;
      ptDriver->ptDevice->uarttxiflsel_b.TXIFLSEL = ptDriver->tConfiguration.eTxFifoWatermark;
      ptDriver->TxBuffer = 0;
      ptDriver->TxBufferSize = 0;
    }
    else
    {
      if(ptDriver->TxBufferSize - ptDriver->TxBufferCounter < ptDriver->tConfiguration.eTxFifoWatermark)
      {
        ptDriver->ptDevice->uarttxiflsel_b.TXIFLSEL = ptDriver->TxBufferSize - ptDriver->TxBufferCounter;
      }
    }
  }
  if(rtis == 1)
  {
    if(ptDriver->tConfiguration.fnRxTimeoutCallback != 0)
    {
      ptDriver->tConfiguration.fnRxTimeoutCallback(ptDriver, ptDriver->tConfiguration.pRxTimeoutCallbackHandle);
    }
    else
    {
      ptDriver->ptDevice->uartcr_b.RTIE = 0;
    }
  }
  ptDriver->ptDevice->uartiir = (uint32_t) -1;
}

/*!***************************************************************************/
/* Static function Definitions                                               */
/*!***************************************************************************/

/* Verify parameters before Initialization. */
__STATIC_INLINE DRV_STATUS_E drv_UART_Init_VerifyParameters(DRV_UART_HANDLE_T* ptDriver)
{
  DRV_STATUS_E eRet = DRV_OK;

  if((uint32_t) ptDriver->tConfiguration.eLineControl & (uint32_t) DRV_UART_LINE_CONTROL_MASK_SEND_BREAK
    || (uint32_t) ptDriver->tConfiguration.eRTSControl & (uint32_t) DRV_UART_RTS_CONTROL_MASK
    || (uint32_t) ptDriver->tConfiguration.eTxMode & (uint32_t) DRV_UART_TX_MODE_MASK_RECEIVE_ONLY)
  {
    return DRV_NSUPP;
  }
  /* The UART driver does not support DMA mode with disabled FIFO. */
  if((DRV_UART_WATERMARK_1 == ptDriver->tConfiguration.eTxFifoWatermark) &&
     (DRV_UART_WATERMARK_1 == ptDriver->tConfiguration.eRxFifoWatermark) &&
     (DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode) &&
     (0 == (ptDriver->tConfiguration.eLineControl & (uint32_t)DRV_UART_LINE_CONTROL_MASK_FIFO_ENABLE)))
  {
    return DRV_NSUPP;
  }
  if(((ptDriver->tConfiguration.eWordLength < DRV_UART_WORD_LENGTH_MIN || ptDriver->tConfiguration.eWordLength > DRV_UART_WORD_LENGTH_MAX)
    && ptDriver->tConfiguration.eWordLength != DRV_UART_WORD_LENGTH_RESET) || ptDriver->tConfiguration.eRxFifoWatermark > DRV_UART_WATERMARK_MAX
    || ptDriver->tConfiguration.eTxFifoWatermark > DRV_UART_WATERMARK_MAX || ptDriver->tConfiguration.Baud_Rate_Mode > DRV_UART_BAUDRATEMODE_MAX)
  {
    return DRV_ERROR_PARAM;
  }
  if(ptDriver->tConfiguration.eBaudrate > DRV_UART_BAUDRATE_MAX)
  {
    return DRV_ERROR_PARAM;
  }
  if(ptDriver->tConfiguration.eDeviceID >= DRV_UART_DEVICE_ID_MIN && ptDriver->tConfiguration.eDeviceID <= DRV_UART_DEVICE_ID_MAX)
  {
    ptDriver->ptDevice = s_apDeviceAddressTable[(unsigned int) ptDriver->tConfiguration.eDeviceID - (unsigned int) DRV_UART_DEVICE_ID_MIN];
  }
  else
  {
    return DRV_ERROR_PARAM;
  }

  return eRet;
}

/* Assign Default values to the Uninitialized parameters. */
__STATIC_INLINE void drv_UART_Init_SetUninitializedParametersToDefault(DRV_UART_HANDLE_T* ptDriver)
{
  if(ptDriver->tConfiguration.Baud_Rate_Mode == DRV_UART_BAUDRATEMODE_RESET)
  {
    ptDriver->tConfiguration.Baud_Rate_Mode = DRV_UART_BAUDRATEMODE_DEFAULT;
  }
  if(ptDriver->tConfiguration.eBaudrate == DRV_UART_BAUDRATE_DEFAULT)
  {
    ptDriver->tConfiguration.eBaudrate = DRV_UART_BAUDRATE_9600;
  }
  if(ptDriver->tConfiguration.eWordLength == DRV_UART_WORD_LENGTH_RESET)
  {
    ptDriver->tConfiguration.eWordLength = DRV_UART_WORD_LENGTH_DEFAULT;
  }
  if(ptDriver->tConfiguration.eRxFifoWatermark == DRV_UART_WATERMARK_UNINITIALIZED)
  {
    ptDriver->tConfiguration.eRxFifoWatermark = DRV_UART_WATERMARK_DEFAULT;
  }
  if(ptDriver->tConfiguration.eTxFifoWatermark == DRV_UART_WATERMARK_UNINITIALIZED)
  {
    ptDriver->tConfiguration.eTxFifoWatermark = DRV_UART_WATERMARK_DEFAULT;
  }
}

/* Calculates Bauddiv by the given baudrate. if the input baudrate is over 3.125 MB, the oversampling is set to 8x. */
__STATIC_INLINE void drv_UART_Init_CalculateBaudDiv(DRV_UART_HANDLE_T* ptDriver)
{
  uint64_t ullBaudRate = ptDriver->tConfiguration.eBaudrate;
  uint64_t ullBaudDiv = 0;
  uint32_t ulUartcr_2_ReducedOverSampling = 0; /* Initially oversampling is 16x. */
  uint32_t ulUartcr_2_bits = 0;
  uint32_t ulBaudDiv_LSB = 0;
  uint32_t ulBaudDiv_MSB = 0;

  /* Compensates the fact, that the baudrate enum values are divided by 100. */
  uint64_t ullSysClock = SystemCoreClock/100;

  /* Only Mode 1 is used.*/
  uint32_t ulUartcr_2_BaudRateMode = (uint32_t)DRV_UART_BAUDRATEMODE_1 - (uint32_t)DRV_UART_BAUDRATEMODE_MIN;

  /* If a baudrate over 3.125 Mbps is requested, reduced oversampling of 8x is used.
   * It doubles the baudrate internally, but the bit reception is more error-prone in noisy environments. */
  if(ullBaudRate > DRV_UART_MAX_BAUDRATE_AT_OVERSAMPL16)
  {
    ulUartcr_2_ReducedOverSampling = 1;
    /* Calculate the bauddiv, using Mode 1 formula and divide by two, because of the reduced oversampling
     * doubles the baudrate internally. */
    ullBaudDiv = (ullBaudRate * 8 * 65536) / ullSysClock;
  }
  else
  {
    /* Calculate the bauddiv, using Mode 1 standard formula: */
    ullBaudDiv = (ullBaudRate * 16 * 65536) / ullSysClock;
  }

  /* Setting the control register uartcr_2 */
  ulUartcr_2_bits = ((ulUartcr_2_BaudRateMode) |
                     (ulUartcr_2_ReducedOverSampling << uart_uartcr_2_oversampling_8x_Pos));
  ptDriver->ptDevice->uartcr_2 = ulUartcr_2_bits;

  /* Set the bauddiv. */
  ulBaudDiv_LSB = ullBaudDiv & 0xFF;
  ulBaudDiv_MSB = ullBaudDiv >> 8;
  ptDriver->ptDevice->uartlcr_l = ulBaudDiv_LSB;
  ptDriver->ptDevice->uartlcr_m = ulBaudDiv_MSB;
}

/* Sets UART Line control parameters in the uartlcr_h register.
 * Setting TxFifoWaterMark or RxFifoWaterMark higher than 1 automatically enables FIFO, overriding user's choice. */
__STATIC_INLINE void drv_UART_Init_SetLineControlParameters(DRV_UART_HANDLE_T* ptDriver)
{
  uint32_t ulUart_lcr_h_bits = 0;

  /* Set UART parameters in the uartlcr_h register :
   *  Word length [bits]: 5 / 6 / 7 / 8,      default = 8
   *  Stop bit(s):        1 / 2,              default = 1
   *  Parity:             Off / Odd / Even,   default = Off
   *  FIFO:               Disabled / Enabled, default = Disabled
   * */
  ulUart_lcr_h_bits = ptDriver->tConfiguration.eLineControl & (uint32_t)DRV_UART_LINE_CONTROL_MASK;
  ulUart_lcr_h_bits |= (ptDriver->tConfiguration.eWordLength << uart_uartlcr_h_WLEN_Pos) & uart_uartlcr_h_WLEN_Msk;

  /* Setting TxFifoWaterMark or RxFifoWaterMark higher than 1 automatically enables FIFO, overriding user's choice.
   * In Poll mode disabling FIFO could cause problems in transmissions over 5 Mbaud.
   * In IRQ mode disabling FIFO could cause problems in transmissions over 2 Mbaud.  */
  if((ptDriver->tConfiguration.eTxFifoWatermark > DRV_UART_WATERMARK_1) ||
     (ptDriver->tConfiguration.eRxFifoWatermark > DRV_UART_WATERMARK_1))
  {
    ulUart_lcr_h_bits |= DRV_UART_LINE_CONTROL_MASK_FIFO_ENABLE;
  }
  ptDriver->ptDevice->uartlcr_h = ulUart_lcr_h_bits;
}

/*!
 * The generator define for generating irq handler source code.
 */
#define DRV_UART_IRQHandler_Generator(id, _) DRV_Default_IRQHandler_Function_Generator(DRV_UART_IRQ_HANDLER ## id,DRV_UART_IRQ_Inline_Handler,DRV_UART_DEVICE_ID_UART ## id)

/*!
 * The define to where the irq handler code shall be generated.
 */
/*lint -save -e123 The name of a macro defined with arguments was subsequently used without a following '(' */
DRV_DEF_REPEAT_EVAL(DRV_UART_DEVICE_COUNT, DRV_UART_IRQHandler_Generator, ~)
/*lint -restore */

/*! \} *//* End of group UART */

#endif /* DRV_UART_MODULE_DISABLED */
