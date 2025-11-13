/*!************************************************************************//*!
 * \file    netx_drv_uart.c
 * \brief   UART peripheral module driver.
 * $Revision: 11698 $
 * $Date: 2024-11-19 13:22:39 +0100 (Di, 19 Nov 2024) $
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
 * \brief Table of the device addresses.
 *
 * Used to identify the device addresses by the device id.
 */
static DRV_UART_DEVICE_T * const s_apDeviceAddressTable[DRV_UART_DEVICE_COUNT] = DRV_UART_DEVICE_LIST;
/*!
 * \brief Table of the device associated DMA channels.
 *
 * Used to identify the device DMA channels by the device id.
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

#define DRV_UART_RX_FIFO_EMPTY             (1u)
#define DRV_UART_TX_FIFO_NOT_FULL          (0u)
#define DRV_UART_TX_FIFO_FULL              (1u)
#define DRV_UART_RX_STATIC_BUFFER_SIZE     (16)

#define DRV_UART_CLEAR_ALL_RSR_FLAGS       (0x0000000Fu)

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

__STATIC_INLINE DRV_STATUS_E
drv_UART_Init_VerifyParameters(DRV_UART_HANDLE_T* ptDriver);

__STATIC_INLINE void
drv_UART_Init_SetUninitializedParametersToDefault(DRV_UART_HANDLE_T* ptDriver);

__STATIC_INLINE void
drv_UART_Init_CalculateBaudDiv(DRV_UART_HANDLE_T* ptDriver);

__STATIC_INLINE void
drv_UART_Init_SetLineControlParameters(DRV_UART_HANDLE_T* ptDriver);

/*!***************************************************************************/
/* Definitions                                                               */
/*!***************************************************************************/

/*!
 * This callback is used in DMA operation mode. It is registered in the DMA channel API to get informed if
 * the DMA finished copying.
 */
static void
DRV_UART_Flush_DMA_Callback_Rx(void * ptDriverHandle,
                               DRV_UART_HANDLE_T * const ptDriver)
{
  if(NULL != ptDriver->tConfiguration.fnRxCompleteCallback)
  {
    ptDriver->tConfiguration.fnRxCompleteCallback(ptDriver, ptDriver->tConfiguration.pRxCompleteCallbackHandle);
  }

  ptDriver->RxBuffer = NULL;
  ptDriver->RxBufferSize = 0;
}

/*!
 * This callback is used in DMA operation mode. It is registered in the DMA channel API to get informed if
 * the DMA finished copying.
 */
static void
DRV_UART_Flush_DMA_Callback_Tx(void * ptDriverHandle,
                               DRV_UART_HANDLE_T * const ptDriver)
{
  if(NULL != ptDriver->tConfiguration.fnTxCompleteCallback)
  {
    ptDriver->tConfiguration.fnTxCompleteCallback(ptDriver, ptDriver->tConfiguration.pTxCompleteCallbackHandle);
  }

  ptDriver->TxBuffer = NULL;
  ptDriver->TxBufferSize = 0;
}

