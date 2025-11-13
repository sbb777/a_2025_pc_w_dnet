/*!************************************************************************//*!
 * \file     netx_drv_spi.c
 * \brief    SPI peripheral module driver.
 * $Revision: 11376 $
 * $Date: 2024-06-12 15:33:14 +0300 (Wed, 12 Jun 2024) $
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
#ifdef DRV_SPI_MODULE_ENABLED

/*! \defgroup SPI SPI
 * \{
 * \brief The SPI driver, defined by DRV_SPI_HANDLE_T
 *
 * \details
 *
 * @startuml
 * robust "FSS/CSS" as CSr
 * robust "Clock" as CLKr
 * robust "MOSI" as MOSIr
 * robust "MISO" as MISOr
 * concise "MOSI" as MOSIc
 * concise "MISO" as MISOc
 *
 * @0
 * CSr is high
 * CLKr is high
 * MOSIr is high
 * MISOr is high
 * MOSIc is {-}
 * MISOc is {-}
 *
 * @100
 * CSr is low
 * CLKr is high
 * MOSIr is high
 * MISOr is high
 * MOSIc is Data
 * MISOc is Data
 *
 * @300
 * CSr is high
 * CLKr is high
 * MOSIr is high
 * MISOr is high
 * MOSIc is {-}
 * MISOc is {-}
 *
 * @enduml
 */

/*!
 * \brief Table of the device addresses.
 *
 * Used to identify the device addresses by the device id.
 */
static DRV_SPI_DEVICE_U const s_apDeviceAddressTable[DRV_SPI_DEVICE_COUNT] = DRV_SPI_DEVICE_LIST;

/*!
 * \brief Table of the dma channels of the devices.
 */
static DRV_DMAC_PERIPHERAL_E const s_apDeviceDmacTable[DRV_SPI_DEVICE_COUNT] = DRV_SPI_DEVICE_DMA_LIST;
/*!
 * \brief Table of the IRQ vector numbers.
 *
 * Used to identify the interrupt channels by the device id.
 */
static IRQn_Type const s_apHandleIRQnTable[DRV_SPI_DEVICE_COUNT] = DRV_SPI_DEVICE_IRQ_LIST;
/*!
 * \brief Used for mapping the handle to an interrupt.
 *
 * Threadsafe and reentrant because its is only written in normal context an used in interrupt context of the specific interrupt.
 */
static DRV_SPI_HANDLE_T * s_apHandleAddressTable[DRV_SPI_DEVICE_COUNT] = { 0 };

/*!
 * Define for checking the consistency of the handle or static representation of the driver.
 */
#ifndef DRV_HANDLE_CHECK_INACTIVE
#define DRV_HANDLE_CHECK(handle);\
  if((handle)==0){ \
    return DRV_ERROR_PARAM; \
  }\
  if((handle)->ptDevice.ptSPI==0){ \
    return DRV_ERROR_PARAM; \
  }
#else
#define DRV_HANDLE_CHECK(handle);
#endif

/*!
 * Timeout in mS for DRV_SPI_WaitNotBusy function.
 */
#define DRV_SPI_WAIT_SPI_NOT_BUSY_TIMEOUT_MS          5

/*!
 * This function is used to wait until SPI interface finishes transmitting/receiving data.
 * For safety reasons a timeout is implemented otherwise device could get stuck.
 */
__STATIC_INLINE void DRV_SPI_WaitNotBusy(DRV_SPI_HANDLE_T* const ptDriver);

__STATIC_INLINE DRV_STATUS_E DRV_SPI_InitDmaMode(DRV_SPI_HANDLE_T * const ptDriver);

__STATIC_INLINE DRV_STATUS_E DRV_SPI_InitVerifyConfig(DRV_SPI_HANDLE_T * const ptDriver);

__STATIC_INLINE DRV_STATUS_E DRV_SPI_InitVerifyCommonConfig(DRV_SPI_HANDLE_T * const ptDriver);

__STATIC_INLINE DRV_STATUS_E DRV_SPI_InitVerifySqiConfig(DRV_SPI_HANDLE_T * const ptDriver);

__STATIC_INLINE DRV_STATUS_E DRV_SPI_InitVerifySpiConfig(DRV_SPI_HANDLE_T * const ptDriver);

__STATIC_INLINE DRV_STATUS_E DRV_SPI_InitVerifyDmaConfig(DRV_SPI_HANDLE_T * const ptDriver);

/*!
 * This callback is used in dma operation mode. It is registered in the dmac api to get informed if
 * the dma finished copying.
 */
static void DRV_SPI_Flush_DMA_Callback_Rx(void * ptDriverHandle, DRV_SPI_HANDLE_T * const ptDriver)
{
  if(ptDriver->TxBufferSize == 0)
  {
    if(ptDriver->tConfiguration.eFSSStatic == DRV_SPI_FSS_STATIC_DRIVER && ptDriver->tConfiguration.eBehaviour == DRV_SPI_BEHAVIOUR_MASTER)
    {
      // After Rx last byte, the bus should be idle in SPI.
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
    }
    if(ptDriver->tConfiguration.fnCompleteCallback != 0)
    {
      ptDriver->tConfiguration.fnCompleteCallback(ptDriver, ptDriver->tConfiguration.pCompleteCallbackHandle);
    }
  }
  ptDriver->RxBuffer = 0;
  ptDriver->RxBufferSize = 0;
  ptDriver->RxBufferCounter = 0;
}

/*!
 * This callback is used in dma operation mode. It is registered in the dmac api to get informed if
 * the dma finished copying.
 */
static void DRV_SPI_Flush_DMA_Callback_Tx(void * ptDriverHandle, DRV_SPI_HANDLE_T * const ptDriver)
{
  if(ptDriver->RxBufferSize == 0)
  {
    if(ptDriver->tConfiguration.eFSSStatic == DRV_SPI_FSS_STATIC_DRIVER && ptDriver->tConfiguration.eBehaviour == DRV_SPI_BEHAVIOUR_MASTER)
    {
      // Should not happen, because last byte Rx should still be pending in SPI.
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
    }
    if(ptDriver->tConfiguration.fnCompleteCallback != 0)
    {
      ptDriver->tConfiguration.fnCompleteCallback(ptDriver, ptDriver->tConfiguration.pCompleteCallbackHandle);
    }
  }
  ptDriver->TxBuffer = 0;
  ptDriver->TxBufferSize = 0;
  ptDriver->TxBufferCounter = 0;
}

/*!
 * This function is used in the DRV_SPI_Init() function in order to check the given parameters in the DRV_SPI_ATTRIBUTES_T structure if there are parameter combinations not feasible.
*/
__STATIC_INLINE
DRV_STATUS_E DRV_SPI_InitVerifyConfig(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;

  eRslt = DRV_SPI_InitVerifyCommonConfig(ptDriver);
  if(DRV_OK == eRslt)
  {
    eRslt = DRV_SPI_InitVerifySqiConfig(ptDriver);
  }
  if(DRV_OK == eRslt)
  {
    eRslt = DRV_SPI_InitVerifySpiConfig(ptDriver);
  }
  if(DRV_OK == eRslt)
  {
    eRslt = DRV_SPI_InitVerifyDmaConfig(ptDriver);
  }

  return eRslt;
}

/*!
 * This function is used in the DRV_SPI_Init() function in order to check the common configuration parameters in the DRV_SPI_ATTRIBUTES_T structure.
*/
__STATIC_INLINE DRV_STATUS_E
DRV_SPI_InitVerifyCommonConfig(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;

  if(((uint32_t)DRV_SPI_DATA_SIZE_SELECT_MAX < (uint32_t)ptDriver->tConfiguration.eDataSize))
  {
    eRslt = DRV_ERROR_PARAM;
  }
  else if((DRV_SPI_DATA_SIZE_SELECT_MIN > ptDriver->tConfiguration.eDataSize) &&
          (DRV_SPI_DATA_SIZE_SELECT_RESET != ptDriver->tConfiguration.eDataSize))
  {
    eRslt = DRV_ERROR_PARAM;
  }
  else if((DRV_SPI_DEVICE_ID_MIN > ptDriver->tConfiguration.eSPIDeviceID) ||
          (DRV_SPI_DEVICE_ID_MAX < ptDriver->tConfiguration.eSPIDeviceID))
  {
    eRslt = DRV_ERROR_PARAM;
  }
  else if((DRV_SPI_FREQUENCY_MIN > ptDriver->tConfiguration.eFrequency) ||
    /*lint -save -e685 */
          (DRV_SPI_FREQUENCY_MAX < ptDriver->tConfiguration.eFrequency))
    /*lint -restore */
  {
    eRslt = DRV_ERROR_PARAM;
  }
  else if((DRV_SPI_PARALLELISM_MIN > ptDriver->tConfiguration.eParallelism) ||
          (DRV_SPI_PARALLELISM_MAX < ptDriver->tConfiguration.eParallelism))
  {
    eRslt = DRV_ERROR_PARAM;
  }
  else if((DRV_SPI_BEHAVIOUR_SLAVE == ptDriver->tConfiguration.eBehaviour) &&
          (DRV_SPI_FSS_STATIC_DRIVER == ptDriver->tConfiguration.eFSSStatic))
  {
    eRslt = DRV_ERROR_PARAM; /*! Protect Slave from entering fss_static mode, where it could accidentally manipulate chip select signals */
  }
  else if((DRV_SPI_FSS_STATIC_CALLER == ptDriver->tConfiguration.eFSSStatic) &&
          (DRV_SPI_FSS_NONE != ptDriver->tConfiguration.eFSS))
  {
    eRslt = DRV_ERROR_PARAM;
  }
  else if((DRV_SPI_PARALLELISM_1BIT != ptDriver->tConfiguration.eParallelism) &&
          (DRV_SPI_FSS_STATIC_HARDWARE == ptDriver->tConfiguration.eFSSStatic))
  {
    eRslt = DRV_NSUPP; /*! For SQI modes (2 & 4 bits), chip select is never generated automatically. */
  }

  return eRslt;
}

/*!
 * This function is used in the DRV_SPI_Init() function in order to check the SQI configuration parameters in the DRV_SPI_ATTRIBUTES_T structure.
*/
__STATIC_INLINE DRV_STATUS_E
DRV_SPI_InitVerifySqiConfig(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;

  if((DRV_SPI_DEVICE_ID_SQI_BORDER <= ptDriver->tConfiguration.eSPIDeviceID) &&
     (DRV_SPI_DEVICE_ID_QSPI_BORDER > ptDriver->tConfiguration.eSPIDeviceID))
  {
    if(DRV_SPI_LOOP_BACK_MODE_ACTIVE == ptDriver->tConfiguration.eLoopBackMode)
    {
      eRslt = DRV_NSUPP; /*! Loopback mode is not available for SQI devices */
    }

    else if(DRV_SPI_BEHAVIOUR_SLAVE == ptDriver->tConfiguration.eBehaviour)
    {
      eRslt = DRV_NSUPP; /*! Slave mode is not available for SQI devices */
    }

    else if((DRV_SPI_DUMMYPATTERN_FULL != ptDriver->tConfiguration.uDummyPattern) &&
            (DRV_SPI_DUMMYPATTERN_NULL != ptDriver->tConfiguration.uDummyPattern))
    {
      eRslt = DRV_NSUPP; /*! Alternating Dummy patterns are not available for SQI devices */
    }

    else if(DRV_SPI_MISO_INACTIVE == ptDriver->tConfiguration.eMISO)
    {
      eRslt = DRV_NSUPP; /*! MISO must be active for SQI devices */
    }

    else if((DRV_SPI_DUPLEX_FULL == ptDriver->tConfiguration.eDuplex) &&
            (DRV_SPI_PARALLELISM_1BIT != ptDriver->tConfiguration.eParallelism))
    {
      eRslt = DRV_NSUPP; /*! Full duplex is not available for 2-bit and 4-bit SQI modes */
    }
  }

  return eRslt;
}

/*!
 * This function is used in the DRV_SPI_Init() function in order to check the SPI configuration parameters in the DRV_SPI_ATTRIBUTES_T structure.
*/
__STATIC_INLINE DRV_STATUS_E
DRV_SPI_InitVerifySpiConfig(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;

  if(DRV_SPI_DEVICE_ID_SQI_BORDER > ptDriver->tConfiguration.eSPIDeviceID)
  {
    if(DRV_SPI_DUPLEX_HALF == ptDriver->tConfiguration.eDuplex)
    {
      eRslt = DRV_NSUPP; /*! SPI Devices do not support Half Duplex */
    }
  }

  return eRslt;
}

/*!
 * This function is used in the DRV_SPI_Init() function in order to check the DMA configuration parameters in the DRV_SPI_ATTRIBUTES_T structure.
*/
__STATIC_INLINE DRV_STATUS_E
DRV_SPI_InitVerifyDmaConfig(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;

  if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
  {
    if((0 == ptDriver->tConfiguration.ptSequencerTx) ||
       (0 == ptDriver->tConfiguration.ptSequencerRx))
    {
      eRslt = DRV_ERROR_PARAM;
    }
    else if((DRV_SPI_FSS_STATIC_DRIVER == ptDriver->tConfiguration.eFSSStatic))
    {
      eRslt = DRV_NSUPP; /*! In DMA mode FSS must be controlled automatically (by hardware) or manually */
    }
  }

  return eRslt;
}

/*!
 * This function is used in the DRV_SPI_Init() function in order to configure and initialize the DMA channels
*/
__STATIC_INLINE
DRV_STATUS_E DRV_SPI_InitDmaMode(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;
  DRV_DMAC_TRANSFER_WIDTH_E dma_width = DRV_DMAC_TRANSFER_WIDTH_8b;

  /* when using DRV_SPI_DEVICE_U union with SQI device, these lines actually change sqi_pio_out register */
  ptDriver->ptDevice.ptSPI->spi_dmacr_b.RXDMAE = 1;
  ptDriver->ptDevice.ptSPI->spi_dmacr_b.TXDMAE = 1;

  ptDriver->ptDevice.ptSQI->sqi_dmacr_b.rx_dma_en = 1;
  ptDriver->ptDevice.ptSQI->sqi_dmacr_b.tx_dma_en = 1;

  if(DRV_SPI_DATA_SIZE_SELECT_8b < ptDriver->tConfiguration.eDataSize)
  {
    dma_width = DRV_DMAC_TRANSFER_WIDTH_16b;
  }

  if ((DRV_SPI_DEVICE_ID_SQI_BORDER <= ptDriver->tConfiguration.eSPIDeviceID) &&
      (DRV_SPI_DEVICE_ID_QSPI_BORDER > ptDriver->tConfiguration.eSPIDeviceID))
  {
    if(DRV_SPI_PARALLELISM_1BIT < ptDriver->tConfiguration.eParallelism)
    {
      dma_width = DRV_DMAC_TRANSFER_WIDTH_32b; /* Needed for QSPI-Mode, 2 or 4 bits. For 1-bit (SPI mode), previous settings apply */
    }
  }

  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eDeviceID = ptDriver->tConfiguration.eDMARx;
  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.ePeripheralSource =
    (DRV_DMAC_PERIPHERAL_E) ((uint32_t) s_apDeviceDmacTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]
      + 0u);
  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.ePeripheralDest = DRV_DMAC_PERIPHERAL_MEMORY;
  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eIncrementationSource = DRV_DMAC_INCREMENTATION_INACTIVE;
  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eIncrementationDest = DRV_DMAC_INCREMENTATION_ACTIVE;
  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eTransferWidthSource = dma_width;
  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eTransferWidthDest = dma_width;
  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.fCallbackComplete = (DRV_CALLBACK_F) DRV_SPI_Flush_DMA_Callback_Rx;
  ptDriver->tConfiguration.ptSequencerRx->tConfiguration.ptCallbackHandleComplete = ptDriver;

  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eDeviceID = ptDriver->tConfiguration.eDMATx;
  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.ePeripheralSource = DRV_DMAC_PERIPHERAL_MEMORY;
  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.ePeripheralDest =
    (DRV_DMAC_PERIPHERAL_E) ((uint32_t) s_apDeviceDmacTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]
      + 1u);
  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eIncrementationSource = DRV_DMAC_INCREMENTATION_ACTIVE;
  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eIncrementationDest = DRV_DMAC_INCREMENTATION_INACTIVE;
  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eTransferWidthSource = dma_width;
  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eTransferWidthDest = dma_width;
  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.fCallbackComplete = (DRV_CALLBACK_F) DRV_SPI_Flush_DMA_Callback_Tx;
  ptDriver->tConfiguration.ptSequencerTx->tConfiguration.ptCallbackHandleComplete = ptDriver;

  eRslt = DRV_DMAC_Init(ptDriver->tConfiguration.ptSequencerTx);
  if(DRV_OK == eRslt)
  {
    eRslt = DRV_DMAC_Init(ptDriver->tConfiguration.ptSequencerRx);
  }

  return eRslt;
}