DRV_STATUS_E
DRV_UART_Init(DRV_UART_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;
  if(ptDriver == 0)
  {
    return DRV_ERROR_PARAM;
  }
  ptDriver->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptDriver);

  eRslt = drv_UART_Init_VerifyParameters(ptDriver);
  if(DRV_OK != eRslt)
  {
    return eRslt;
  }

  drv_UART_Init_SetUninitializedParametersToDefault(ptDriver);

  // Configure DMA (channel, IRQ, priority and so on)
  if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
  {
    if(NULL != ptDriver->tConfiguration.ptSequencerRx)
    {
      ptDriver->ptDevice->uartrxiflsel_b.RXDMA = 1;

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

      eRslt = DRV_DMAC_Init(ptDriver->tConfiguration.ptSequencerRx);
      if(DRV_OK != eRslt)
      {
        return eRslt;
      }
    }

    if(NULL != ptDriver->tConfiguration.ptSequencerTx)
    {
      ptDriver->ptDevice->uarttxiflsel_b.TXDMA = 1;

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

      eRslt = DRV_DMAC_Init(ptDriver->tConfiguration.ptSequencerTx);
      if(DRV_OK != eRslt)
      {
        return eRslt;
      }
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

  /* Configure NVIC (activate IRQ, define priority and so on) */
  if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
  {
    if(NULL != ptDriver->tConfiguration.fnRxCallback)  // enable IRQ only if we have 'fnRxCallback' defined
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

DRV_STATUS_E
DRV_UART_DeInit(DRV_UART_HANDLE_T * const ptDriver)
{

  DRV_STATUS_E eRslt = DRV_OK;
  ptDriver->ullFrameStartTick = 0x00ull;

  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
  DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN]);
  ptDriver->ptDevice->uartcr_b.MSIE = 0u;
  ptDriver->ptDevice->uartcr_b.RIE = 0u;
  ptDriver->ptDevice->uartcr_b.RTIE = 0u;
  ptDriver->ptDevice->uartcr_b.TIE = 0u;
  ptDriver->ptDevice->uartiir = 0xFFFFFFFFul;

  /* Disable the interface component. */
  ptDriver->ptDevice->uartcr_b.uartEN = 0;

  /* Clear the RxFIFO by reading, without saving data. There is no other mechanism to clear the FIFO. */
  while(0 == ptDriver->ptDevice->uartfr_b.RXFE)
  {
    uint32_t ulDummyRead = ptDriver->ptDevice->uartdr;
    if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
    {
      ulDummyRead++;    /* Silencing the Compiler warning */
      eRslt = DRV_TOUT;
      break;
    }
  }

  if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
  {
    ptDriver->ptDevice->uarttxiflsel_b.TXDMA = 0;
    ptDriver->ptDevice->uartrxiflsel_b.RXDMA = 0;
  }

  /* Clear the receive status error flags. */
  ptDriver->ptDevice->uartrsr = DRV_UART_CLEAR_ALL_RSR_FLAGS;

  *ptDriver = (DRV_UART_HANDLE_T ) { 0 };

  return eRslt;
}

/*!
 * \brief This function shall flush the software and hardware buffers of the device.
 *
 * This function is the hardware closest layer of the UART driver.
 * The handle contains some variables defining the software buffer state. This state shall be equalized in this function. The transmit buffer is transfered to
 * the transmit FIFO of the device from the actual counter position to the given size. The receive FIFO of the device is written to the current counter position
 * until the given size is reached. If there is no buffer given, the function will transmit the dummy pattern specified in the attributes, and receives what is
 * contained in the receive FIFO but discard it.
 *
 *
 * \private
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return DRV_OK always, because there is no error condition occurring
 */
__STATIC_FORCEINLINE void
DRV_UART_Flush_Buffers(DRV_UART_HANDLE_T * const ptDriver)
{
  if(NULL != ptDriver->RxBuffer)
  {
    while(DRV_UART_RX_FIFO_EMPTY != ptDriver->ptDevice->uartfr_b.RXFE)
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
    if(NULL != ptDriver->tConfiguration.fnRxCallback) /* but a receive callback is defined */
    {
      ptDriver->RxBufferSize = DRV_UART_RX_STATIC_BUFFER_SIZE;              /* set to DRV_UART_WATERMARK_MAX */
      while(DRV_UART_RX_FIFO_EMPTY != ptDriver->ptDevice->uartfr_b.RXFE)    /* copy into RxBufferStatic[] */
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
        // Relevant only in IRQ mode
        ptDriver->ptDevice->uartcr_b.RTIE = 1;
        ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->tConfiguration.eRxFifoWatermark;
      }

      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferSize = 0;
      ptDriver->RxBufferCounter = 0;
    }
  }

  if(NULL != ptDriver->TxBuffer)
  {
    while(DRV_UART_TX_FIFO_NOT_FULL == ptDriver->ptDevice->uartfr_b.TXFF)
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

DRV_STATUS_E
DRV_UART_Transmit(DRV_UART_HANDLE_T * const ptDriver,
                  uint8_t const * const data,
                  size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  if((NULL == data) ||
     (0 == size))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  DRV_STATUS_E eRslt = DRV_ERROR;
  uint32_t ulExclusiveRead = 0xFFFFFFFFul;
  ptDriver->ullFrameStartTick = 0;

  if((NULL != ptDriver->TxBuffer)  ||
     (0 != ptDriver->TxBufferSize))
  {
    eRslt = DRV_BUSY;
  }
  else
  {
    ptDriver->TxBufferCounter = 0;
    ptDriver->TxBufferSize = size;
    ptDriver->TxBuffer = (void*) data;

    if(DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode)
    {
      while(ptDriver->TxBufferCounter != ptDriver->TxBufferSize)
      {
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          eRslt = DRV_TOUT;
          break;
        }
        DRV_UART_Flush_Buffers(ptDriver);
      }

      if(DRV_TOUT != eRslt)
      {
        if(NULL != ptDriver->tConfiguration.fnTxCompleteCallback)
        {
          ptDriver->tConfiguration.fnTxCompleteCallback(ptDriver, ptDriver->tConfiguration.pTxCompleteCallbackHandle);
        }
        eRslt = DRV_OK;
      }

      ptDriver->TxBuffer = NULL;
      ptDriver->TxBufferSize = 0;
    }
    else if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
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
      eRslt = DRV_OK;
    }
    else if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
    {
      /* Needed for the calculation of the TxCounter. */
      ptDriver->ulDmaTxStartAddress = (uint32_t)(ptDriver->TxBuffer);

      eRslt = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerTx,
                             ptDriver->TxBuffer,
                             (void * const ) &ptDriver->ptDevice->uartdr,
                             ptDriver->TxBufferSize);
    }
    else
    {
      ptDriver->TxBuffer = NULL;
      ptDriver->TxBufferCounter = 0;
      ptDriver->TxBufferSize = 0;

      eRslt = DRV_ERROR_PARAM;
    }
  }
  DRV_UNLOCK(ptDriver);
  return eRslt;
}