/*!
 * The function takes a DRV_SPI_HANDLE_T pointer which contains a DRV_SPI_ATTRIBUTES_T structure. Those structure contains the configuration/initialization parameters of the device.
 * While most attributes have valid default behavior, it is necessary to configure a bus speed and the DRV_SPI_DEVICE_ID_E
 *
 * It is not possible to support all bus speeds by the driver. In future, there will be a matrix with configuration options and build options followed by possible speeds.
 * At the moment we recommend to use DRV_SPI_Frequency_1_5625MHz as value for ulFrequency.
 *
 * The SPIs initialize function forces the initialization of the SPI peripheral and the handle.
 * The driver lock is set and the given parameters in the DRV_SPI_ATTRIBUTES_T structure are checked if there are parameter combinations not feasible.
 * If everything is ok, the Buffers will be reset and the configuration registers are written, regarding the given attributes. At last the lock will be released.
 *
 * In case of an error during initialization, the function returns an error value, but the lock for the handle, will not be released.
 * Together with the return value, this will ensure that the driver can only be interacted with if it has been correctly initialized.
 * Since the lock is initialized when this function is called, this function can still be called again with other parameters after a failed initialization.
 *
 * \public
 * \memberof DRV_SPI_HANDLE_T
 * \param[out] ptDriver The ptDriver to be
 * \return  DRV_OK
 *          DRV_ERROR_PARAM
 *          DRV_NSUPP
 */
DRV_STATUS_E DRV_SPI_Init(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E eRslt = DRV_OK;

  if(ptDriver == 0)
  {
    return DRV_ERROR_PARAM;
  }

  ptDriver->tLock = DRV_LOCK_INITIALIZER;
  DRV_LOCK(ptDriver);

  eRslt = DRV_SPI_InitVerifyConfig(ptDriver);
  if (DRV_OK != eRslt)
  {
    return eRslt;
  }

  s_apHandleAddressTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN] = ptDriver;
  ptDriver->ptDevice = s_apDeviceAddressTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN];

  if(ptDriver->tConfiguration.eSPIDeviceID >= DRV_SPI_DEVICE_ID_MIN && ptDriver->tConfiguration.eSPIDeviceID < DRV_SPI_DEVICE_ID_SQI_BORDER)
  {
    /* Pre-Initial values in SPI control registers:
     *
     *** spi_cr0 ***
     * netX 100/500 compatible mode = 0
     * slave_sig_early = 0 (not early)
     * filter_in       = 0 (Inactive)
     * format          = 0 (Motorola SPI frame format)
     * Frequency       = 0x800 (50 MHz)
     * SPH             = 0
     * SPO             = 0
     * data_size       = 7 (8-bit) */
    ptDriver->ptDevice.ptSPI->spi_cr0 = 0x00080007;

    /*
     *** spi_cr1 ***
     * rx_fifo_wm = 8
     * tx_fifo_wm = 8
     * fss_static = 0
     * fss        = 0
     * sod        = 0
     * ms         = 0
     * sse        = 0
     * lbm        = 0
     */
    ptDriver->ptDevice.ptSPI->spi_cr1 = 0x08080000;

    ptDriver->ptDevice.ptSPI->spi_imsc  = 0x00000000;
    ptDriver->ptDevice.ptSPI->spi_icr   = (unsigned) -1;
    ptDriver->ptDevice.ptSPI->spi_dmacr = 0x00000000;
  }
  else if(ptDriver->tConfiguration.eSPIDeviceID >= DRV_SPI_DEVICE_ID_SQI_BORDER
    && ptDriver->tConfiguration.eSPIDeviceID < DRV_SPI_DEVICE_ID_QSPI_BORDER)
  {

    /* Pre-Initial values in SQI control registers:
     *
     *** sqi_cr0 ***
     * filter_in       = 0 (Inactive)
     * sio_cfg         = 0 (only SIO2+3 are controllable as PIOs (2-bit SPI or standard Motorola SPI))
     * Frequency       = 0x800 (50 MHz)
     * SPH             = 0
     * SPO             = 0
     * data_size       = 7 (8-bit) */
    ptDriver->ptDevice.ptSQI->sqi_cr0 = 0x00080007;

    /*
     *** sqi_cr1 ***
     * rx_fifo_wm     = 8
     * tx_fifo_wm     = 8
     * spi_trans_ctrl = 0:  Transfers start immediately after transfer data has been written to TX FIFO
     * fss_static     = 0
     * fss            = 0
     * se             = 0
     */
    ptDriver->ptDevice.ptSQI->sqi_cr1 = 0x08080000;

    /*** sqi_tcr ***
     * ms_byte_first  = 0 less significant byte first, Little Endian)
     * ms_bit_first   = 1 (Most significant bit first)
     * duplex         = 3 (Full-duplex)
     * mode           = 0 (Standard Motorola SPI mode)
     * start_transfer = 0
     * tx_oe          = 0
     * tx_out         = 0
     * transfer_size  = 0
     */
    ptDriver->ptDevice.ptSQI->sqi_tcr = 0x1c000000;

    ptDriver->ptDevice.ptSQI->sqi_irq_mask   = 0x00000000;
    ptDriver->ptDevice.ptSQI->sqi_irq_raw    = (unsigned) -1;
    ptDriver->ptDevice.ptSQI->sqi_irq_clear  = (unsigned) -1;
    ptDriver->ptDevice.ptSQI->sqi_dmacr      = 0x00000000;
    ptDriver->ptDevice.ptSQI->sqi_pio_oe     = 0x00000000;
    ptDriver->ptDevice.ptSQI->sqi_pio_out    = 0x0000000e;
    ptDriver->ptDevice.ptSQI->sqi_sqirom_cfg = 0x02020004;
  }
  else if((ptDriver->tConfiguration.eSPIDeviceID >= DRV_SPI_DEVICE_ID_QSPI_BORDER) &&
          (ptDriver->tConfiguration.eSPIDeviceID <= DRV_SPI_DEVICE_ID_MAX))
  {
    return DRV_NSUPP; // device yet not available
  }

  /* Configure DMA (channels, IRQ, priority and so on) */
  if(DRV_OPERATION_MODE_DMA == ptDriver->tConfiguration.eOperationMode)
  {
    eRslt = DRV_SPI_InitDmaMode(ptDriver);
    if (DRV_OK != eRslt)
    {
      return eRslt;
    }
  }

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
  ptDriver->ptDevice.ptSQI->sqi_cr0_b.sio_cfg = 01;
  ptDriver->ptDevice.ptSPI->spi_cr0_b.netx100_comp = 0x0u;
  ptDriver->ptDevice.ptSPI->spi_cr0_b.slave_sig_early = ptDriver->tConfiguration.eSlaveSigEarly;
  ptDriver->ptDevice.ptSPI->spi_cr0_b.filter_in = ptDriver->tConfiguration.eFILTER;
  ptDriver->ptDevice.ptSPI->spi_cr0_b.format = ptDriver->tConfiguration.eFrameFormat;
  ptDriver->ptDevice.ptSPI->spi_cr0_b.sck_muladd = ptDriver->tConfiguration.eFrequency;
  ptDriver->ptDevice.ptSPI->spi_cr0_b.SPH = ptDriver->tConfiguration.uMode.structure.eSPH;
  ptDriver->ptDevice.ptSPI->spi_cr0_b.SPO = ptDriver->tConfiguration.uMode.structure.eSPO;
  if(DRV_SPI_DATA_SIZE_SELECT_RESET == ptDriver->tConfiguration.eDataSize)
  {
    ptDriver->tConfiguration.eDataSize = DRV_SPI_DATA_SIZE_SELECT_DEFAULT;
  }
  ptDriver->ptDevice.ptSPI->spi_cr0_b.datasize = ptDriver->tConfiguration.eDataSize;

  ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 0x01u;
  ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 0x01u;
  if(DRV_SPI_FIFO_WM_RESET == ptDriver->tConfiguration.eRxFiFoWm)
  {
    ptDriver->tConfiguration.eRxFiFoWm = DRV_SPI_FIFO_WM_DEFAULT;
  }
  ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_wm = ptDriver->tConfiguration.eRxFiFoWm - (unsigned int) DRV_SPI_FIFO_WM_MIN;
  if(DRV_SPI_FIFO_WM_RESET == ptDriver->tConfiguration.eTxFiFoWm)
  {
    ptDriver->tConfiguration.eTxFiFoWm = DRV_SPI_FIFO_WM_DEFAULT;
  }
  if(DRV_SPI_FIFO_WM_RESET == ptDriver->tConfiguration.eTxFiFoRefillLevel)
  {
    ptDriver->tConfiguration.eTxFiFoRefillLevel = DRV_SPI_FIFO_WM_MAX;
  }
  ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_wm = ptDriver->tConfiguration.eTxFiFoWm - (unsigned int) DRV_SPI_FIFO_WM_MIN;
  if(DRV_SPI_FSS_STATIC_DRIVER == ptDriver->tConfiguration.eFSSStatic)
  {
    ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
  }
  else if(DRV_SPI_FSS_STATIC_HARDWARE == ptDriver->tConfiguration.eFSSStatic)
  {
    ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = ptDriver->tConfiguration.eFSS;
  }
  ptDriver->ptDevice.ptSPI->spi_cr1_b.fss_static = ptDriver->tConfiguration.eFSSStatic;
  ptDriver->ptDevice.ptSPI->spi_cr1_b.SOD = ptDriver->tConfiguration.eMISO;
  ptDriver->ptDevice.ptSPI->spi_cr1_b.MS = ptDriver->tConfiguration.eBehaviour;
  ptDriver->ptDevice.ptSPI->spi_cr1_b.SSE = 0x01u;
  ptDriver->ptDevice.ptSPI->spi_cr1_b.LBM = ptDriver->tConfiguration.eLoopBackMode;

  // Configure nvic (activate IRQ, define priority and so on)
  if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]);
    DRV_NVIC_EnableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]);
  }

  DRV_UNLOCK(ptDriver);

  return DRV_OK;
}

/*!
 * The de-initialize function is supposed to reset the handle, the device and disable the interrupt signaling.
 * The handle might be used afterwards again for initializing a SPI device driver handle.
 *
 * \public
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The ptDriver to be deinitialized
 * \return  DRV_OK
 *          DRV_LOCKED
 *          DRV_ERROR_PARAM
 */
DRV_STATUS_E DRV_SPI_DeInit(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]);
  DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]);
  ptDriver->ptDevice.ptSQI->sqi_irq_mask = 0u;
  ptDriver->ptDevice.ptSQI->sqi_irq_clear = 0xFFFFFFFFul;
  *ptDriver = (DRV_SPI_HANDLE_T ) { { 0 } };
  return DRV_OK;
}

/*!
 * \brief This function shall flush the software and hardware buffers of the device.
 *
 * \details This function is the hardware closest layer of the spi driver.
 * The handle contains some variables defining the software buffer state. This state shall be equalized in this function. The transmit buffer is transfered to
 * the transmit fifo of the device from the actual counter position to the given size. The receive fifo of the device is written to the current counter position
 * until the given size is reached. If there is no buffer given, the function will transmit the dummy pattern specified in the attributes, and receives what is
 * contained in the receive fifo but discard it.
 *
 *
 * \private
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return DRV_OK always, because there is no error condition occurring
 */
__STATIC_INLINE DRV_STATUS_E DRV_SPI_Flush_Buffers(DRV_SPI_HANDLE_T * const ptDriver)
{
  size_t border, level;
  size_t i;
// Hardware half or full duplex
  if(ptDriver->RxBufferCounter != ptDriver->RxBufferSize)
  {
    border = ptDriver->ptDevice.ptSPI->spi_sr_b.rx_fifo_level;
    if(ptDriver->RxBufferCounter + border >= ptDriver->RxBufferSize)
    {
      border = ptDriver->RxBufferSize - ptDriver->RxBufferCounter;
    }
    for(i = 0; i < border; i++)
    {
      // Software half or full duplex
      if(ptDriver->RxBuffer == 0)
      {
        volatile uint32_t tmp = ptDriver->ptDevice.ptSPI->spi_dr;
        UNUSED(tmp); // UNUSED
      }
      else
      {
        uint32_t ulTMP = ptDriver->ptDevice.ptSPI->spi_dr;
        switch (ptDriver->tConfiguration.eParallelism)
        {
        case DRV_SPI_PARALLELISM_2BIT:
        case DRV_SPI_PARALLELISM_4BIT:
        {
          ((uint32_t*) ptDriver->RxBuffer)[ptDriver->RxBufferCounter + i] = ulTMP;
          break;
        }
        default:
        {
          if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
          {
            ((uint16_t*) ptDriver->RxBuffer)[ptDriver->RxBufferCounter + i] = ulTMP;
          }
          else
          {
            ((uint8_t*) ptDriver->RxBuffer)[ptDriver->RxBufferCounter + i] = ulTMP;
          }
          break;
        }
        }
      }
    }
    ptDriver->RxBufferCounter += border;
  }
  // Hardware half or full duplex
  if(ptDriver->TxBufferCounter != ptDriver->TxBufferSize)
  {
    level = ptDriver->ptDevice.ptSPI->spi_sr_b.tx_fifo_level;
    if(level < ptDriver->tConfiguration.eTxFiFoRefillLevel)
    {
      border = DRV_SPI_FIFO_WM_MAX - level;
      if(border > ptDriver->tConfiguration.eTxFiFoRefillLevel)
      {
        border = ptDriver->tConfiguration.eTxFiFoRefillLevel - level;
      }
      if(ptDriver->TxBufferCounter + border >= ptDriver->TxBufferSize)
      {
        border = ptDriver->TxBufferSize - ptDriver->TxBufferCounter;
      }
      for(i = 0; i < border; i++)
      {
        // Software half or full duplex
        if(ptDriver->TxBuffer == 0)
        {
          ptDriver->ptDevice.ptSPI->spi_dr = ptDriver->tConfiguration.uDummyPattern;
        }
        else
        {
          switch (ptDriver->tConfiguration.eParallelism)
          {
          case DRV_SPI_PARALLELISM_2BIT:
          case DRV_SPI_PARALLELISM_4BIT:
          {
            ptDriver->ptDevice.ptSPI->spi_dr = ((uint32_t*) ptDriver->TxBuffer)[ptDriver->TxBufferCounter + i];
            break;
          }
          default:
          {
            if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
            {
              ptDriver->ptDevice.ptSPI->spi_dr = (uint32_t) ((uint16_t*) ptDriver->TxBuffer)[ptDriver->TxBufferCounter + i];
            }
            else
            {
              ptDriver->ptDevice.ptSPI->spi_dr = (uint32_t) ((uint8_t*) ptDriver->TxBuffer)[ptDriver->TxBufferCounter + i];
            }
            break;
          }
          }
        }
      }
      ptDriver->TxBufferCounter += border;
    }
  }

  return DRV_OK;
}

/*!
 * \brief This method shall perform a flush by polling mode.
 *
 * \details This method is the middle layer between the api calls and the hardware close flushing. It contains the polling method specific functionalities.
 * After calling the hardware near flushing function DRV_SPI_Flush_Buffers() and waiting, that all data has been flushed, it waits until the driver
 * has transmitted everything on the media and is finished. Then the FSS can be reset if it is software controlled and the software buffers can be
 * reset again.
 *
 * \private
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return  DRV_OK
 *          DRV_TOUT
 */