DRV_STATUS_E
DRV_UART_Receive(DRV_UART_HANDLE_T * const ptDriver,
                 uint8_t * const data,
                 size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  if((NULL == data) ||
     (0 == size))
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  DRV_STATUS_E eRslt = DRV_ERROR;
  uint32_t ulExclusiveRead = 0xFFFFFFFFul;
  ptDriver->ullFrameStartTick = 0;

  if((NULL != ptDriver->RxBuffer) ||
     (0 != ptDriver->RxBufferSize))
  {
    eRslt = DRV_BUSY;
  }
  else
  {
    ptDriver->RxBufferCounter = 0;
    ptDriver->RxBufferSize = size;
    ptDriver->RxBuffer = data;
    ptDriver->ptDevice->uartrsr = DRV_UART_CLEAR_ALL_RSR_FLAGS;
    if(DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode)
    {
      while(ptDriver->RxBufferCounter != ptDriver->RxBufferSize)
      {
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          eRslt = DRV_TOUT;
          break;
        }
        DRV_UART_Flush_Buffers(ptDriver);
      }

      if(DRV_TOUT != eRslt)
      {
        if(ptDriver->tConfiguration.fnRxCompleteCallback != 0)
        {
          ptDriver->tConfiguration.fnRxCompleteCallback(ptDriver, ptDriver->tConfiguration.pRxCompleteCallbackHandle);
        }
        eRslt = DRV_OK;
      }

      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferSize = 0;
    }
    else if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
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
      eRslt = DRV_OK;
    }
    else if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
    {
      /* Needed for the calculation of the RxCounter. */
      ptDriver->ulDmaRxStartAddress = (uint32_t)(ptDriver->RxBuffer);

      eRslt = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerRx,
                             (void * const ) &ptDriver->ptDevice->uartdr,
                             ptDriver->RxBuffer,
                             ptDriver->RxBufferSize);
    }
    else
    {
      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferCounter = 0;
      ptDriver->RxBufferSize = 0;

      eRslt = DRV_ERROR_PARAM;
    }
  }
  DRV_UNLOCK(ptDriver);
  return eRslt;
}