static DRV_STATUS_E DRV_SPI_Flush_POLL(DRV_SPI_HANDLE_T * const ptDriver)
{
  ptDriver->ullFrameStartTick = 0;
  DRV_STATUS_E ret = DRV_OK;
  while(ptDriver->TxBufferCounter < ptDriver->TxBufferSize || ptDriver->RxBufferCounter < ptDriver->RxBufferSize)
  {
    ret = DRV_SPI_Flush_Buffers(ptDriver);
    if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
    {
      ret = DRV_TOUT;
      break;
    }
  }
  while(ptDriver->ptDevice.ptSPI->spi_sr_b.BSY == 1 || ptDriver->ptDevice.ptSPI->spi_sr_b.RNE || !ptDriver->ptDevice.ptSPI->spi_sr_b.TFE)
  {
    if(ptDriver->ullFrameStartTick++ > ptDriver->tConfiguration.ulDriverTimeout)
    {
      ret = DRV_TOUT;
      break;
    }
  }
  if(ptDriver->tConfiguration.eFSSStatic == DRV_SPI_FSS_STATIC_DRIVER && ptDriver->tConfiguration.eBehaviour == DRV_SPI_BEHAVIOUR_MASTER)
  {
    ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
  }
  if(ptDriver->ullFrameStartTick < ptDriver->tConfiguration.ulDriverTimeout)
  {
    if(ptDriver->tConfiguration.fnCompleteCallback != 0)
    {
      ptDriver->tConfiguration.fnCompleteCallback(ptDriver, ptDriver->tConfiguration.pCompleteCallbackHandle);
    }
    ptDriver->TxBuffer = 0;
    ptDriver->TxBufferSize = 0;
    ptDriver->TxBufferCounter = 0;
    ptDriver->RxBuffer = 0;
    ptDriver->RxBufferSize = 0;
    ptDriver->RxBufferCounter = 0;
  }
  return ret;
}
/*!
 * \brief This method shall perform a flush by interrupt mode.
 *
 * \details It is the middle layer between the api calls and the flush itself. Because every flow is handled in the interrupt handler this method only modifies the interrupt masks.
 *
 * \private
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return  DRV_OK
 */
static DRV_STATUS_E DRV_SPI_Flush_IRQ(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_STATUS_E ret = DRV_OK;
  (void) DRV_IRQ_Disable();
  if((ptDriver->tConfiguration.eRxFiFoWm - 1u) < ptDriver->RxBufferSize - ptDriver->RxBufferCounter)
  {
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_wm = ptDriver->tConfiguration.eRxFiFoWm - 1;
  }
  else
  {
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_wm = ptDriver->RxBufferSize - ptDriver->RxBufferCounter - 1;
  }
  if(ptDriver->tConfiguration.eTxFiFoWm - 1u < ptDriver->TxBufferSize - ptDriver->TxBufferCounter)
  {
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_wm = ptDriver->tConfiguration.eTxFiFoWm - 1;
  }
  else
  {
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_wm = ptDriver->TxBufferSize - ptDriver->TxBufferCounter;
  }
  ptDriver->ptDevice.ptSPI->spi_imsc_b.RXIM = 1u; // Mask set and clear RW
  ptDriver->ptDevice.ptSPI->spi_imsc_b.TXIM = 1u; // Mask set and clear RW
  ptDriver->ptDevice.ptSPI->spi_imsc_b.RTIM = 1u; // Mask set and clear RW
  ptDriver->ptDevice.ptSQI->sqi_irq_mask_b.trans_end = 1u;
  (void) DRV_IRQ_Enable();
  return ret;
}

/*!
 * \brief This method shall perform a flush by dma mode.
 *
 * \details It is the middle layer between the api calls and the flush itself.
 *
 * \private
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return  DRV_ERROR_PARAM
 *          DRV_ERROR
 *          DRV_LOCKED
 *          DRV_BUSY
 *          DRV_OK
 */
static DRV_STATUS_E DRV_SPI_Flush_DMA(DRV_SPI_HANDLE_T * const ptDriver)
{
  if(ptDriver->RxBufferSize > 0x0FFFu || ptDriver->TxBufferSize > 0x0FFFu)
  {
    return DRV_ERROR_PARAM;
  }
  if(ptDriver->tConfiguration.ptSequencerTx == 0 || ptDriver->tConfiguration.ptSequencerRx == 0)
  {
    return DRV_ERROR_PARAM;
  }
  DRV_STATUS_E ret0 = DRV_OK;
  size_t ulBitCnt;
  switch (ptDriver->tConfiguration.eParallelism)
  {
  case DRV_SPI_PARALLELISM_1BIT:
    if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
    {
      ulBitCnt = 2;
    }
    else
    {
      ulBitCnt = 1;
    }
    break;
  case DRV_SPI_PARALLELISM_2BIT:
    ulBitCnt = 2;
    break;
  case DRV_SPI_PARALLELISM_4BIT:
    ulBitCnt = 4;
    break;
  default:
    return DRV_ERROR_PARAM;
  }
// Hardware half or full duplex
  if(ptDriver->RxBufferCounter != ptDriver->RxBufferSize)
  {
    // Software half or full duplex
    if(ptDriver->RxBuffer != 0)
    {
      ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eIncrementationDest = DRV_DMAC_INCREMENTATION_ACTIVE;
      ret0 = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerRx, (void * const ) &ptDriver->ptDevice.ptSPI->spi_dr, ptDriver->RxBuffer,
        ptDriver->RxBufferSize * ulBitCnt);
    }
    else
    {
      ptDriver->tConfiguration.ptSequencerRx->tConfiguration.eIncrementationDest = DRV_DMAC_INCREMENTATION_INACTIVE;
      ret0 = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerRx, (void * const ) &ptDriver->ptDevice.ptSPI->spi_dr,
        (void * const ) &ptDriver->RxBuffer, ptDriver->RxBufferSize * ulBitCnt);
    }
  }
  DRV_STATUS_E ret1 = DRV_OK;
// Hardware half or full duplex
  if(ptDriver->TxBufferCounter != ptDriver->TxBufferSize)
  {
    // Software half or full duplex
    if(ptDriver->TxBuffer != 0)
    {
      ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eIncrementationSource = DRV_DMAC_INCREMENTATION_ACTIVE;
      ret1 = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerTx, ptDriver->TxBuffer, (void * const ) &ptDriver->ptDevice.ptSPI->spi_dr,
        ptDriver->TxBufferSize * ulBitCnt);
    }
    else
    {
      ptDriver->tConfiguration.ptSequencerTx->tConfiguration.eIncrementationSource = DRV_DMAC_INCREMENTATION_INACTIVE;
      ret1 = DRV_DMAC_Start(ptDriver->tConfiguration.ptSequencerTx, (void * const ) &ptDriver->tConfiguration.uDummyPattern,
        (void * const ) &ptDriver->ptDevice.ptSPI->spi_dr, ptDriver->TxBufferSize * ulBitCnt);
    }
  }
  if(ret1 == ret0)
  {
    return ret0;
  }
  else
  {
    return DRV_ERROR;
  }
}

/*!
 * \details The Transmit function takes besides the this handle a data pointer and a size to perform the transmission of the given data.
 * It will check consistency of the handle, the locking of the driver api and checks if the driver is busy. After everything is
 * ready for transmission the software buffers are set and the FSS static is set if it is software controlled and the device is
 * master. At last, the flushing is performed regarding the chosen control flow method, the driver is unlocked and the function
 * returns.
 *
 * \public
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] pcData Pointer to the data to transmit
 * \param[in] size Amount of chars to transmit
 * \return  DRV_ERROR_PARAM
 *          DRV_ERROR_BUFFER
 *          DRV_ERROR
 *          DRV_LOCKED
 *          DRV_BUSY
 *          DRV_NSUPP
 *          DRV_NIMPL
 *          DRV_OK
 */