DRV_STATUS_E
DRV_UART_TransmitReceive(DRV_UART_HANDLE_T * const ptDriver,
                         uint8_t * const txData,
                         uint8_t * const rxData,
                         size_t size)
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

  DRV_STATUS_E eRslt = DRV_NIMPL;
  uint32_t ulExclusiveRead = 0xFFFFFFFFul;
  ptDriver->ullFrameStartTick = 0;

  if((NULL != ptDriver->TxBuffer)     ||
     (0    != ptDriver->TxBufferSize) ||
     (NULL != ptDriver->RxBuffer)     ||
     (0    != ptDriver->RxBufferSize))
  {
    eRslt = DRV_BUSY;
  }
  else
  {
    ptDriver->TxBufferCounter = 0;
    ptDriver->TxBufferSize = size;
    ptDriver->TxBuffer = txData;

    ptDriver->RxBufferCounter = 0;
    ptDriver->RxBufferSize = size;
    ptDriver->RxBuffer = rxData;

    ptDriver->ptDevice->uartrsr = DRV_UART_CLEAR_ALL_RSR_FLAGS;

    if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      while((ptDriver->RxBufferCounter != ptDriver->RxBufferSize) ||
            (ptDriver->TxBufferCounter != ptDriver->TxBufferSize))
      {
        if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
        {
          eRslt = DRV_TOUT;
          break;
        }
        DRV_UART_Flush_Buffers(ptDriver);
      }

      if(DRV_TOUT != eRslt)
      {
        if(NULL != ptDriver->tConfiguration.fnTxCompleteCallback)
        {
          ptDriver->tConfiguration.fnTxCompleteCallback(ptDriver, ptDriver->tConfiguration.pTxCompleteCallbackHandle);
        }
        if(NULL != ptDriver->tConfiguration.fnRxCompleteCallback)
        {
          ptDriver->tConfiguration.fnRxCompleteCallback(ptDriver, ptDriver->tConfiguration.pRxCompleteCallbackHandle);
        }
        eRslt = DRV_OK;
      }

      ptDriver->TxBuffer = NULL;
      ptDriver->TxBufferSize = 0;

      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferSize = 0;
    }
    else if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
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
      eRslt = DRV_OK;
    }
    else if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
    {
      if((NULL == ptDriver->tConfiguration.ptSequencerTx) ||
         (NULL == ptDriver->tConfiguration.ptSequencerRx))
      {
        eRslt = DRV_ERROR_PARAM;
      }
      else
      {
        /* Needed for the calculation of TxCounter and RxCounter. */
        ptDriver->ulDmaTxStartAddress = (uint32_t)(ptDriver->TxBuffer);
        ptDriver->ulDmaRxStartAddress = (uint32_t)(ptDriver->RxBuffer);

        if((eRslt = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerTx,
                                   ptDriver->TxBuffer,
                                   (void * const ) &ptDriver->ptDevice->uartdr,
                                   ptDriver->TxBufferSize))
                                                                                !=
                    DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerRx,
                                   (void * const ) &ptDriver->ptDevice->uartdr,
                                   ptDriver->RxBuffer,
                                   ptDriver->RxBufferSize))
        {
          eRslt = DRV_ERROR;
        }
      }
    }
    else
    {
      ptDriver->TxBuffer = NULL;
      ptDriver->TxBufferCounter = 0;
      ptDriver->TxBufferSize = 0;

      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferCounter = 0;
      ptDriver->RxBufferSize = 0;

      eRslt = DRV_ERROR_PARAM;
    }
  }
  DRV_UNLOCK(ptDriver);
  return eRslt;
}

DRV_STATUS_E
DRV_UART_PutChar(DRV_UART_HANDLE_T * const ptDriver,
                 unsigned char const cData)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);

  DRV_STATUS_E eRslt = DRV_ERROR;
  ptDriver->ullFrameStartTick = 0;

  if((ptDriver->TxBufferSize > 0)                          &&
     (ptDriver->TxBufferSize != ptDriver->TxBufferCounter))
  {
    eRslt = DRV_BUSY;
  }
  else
  {
    while(DRV_UART_TX_FIFO_FULL == ptDriver->ptDevice->uartfr_b.TXFF)
    {
      if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
      {
        DRV_UNLOCK(ptDriver);
        return DRV_TOUT;
      }
    }
    ptDriver->ptDevice->uartdr = cData;
    if(0 == ptDriver->ptDevice->uartrsr)
    {
      eRslt = DRV_OK;
    }
    else
    {
      eRslt = DRV_ERROR;
    }
  }
  DRV_UNLOCK(ptDriver);
  return eRslt;
}

DRV_STATUS_E
DRV_UART_GetChar(DRV_UART_HANDLE_T * const ptDriver,
                 unsigned char * const pcData)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(pcData == 0)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }

  DRV_STATUS_E eRslt = DRV_ERROR;
  ptDriver->ullFrameStartTick = 0;

  if((ptDriver->RxBufferSize > 0)                          &&
     (ptDriver->RxBufferSize != ptDriver->RxBufferCounter))
  {
    eRslt = DRV_BUSY;
  }
  else
  {
    while(DRV_UART_RX_FIFO_EMPTY == ptDriver->ptDevice->uartfr_b.RXFE)
    {
      if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
      {
        DRV_UNLOCK(ptDriver);
        return DRV_TOUT;
      }
    }
    *pcData = ptDriver->ptDevice->uartdr;
    if(0 == ptDriver->ptDevice->uartrsr)
    {
      eRslt = DRV_OK;
    }
    else
    {
      eRslt = DRV_ERROR;
    }
  }
  DRV_UNLOCK(ptDriver);
  return eRslt;
}