DRV_STATUS_E DRV_SPI_Transmit(DRV_SPI_HANDLE_T * const ptDriver, uint8_t* pcData, size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(pcData == 0 || size == 0 || size > 0x7ffff)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
  {
    if(size % 2)
    {
      DRV_UNLOCK(ptDriver);
      return DRV_ERROR_PARAM;
    }
  }
  DRV_STATUS_E ret = DRV_ERROR;
  if(DRV_OK == (ret = DRV_SPI_GetState(ptDriver, 0)))
  {
    size_t ulArraySize = size;
    switch (ptDriver->tConfiguration.eParallelism)
    {
    case DRV_SPI_PARALLELISM_1BIT:
    {
      if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
      {
        ulArraySize += (2 - ulArraySize) % 2;
        ulArraySize /= 2;
      }
      break;
    }
    case DRV_SPI_PARALLELISM_2BIT:
    case DRV_SPI_PARALLELISM_4BIT:
    {
      ulArraySize += (4 - ulArraySize) % 4;
      ulArraySize /= 4;
      break;
    }
    default:
    {
      break;
    }
    }
    ptDriver->TxBuffer = pcData;
    ptDriver->TxBufferSize = ulArraySize;
    ptDriver->TxBufferCounter = 0;
    if(ptDriver->tConfiguration.eDuplex == DRV_SPI_DUPLEX_FULL)
    {
      ptDriver->RxBuffer = 0;
      ptDriver->RxBufferSize = ulArraySize;
      ptDriver->RxBufferCounter = 0;
    }
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 0x01u;
    if(ptDriver->tConfiguration.eFSSStatic == DRV_SPI_FSS_STATIC_DRIVER && ptDriver->tConfiguration.eBehaviour == DRV_SPI_BEHAVIOUR_MASTER)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = ptDriver->tConfiguration.eFSS;
    }
    if(ptDriver->tConfiguration.eSPIDeviceID >= DRV_SPI_DEVICE_ID_SQI_BORDER && ptDriver->tConfiguration.eSPIDeviceID < DRV_SPI_DEVICE_ID_QSPI_BORDER)
    {

      ptDriver->ptDevice.ptSQI->sqi_tcr_b.mode = ptDriver->tConfiguration.eParallelism;
      if(ptDriver->tConfiguration.eDuplex == DRV_SPI_DUPLEX_FULL)
      {
        ptDriver->ptDevice.ptSQI->sqi_tcr_b.duplex = 3u;
        ptDriver->ptDevice.ptSQI->sqi_cr1_b.spi_trans_ctrl = 0u;
      }
      else if(ptDriver->tConfiguration.eDuplex == DRV_SPI_DUPLEX_HALF)
      {
        ptDriver->ptDevice.ptSQI->sqi_tcr_b.duplex = 2u;
        ptDriver->ptDevice.ptSQI->sqi_cr1_b.spi_trans_ctrl = 1u;
        if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
        {
          ptDriver->ptDevice.ptSQI->sqi_tcr_b.transfer_size = size / 2 - 1;
        }
        else
        {
          ptDriver->ptDevice.ptSQI->sqi_tcr_b.transfer_size = size - 1;
        }
        ptDriver->ptDevice.ptSQI->sqi_tcr_b.start_transfer = 1;
      }
      else
      {
        DRV_UNLOCK(ptDriver);
        return DRV_ERROR;
      }

    }
    if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ret = DRV_SPI_Flush_POLL(ptDriver);
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ret = DRV_SPI_Flush_IRQ(ptDriver);
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_SPI_Flush_DMA(ptDriver);
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
 * The receive function takes besides the this handle a data pointer and a size to perform the receiving of the given data.
 * It will check consistency of the handle, the lockage of the driver api and checks if the driver is busy. After everything is
 * ready for transmission the software buffers are configured and the FSS static is set if it is software controlled and the device is
 * master. At last, the flushing is performed regarding the chosen control flow method, the driver is unlocked and the function
 * returns.
 *
 * \warning In 2 and 4 byte parallelism the trailing bytes e.g. the fourth byte on a three byte receive might be overwritten.
 *
 * \public
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] pcData Pointer to the data to receive
 * \param[in] size Amount of chars to receive
 * \return  DRV_ERROR_PARAM
 *          DRV_ERROR_BUFFER
 *          DRV_ERROR
 *          DRV_LOCKED
 *          DRV_BUSY
 *          DRV_NSUPP
 *          DRV_NIMPL
 *          DRV_OK
 */
DRV_STATUS_E DRV_SPI_Receive(DRV_SPI_HANDLE_T * const ptDriver, uint8_t* pcData, size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  if(pcData == 0 || size == 0 || size > 0x7ffff)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
  {
    if(size % 2)
    {
      DRV_UNLOCK(ptDriver);
      return DRV_ERROR_PARAM;
    }
  }
  DRV_STATUS_E ret = DRV_ERROR;
  if(DRV_OK == (ret = DRV_SPI_GetState(ptDriver, 0)))
  {
    size_t ulArraySize = size;
    switch (ptDriver->tConfiguration.eParallelism)
    {
    case DRV_SPI_PARALLELISM_1BIT:
    {
      if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
      {
        ulArraySize += (2 - ulArraySize) % 2;
        ulArraySize /= 2;
      }
      break;
    }
    case DRV_SPI_PARALLELISM_2BIT:
    case DRV_SPI_PARALLELISM_4BIT:
    {
      ulArraySize += (4 - ulArraySize) % 4;
      ulArraySize /= 4;
      break;
    }
    default:
    {
      break;
    }
    }
    ptDriver->RxBuffer = pcData;
    ptDriver->RxBufferSize = ulArraySize;
    ptDriver->RxBufferCounter = 0;
    if(ptDriver->tConfiguration.eDuplex == DRV_SPI_DUPLEX_FULL)
    {
      ptDriver->TxBuffer = 0;
      ptDriver->TxBufferSize = ulArraySize;
      ptDriver->TxBufferCounter = 0;
    }
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 0x01u;
    if(ptDriver->tConfiguration.eFSSStatic == DRV_SPI_FSS_STATIC_DRIVER && ptDriver->tConfiguration.eBehaviour == DRV_SPI_BEHAVIOUR_MASTER)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = ptDriver->tConfiguration.eFSS;
    }
    if(ptDriver->tConfiguration.eSPIDeviceID >= DRV_SPI_DEVICE_ID_SQI_BORDER && ptDriver->tConfiguration.eSPIDeviceID < DRV_SPI_DEVICE_ID_QSPI_BORDER)
    {

      ptDriver->ptDevice.ptSQI->sqi_tcr_b.mode = ptDriver->tConfiguration.eParallelism;
      if(ptDriver->tConfiguration.eDuplex == DRV_SPI_DUPLEX_FULL)
      {
        ptDriver->ptDevice.ptSQI->sqi_tcr_b.duplex = 3u;
        ptDriver->ptDevice.ptSQI->sqi_cr1_b.spi_trans_ctrl = 0u;
      }
      else if(ptDriver->tConfiguration.eDuplex == DRV_SPI_DUPLEX_HALF)
      {
        ptDriver->ptDevice.ptSQI->sqi_tcr_b.duplex = 1u;
        ptDriver->ptDevice.ptSQI->sqi_cr1_b.spi_trans_ctrl = 1u;
        if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
        {
          ptDriver->ptDevice.ptSQI->sqi_tcr_b.transfer_size = size / 2 - 1;
        }
        else
        {
          ptDriver->ptDevice.ptSQI->sqi_tcr_b.transfer_size = size - 1;
        }
        ptDriver->ptDevice.ptSQI->sqi_tcr_b.start_transfer = 1;
      }
      else
      {
        DRV_UNLOCK(ptDriver);
        return DRV_ERROR;
      }

    }
    if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ret = DRV_SPI_Flush_POLL(ptDriver);
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ret = DRV_SPI_Flush_IRQ(ptDriver);
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_SPI_Flush_DMA(ptDriver);
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
 * The receive and transmit function takes besides the this handle a data pointer and a size to perform the receiving and transmitting
 * of the given data. It will check consistency of the handle, the lockage of the driver api and checks if the driver is busy. After
 * everything is ready for transmission the software buffers are configured and the FSS static is set if it is software controlled and
 * the device is master. At last, the flushing is performed regarding the chosen control flow method, the driver is unlocked and the
 * function returns.
 *
 * \public
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[out] pcTxData Pointer to the data to transmit
 * \param[in] pcRxData Pointer to the data to receive
 * \param[in] size Amount of chars to receive
 * \return  DRV_ERROR_PARAM
 *          DRV_ERROR_BUFFER
 *          DRV_ERROR
 *          DRV_LOCKED
 *          DRV_BUSY
 *          DRV_NSUPP
 *          DRV_NIMPL
 *          DRV_OK
 */
DRV_STATUS_E DRV_SPI_TransmitReceive(DRV_SPI_HANDLE_T * const ptDriver, uint8_t* pcTxData, uint8_t* pcRxData, size_t size)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_STATUS_E ret = DRV_OK;
  if(ptDriver->tConfiguration.eDuplex != DRV_SPI_DUPLEX_FULL)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_NSUPP;
  }
  if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
  {
    if(size % 2)
    {
      DRV_UNLOCK(ptDriver);
      return DRV_ERROR_PARAM;
    }
  }
  if(pcTxData == 0 || pcRxData == 0 || size == 0 || size > 0x7ffff)
  {
    DRV_UNLOCK(ptDriver);
    return DRV_ERROR_PARAM;
  }
  if(DRV_OK == (ret = DRV_SPI_GetState(ptDriver, 0)))
  {
    size_t ulArraySize = size;
    if(ptDriver->tConfiguration.eDataSize > DRV_SPI_DATA_SIZE_SELECT_8b)
    {
      ulArraySize += (2 - ulArraySize) % 2;
      ulArraySize /= 2;
    }
    ptDriver->TxBuffer = pcTxData;
    ptDriver->TxBufferSize = ulArraySize;
    ptDriver->TxBufferCounter = 0;
    ptDriver->RxBuffer = pcRxData;
    ptDriver->RxBufferSize = ulArraySize;
    ptDriver->RxBufferCounter = 0;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 0x01u;
    ptDriver->ptDevice.ptSQI->sqi_cr1_b.spi_trans_ctrl = 0u;

    if(ptDriver->tConfiguration.eFSSStatic == DRV_SPI_FSS_STATIC_DRIVER && ptDriver->tConfiguration.eBehaviour == DRV_SPI_BEHAVIOUR_MASTER)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = ptDriver->tConfiguration.eFSS;
    }
    if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
    {
      ret = DRV_SPI_Flush_POLL(ptDriver);
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
    {
      ret = DRV_SPI_Flush_IRQ(ptDriver);
    }
    else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
    {
      ret = DRV_SPI_Flush_DMA(ptDriver);
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
 * The change fss function checks only if the driver is not locked and the device is not
 * busy. If that is true the configuration attribute is changed like in the init method
 * specified.
 *
 * \public
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \param[in] eFss The new FSS state
 * \return  DRV_ERROR_PARAM
 *          DRV_ERROR
 *          DRV_LOCKED
 *          DRV_BUSY
 *          DRV_NSUPP
 *          DRV_NIMPL
 *          DRV_OK
 */
DRV_STATUS_E DRV_SPI_ChangeFss(DRV_SPI_HANDLE_T * const ptDriver, DRV_SPI_FSS_E eFss)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_STATUS_E ret = DRV_ERROR;
  if(DRV_OK == (ret = DRV_SPI_GetState(ptDriver, 0)))
  {
    ptDriver->tConfiguration.eFSS = eFss;
    if(ptDriver->tConfiguration.eFSSStatic == DRV_SPI_FSS_STATIC_DRIVER)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
    }
    else
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = ptDriver->tConfiguration.eFSS;
    }
    ret = DRV_OK;
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The abort function shall end pending interactions and reset any transaction.
 * It forces the bus speed to zero and stops the device first, then sets the FSS to zero so that no
 * peripheral is selected and masks all interrupts out or if dma is used calls the dma abort functionality.
 * All buffers will then be reset and the device deactivated to reset its internal states.
 * At last, the FSS and and bus speeds are set as configured again and the device is activated again.
 *
 * \public
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return  DRV_ERROR_PARAM
 *          DRV_LOCKED
 *          DRV_NSUPP
 *          DRV_OK
 */
DRV_STATUS_E DRV_SPI_Abort(DRV_SPI_HANDLE_T * const ptDriver)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_LOCK(ptDriver);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_POLL)
  {
    ptDriver->ptDevice.ptSPI->spi_cr0_b.sck_muladd = DRV_SPI_FREQUENCY_STOPPED;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.SSE = 0;
    ptDriver->TxBuffer = 0;
    ptDriver->TxBufferSize = 0;
    ptDriver->TxBufferCounter = 0;
    ptDriver->RxBuffer = 0;
    ptDriver->RxBufferSize = 0;
    ptDriver->RxBufferCounter = 0;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 1;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 1;
    ptDriver->ptDevice.ptSPI->spi_cr0_b.sck_muladd = ptDriver->tConfiguration.eFrequency;
    if(ptDriver->tConfiguration.eFSSStatic != DRV_SPI_FSS_STATIC_DRIVER)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = ptDriver->tConfiguration.eFSS;
    }
    ptDriver->ptDevice.ptSPI->spi_cr1_b.SSE = 1;
    ret = DRV_OK;
  }
  else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_IRQ)
  {
    ptDriver->ptDevice.ptSPI->spi_cr0_b.sck_muladd = DRV_SPI_FREQUENCY_STOPPED;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.SSE = 0;
    ptDriver->ptDevice.ptSPI->spi_imsc = 0;
    ptDriver->ptDevice.ptSPI->spi_icr = 0xFFFFFFFF;
    DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]);
    ptDriver->TxBuffer = 0;
    ptDriver->TxBufferSize = 0;
    ptDriver->TxBufferCounter = 0;
    ptDriver->RxBuffer = 0;
    ptDriver->RxBufferSize = 0;
    ptDriver->RxBufferCounter = 0;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 1;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 1;
    ptDriver->ptDevice.ptSPI->spi_cr0_b.sck_muladd = ptDriver->tConfiguration.eFrequency;
    if(ptDriver->tConfiguration.eFSSStatic != DRV_SPI_FSS_STATIC_DRIVER)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = ptDriver->tConfiguration.eFSS;
    }
    DRV_NVIC_ClearPendingIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]);
    DRV_NVIC_EnableIRQ(s_apHandleIRQnTable[(uint32_t) ptDriver->tConfiguration.eSPIDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN]);
    ptDriver->ptDevice.ptSPI->spi_cr1_b.SSE = 1;
    ret = DRV_OK;
  }
  else if(ptDriver->tConfiguration.eOperationMode == DRV_OPERATION_MODE_DMA)
  {
    ret = DRV_OK;
    ptDriver->ptDevice.ptSPI->spi_cr0_b.sck_muladd = DRV_SPI_FREQUENCY_STOPPED;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.SSE = 0;
    ptDriver->ptDevice.ptSPI->spi_imsc = 0;
    ptDriver->ptDevice.ptSPI->spi_icr = 0xFFFFFFFF;
    if(DRV_DMAC_Abort(ptDriver->tConfiguration.ptSequencerRx) != (ret = DRV_DMAC_Abort(ptDriver->tConfiguration.ptSequencerTx)))
    {
      ret = DRV_ERROR;
    }
    ptDriver->TxBuffer = 0;
    ptDriver->TxBufferSize = 0;
    ptDriver->TxBufferCounter = 0;
    ptDriver->RxBuffer = 0;
    ptDriver->RxBufferSize = 0;
    ptDriver->RxBufferCounter = 0;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_clr = 1;
    ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_clr = 1;
    ptDriver->ptDevice.ptSPI->spi_cr0_b.sck_muladd = ptDriver->tConfiguration.eFrequency;
    if(ptDriver->tConfiguration.eFSSStatic != DRV_SPI_FSS_STATIC_DRIVER)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = ptDriver->tConfiguration.eFSS;
    }
    ptDriver->ptDevice.ptSPI->spi_cr1_b.SSE = 1;
  }
  else
  {
    ret = DRV_ERROR_PARAM;
  }
  DRV_UNLOCK(ptDriver);
  return ret;
}