DRV_STATUS_E
DRV_UART_Abort(DRV_UART_HANDLE_T * const ptDriver)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_STATUS_E eRslt = DRV_ERROR;
  if(DRV_OPERATION_MODE_POLL == ptDriver->tConfiguration.eOperationMode)
  {
    eRslt = DRV_NSUPP;
  }
  else if(DRV_OPERATION_MODE_IRQ == ptDriver->tConfiguration.eOperationMode)
  {
    ptDriver->ptDevice->uartrsr = DRV_UART_CLEAR_ALL_RSR_FLAGS;
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
    eRslt = DRV_OK;
  }
  else if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
  {
    if(DRV_DMAC_Abort(ptDriver->tConfiguration.ptSequencerRx) !=
       (eRslt = DRV_DMAC_Abort(ptDriver->tConfiguration.ptSequencerTx)))
    {
      eRslt = DRV_ERROR;
    }
    else
    {
      ptDriver->TxBuffer = NULL;
      ptDriver->TxBufferSize = 0;
      ptDriver->TxBufferCounter = 0;

      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferSize = 0;
      ptDriver->RxBufferCounter = 0;
    }
  }
  else
  {
    eRslt = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptDriver);
  return eRslt;
}

DRV_STATUS_E
DRV_UART_GetState(DRV_UART_HANDLE_T * const ptDriver,
                  DRV_UART_STATE_E * const ptState)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_STATUS_E eRslt = DRV_OK;
  if((NULL != ptDriver->TxBuffer)                ||
     (0    != ptDriver->TxBufferSize)            ||
     (NULL != ptDriver->RxBuffer)                ||
     (0    != ptDriver->RxBufferSize)            ||
     (0    != ptDriver->ptDevice->uartfr_b.BUSY))
  {
    eRslt = DRV_BUSY;
  }
  if(NULL != ptState)
  {
    /* Receive status register flags FE, PE, BE, OE are shifted to bits 16..20. Flag register flags are at bits 0..7 */
    *ptState = (DRV_UART_STATE_E) (ptDriver->ptDevice->uartrsr << 16);
    *ptState = (DRV_UART_STATE_E) ((uint32_t) *ptState | ptDriver->ptDevice->uartfr);
  }
  return eRslt;
}

DRV_STATUS_E
DRV_UART_GetRxState(DRV_UART_HANDLE_T * const ptDriver,
                    DRV_UART_STATE_E * const ptState)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_STATUS_E eRslt = DRV_OK;

  if((NULL != ptDriver->RxBuffer)     ||
     (0    != ptDriver->RxBufferSize))
  {
    eRslt = DRV_BUSY;
  }
  if(NULL != ptState)
  {
    /* Receive status register flags FE, PE, BE, OE are shifted to bits 16..20. Flag register flags are at bits 0..7 */
    *ptState = (DRV_UART_STATE_E)(ptDriver->ptDevice->uartrsr << 16);
    *ptState = (DRV_UART_STATE_E)((uint32_t) *ptState | ptDriver->ptDevice->uartfr);
  }
  return eRslt;
}

DRV_STATUS_E
DRV_UART_GetTxState(DRV_UART_HANDLE_T * const ptDriver,
                    DRV_UART_STATE_E * const ptState)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_STATUS_E eRslt = DRV_OK;

  if((NULL != ptDriver->TxBuffer) ||
     (0 != ptDriver->TxBufferSize))
  {
    eRslt = DRV_BUSY;
  }
  if(NULL != ptState)
  {
    /* Receive status register flags FE, PE, BE, OE are shifted to bits 16..20. Flag register flags are at bits 0..7 */
    *ptState = (DRV_UART_STATE_E) (ptDriver->ptDevice->uartrsr << 16);
    *ptState = (DRV_UART_STATE_E) ((uint32_t) *ptState | ptDriver->ptDevice->uartfr);
  }
  return eRslt;
}

/*!
 * The interrupt handler of the UART device driver.
 *
 * \public
 * \memberof DRV_UART_HANDLE_T
 * \param[in] eDeviceID The device calling the interrupt
 * \return  void
 */