/*!
 * The state of the driver is written to the ptSpiState pointer and an indicating status of the driver is returned.
 * The returned status is the same as the other functions use to indicate a correct configure and idle driver and
 * device ready for transmission. Because the error states depend mostly on the used transaction the spi state has
 * to be checked by the caller. If a Tx buffer overflow occurs the function will return a DRV_ERROR because it is
 * always an error.
 *
 * Tx data has to be written to the spi device in master mode for reading data and TxBuffer underruns might occur.
 * Rx data can be discarded in master mode for sending only but a RxBuffer overflow is produced.
 * Rx data can be discarded in slave mode for sending only but a RxBuffer overflow is produced.
 * Tx data might not be provided if a slave is reading only but a buffer underrun is produced.
 *
 * \public
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class.
 * \param[out] ptState Pointer to a DRV_SPI_STATE_E where the state is written to.
 * \return  DRV_ERROR_PARAM
 *          DRV_ERROR_BUFFER
 *          DRV_BUSY
 *          DRV_OK
 */
DRV_STATUS_E DRV_SPI_GetState(DRV_SPI_HANDLE_T * const ptDriver, DRV_SPI_STATE_E * const ptState)
{
  DRV_HANDLE_CHECK(ptDriver);
  DRV_STATUS_E ret = DRV_ERROR;
  if(ptState != 0)
  {
    *ptState = (DRV_SPI_STATE_E) ptDriver->ptDevice.ptSPI->spi_sr;
  }
  if(ptDriver->ptDevice.ptSPI->spi_sr_b.BSY == 1 || ptDriver->TxBuffer != 0 || ptDriver->TxBufferSize != 0 || ptDriver->TxBufferCounter != 0
    || ptDriver->RxBuffer != 0 || ptDriver->RxBufferSize != 0 || ptDriver->RxBufferCounter != 0)
  {
    ret = DRV_BUSY;
  }
  else
  {
    ret = DRV_OK;
  }
  if((ptDriver->ptDevice.ptSPI->spi_sr
    & ((uint32_t) DRV_SPI_STATE_RX_FIFO_OVERFLOW | (uint32_t) DRV_SPI_STATE_RX_FIFO_UNDERRUN | (uint32_t) DRV_SPI_STATE_TX_FIFO_OVERFLOW
      | (uint32_t) DRV_SPI_STATE_TX_FIFO_UNDERRUN)) != 0)
  {
    ret = DRV_ERROR_BUFFER;
  }
  return ret;
}

/*!
 * This function is the true interrupt service routine or interrupt handler of the SPI device interrupts. The interrupt handlers called by the
 * controller should inline this handler and the constant eSPIDeviceID should define the specific device in compile time. For debug and development
 * purposes a check of the pointers is performed in front that is not necessary for release. It is necessary because a debugger stopped controller
 * does not perform a reset of the interrupt vector and so the ISR shall cope with the not initialized handle pointer.
 *
 * After that the buffer flush is performed to transmit and receive data pending and then checks for the frame end are performed.
 * If the frame has ended, the buffers and the FFS (if static) are reset.
 *
 * \private
 * \memberof DRV_SPI_HANDLE_T
 * \param[in,out] eDeviceID The given id of the drivers class
 * \return void
 */
__STATIC_INLINE void DRV_SPI_IRQ_Inline_Handler(DRV_SPI_DEVICE_ID_E const eDeviceID)
{
  uint32_t ulDeviceOffset = (uint32_t) eDeviceID - (uint32_t) DRV_SPI_DEVICE_ID_MIN;
  DRV_SPI_HANDLE_T * const ptDriver = s_apHandleAddressTable[ulDeviceOffset];
#ifndef NDEBUG
  if(ptDriver->ptDevice.pt != s_apDeviceAddressTable[ulDeviceOffset].pt)
  {
    ((DRV_SPI_DEVICE_U const) s_apDeviceAddressTable[ulDeviceOffset]).ptSPI->spi_imsc = 0;
    DRV_NVIC_DisableIRQ(s_apHandleIRQnTable[ulDeviceOffset]);
    return;
  }
#endif
  DRV_SPI_Flush_Buffers(ptDriver); //lint !e534
  if(ptDriver->RxBufferCounter == ptDriver->RxBufferSize)
  {
    ptDriver->ptDevice.ptSPI->spi_imsc_b.RXIM = 0; // Mask set and clear RW
  }
  else
  {
    if(ptDriver->tConfiguration.eRxFiFoWm - 1u > ptDriver->RxBufferSize - ptDriver->RxBufferCounter)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.rx_fifo_wm = ptDriver->RxBufferSize - ptDriver->RxBufferCounter - 1;
    }
  }
  if(ptDriver->TxBufferCounter == ptDriver->TxBufferSize)
  {
    ptDriver->ptDevice.ptSPI->spi_imsc_b.TXIM = 0; // Mask set and clear RW
  }
  else
  {
    if(ptDriver->tConfiguration.eTxFiFoWm - 1u > ptDriver->TxBufferSize - ptDriver->TxBufferCounter)
    {
      ptDriver->ptDevice.ptSPI->spi_cr1_b.tx_fifo_wm = ptDriver->TxBufferSize - ptDriver->TxBufferCounter;
    }
  }
  if((ptDriver->RxBufferCounter == ptDriver->RxBufferSize) &&
      (ptDriver->TxBufferCounter == ptDriver->TxBufferSize))
  {
    DRV_SPI_WaitNotBusy(ptDriver);

    if(ptDriver->ptDevice.ptSPI->spi_sr_b.BSY != 1)
    {
      if(ptDriver->tConfiguration.eBehaviour == DRV_SPI_BEHAVIOUR_MASTER)
      {
        if(ptDriver->tConfiguration.eFSSStatic == DRV_SPI_FSS_STATIC_DRIVER)
        {
          ptDriver->ptDevice.ptSPI->spi_cr1_b.fss = DRV_SPI_FSS_NONE;
        }
      }
      if(ptDriver->tConfiguration.fnCompleteCallback != 0)
      {
        ptDriver->tConfiguration.fnCompleteCallback(ptDriver, ptDriver->tConfiguration.pCompleteCallbackHandle);
      }
      ptDriver->TxBuffer = 0;
      ptDriver->TxBufferSize = 0;
      ptDriver->TxBufferCounter = 0;
      ptDriver->RxBuffer = 0;
      ptDriver->RxBufferSize = 0;
      ptDriver->RxBufferCounter = 0;
      ptDriver->ptDevice.ptSPI->spi_imsc = 0;
    }
  }
  ptDriver->ptDevice.ptSPI->spi_icr = 0xFFFFFFFFul;
}

/*!
 * The generator define for generating irq handler source code.
 */
#define DRV_SPI_IRQHandler_Generator(id, _) DRV_Default_IRQHandler_Function_Generator(DRV_SPI_IRQ_HANDLER ## id,DRV_SPI_IRQ_Inline_Handler,(DRV_SPI_DEVICE_ID_E)(DRV_SPI_DEVICE_ID_MIN + id))

/*!
 * The define to where the irq handler code shall be generated.
 */
/*lint -save -e123 */
DRV_DEF_REPEAT_EVAL(DRV_SPI_DEVICE_COUNT, DRV_SPI_IRQHandler_Generator, ~)
/*lint -restore */


__STATIC_INLINE void DRV_SPI_WaitNotBusy(DRV_SPI_HANDLE_T* const ptDriver)
{
  uint32_t ulTimeout = DRV_SPI_WAIT_SPI_NOT_BUSY_TIMEOUT_MS * (SystemCoreClock / 1000);

  while((ptDriver->ptDevice.ptSPI->spi_sr_b.BSY == 1) &&
        (ulTimeout > 0))
  {
    ulTimeout--;
  }
}

/*! \} *//* End of group GPIO */

#endif /* DRV_SPI_MODULE_DISABLED */