__STATIC_INLINE void
DRV_UART_IRQ_Inline_Handler(DRV_UART_DEVICE_ID_E const eDeviceID)
{
  uint32_t ulDeviceOffset = (uint32_t) eDeviceID - (uint32_t) DRV_UART_DEVICE_ID_MIN;
  DRV_UART_HANDLE_T * const ptDriver = s_apHandleAddressTable[ulDeviceOffset];
#ifndef NDEBUG
  if((NULL == ptDriver)                                             ||
     (ptDriver->ptDevice != s_apDeviceAddressTable[ulDeviceOffset]))
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
    if(NULL != ptDriver->RxBuffer)
    {
      if(NULL != ptDriver->tConfiguration.fnRxCompleteCallback)
      {
        ptDriver->tConfiguration.fnRxCompleteCallback(ptDriver,
                                                      ptDriver->tConfiguration.pRxCompleteCallbackHandle);
      }
      if(NULL == ptDriver->tConfiguration.fnRxCallback)
      {
        ptDriver->ptDevice->uartcr_b.RIE = 0;
        ptDriver->ptDevice->uartcr_b.RTIE = 0;
      }
      rtis = 0;
      ptDriver->ptDevice->uartrxiflsel_b.RXIFLSEL = ptDriver->tConfiguration.eRxFifoWatermark;
      ptDriver->RxBuffer = NULL;
      ptDriver->RxBufferSize = 0;
    }
    else
    {
      if(NULL == ptDriver->tConfiguration.fnRxCallback)
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
  if(NULL != ptDriver->TxBuffer)
  {
    if(ptDriver->TxBufferCounter == ptDriver->TxBufferSize)
    {
      if(NULL != ptDriver->tConfiguration.fnTxCompleteCallback)
      {
        ptDriver->tConfiguration.fnTxCompleteCallback(ptDriver,
                                                      ptDriver->tConfiguration.pTxCompleteCallbackHandle);
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
    if(NULL != ptDriver->tConfiguration.fnRxTimeoutCallback)
    {
      ptDriver->tConfiguration.fnRxTimeoutCallback(ptDriver,
                                                   ptDriver->tConfiguration.pRxTimeoutCallbackHandle);
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
__STATIC_INLINE DRV_STATUS_E
drv_UART_Init_VerifyParameters(DRV_UART_HANDLE_T* ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;

  if(((uint32_t) DRV_UART_LINE_CONTROL_MASK_SEND_BREAK & (uint32_t) ptDriver->tConfiguration.eLineControl) ||
     ((uint32_t) DRV_UART_RTS_CONTROL_MASK             & (uint32_t) ptDriver->tConfiguration.eRTSControl)  ||
     ((uint32_t) DRV_UART_TX_MODE_MASK_RECEIVE_ONLY    & (uint32_t) ptDriver->tConfiguration.eTxMode))
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
  if((((DRV_UART_WORD_LENGTH_MIN > ptDriver->tConfiguration.eWordLength) ||
       (DRV_UART_WORD_LENGTH_MAX < ptDriver->tConfiguration.eWordLength))     &&
       (DRV_UART_WORD_LENGTH_RESET != ptDriver->tConfiguration.eWordLength))       ||
     (DRV_UART_WATERMARK_MAX < ptDriver->tConfiguration.eRxFifoWatermark)          ||
     (DRV_UART_WATERMARK_MAX < ptDriver->tConfiguration.eTxFifoWatermark)          ||
     (DRV_UART_BAUDRATEMODE_MAX < ptDriver->tConfiguration.Baud_Rate_Mode))
  {
    return DRV_ERROR_PARAM;
  }
  if(ptDriver->tConfiguration.eBaudrate > DRV_UART_BAUDRATE_MAX)
  {
    return DRV_ERROR_PARAM;
  }
  if((DRV_UART_DEVICE_ID_MIN <= ptDriver->tConfiguration.eDeviceID) &&
     (DRV_UART_DEVICE_ID_MAX >= ptDriver->tConfiguration.eDeviceID))
  {
    ptDriver->ptDevice = s_apDeviceAddressTable[(unsigned int) ptDriver->tConfiguration.eDeviceID - (unsigned int) DRV_UART_DEVICE_ID_MIN];
  }
  else
  {
    return DRV_ERROR_PARAM;
  }

  if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
  {
    if((NULL == ptDriver->tConfiguration.ptSequencerTx)  &&
       (NULL == ptDriver->tConfiguration.ptSequencerRx))
    {
      return DRV_ERROR_PARAM;
    }
    else if((NULL != ptDriver->tConfiguration.ptSequencerRx)                 &&
            ((DRV_DMAC_DEVICE_ID_MIN > ptDriver->tConfiguration.eDMARx)  ||
             (DRV_DMAC_DEVICE_ID_MAX < ptDriver->tConfiguration.eDMARx)))
    {
      return DRV_ERROR_PARAM;
    }
    else if((NULL != ptDriver->tConfiguration.ptSequencerTx)                 &&
            ((DRV_DMAC_DEVICE_ID_MIN > ptDriver->tConfiguration.eDMATx)  ||
             (DRV_DMAC_DEVICE_ID_MAX < ptDriver->tConfiguration.eDMATx)))
    {
      return DRV_ERROR_PARAM;
    }
    else if((NULL != ptDriver->tConfiguration.ptSequencerRx) &&
            (NULL != ptDriver->tConfiguration.ptSequencerTx) &&
            (ptDriver->tConfiguration.eDMATx == ptDriver->tConfiguration.eDMARx))
    {
      return DRV_ERROR_PARAM;
    }
  }


  return eRslt;
}

/* Assign Default values to the Uninitialized parameters. */
__STATIC_INLINE void drv_UART_Init_SetUninitializedParametersToDefault(DRV_UART_HANDLE_T* ptDriver)
{
  if(DRV_UART_BAUDRATEMODE_RESET == ptDriver->tConfiguration.Baud_Rate_Mode)
  {
    ptDriver->tConfiguration.Baud_Rate_Mode = DRV_UART_BAUDRATEMODE_DEFAULT;
  }
  if(DRV_UART_BAUDRATE_DEFAULT == ptDriver->tConfiguration.eBaudrate)
  {
    ptDriver->tConfiguration.eBaudrate = DRV_UART_BAUDRATE_9600;
  }
  if(DRV_UART_WORD_LENGTH_RESET == ptDriver->tConfiguration.eWordLength)
  {
    ptDriver->tConfiguration.eWordLength = DRV_UART_WORD_LENGTH_DEFAULT;
  }
  if(DRV_UART_WATERMARK_UNINITIALIZED == ptDriver->tConfiguration.eRxFifoWatermark)
  {
    ptDriver->tConfiguration.eRxFifoWatermark = DRV_UART_WATERMARK_DEFAULT;
  }
  if(DRV_UART_WATERMARK_UNINITIALIZED == ptDriver->tConfiguration.eTxFifoWatermark)
  {
    ptDriver->tConfiguration.eTxFifoWatermark = DRV_UART_WATERMARK_DEFAULT;
  }
}

/* Calculates Bauddiv by the given baudrate. if the input baudrate is over 3.125 MB, the oversampling is set to 8x. */
__STATIC_INLINE void
drv_UART_Init_CalculateBaudDiv(DRV_UART_HANDLE_T* ptDriver)
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
__STATIC_INLINE void
drv_UART_Init_SetLineControlParameters(DRV_UART_HANDLE_T* ptDriver)
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
  if((DRV_UART_WATERMARK_1 < ptDriver->tConfiguration.eTxFifoWatermark) ||
     (DRV_UART_WATERMARK_1 < ptDriver->tConfiguration.eRxFifoWatermark))
  {
    ulUart_lcr_h_bits |= DRV_UART_LINE_CONTROL_MASK_FIFO_ENABLE;
  }
  ptDriver->ptDevice->uartlcr_h = ulUart_lcr_h_bits;
}

/*!
 * The generator define for generating IRQ handler source code.
 */
#define DRV_UART_IRQHandler_Generator(id, _) DRV_Default_IRQHandler_Function_Generator(DRV_UART_IRQ_HANDLER ## id,DRV_UART_IRQ_Inline_Handler,DRV_UART_DEVICE_ID_UART ## id)

/*!
 * The define to where the IRQ handler code shall be generated.
 */
/*lint -save -e123 The name of a macro defined with arguments was subsequently used without a following '(' */
DRV_DEF_REPEAT_EVAL(DRV_UART_DEVICE_COUNT, DRV_UART_IRQHandler_Generator, ~)
/*lint -restore */

#endif /* DRV_UART_MODULE_DISABLED */
