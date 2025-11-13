/*!************************************************************************//*!
 * \file     netx_drv_uart.h
 * \brief     peripheral module driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#include "netx_drv.h"
#ifndef __NETX90_UART_H
#define __NETX90_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*!
 * \addtogroup UART
 * \{
 */

/*!***************************************************************************/


/*!
 * \brief Enumeration of useful uart baudrate mode.
 */
typedef enum DRV_UART_BAUDRATEMODE_Etag
{
  DRV_UART_BAUDRATEMODE_RESET = 0x00u, /*!< Reset value */
  DRV_UART_BAUDRATEMODE_0 = 0x01u, /*!< Obsolete, as it gives inaccuracies at higher baudrates */
  DRV_UART_BAUDRATEMODE_1 = 0x02u,  /*!< Calculate BaudDiv by BAUDDIV = (Baud Rate * 16 * 65536) / 100 MHz */
  DRV_UART_BAUDRATEMODE_DEFAULT = DRV_UART_BAUDRATEMODE_1,  /*!< Default maps to mode 1 */
  DRV_UART_BAUDRATEMODE_MIN = 0x01u,  /*!< Minimal value */
  DRV_UART_BAUDRATEMODE_MAX = 0x02u  /*!< Maximum value */
} DRV_UART_BAUDRATEMODE_E;

/*!
 * \brief Enumeration of useful uart baudrates.
 */
typedef enum DRV_UART_BAUDRATE_Etag
{
  DRV_UART_BAUDRATE_DEFAULT = 0ul,/*!< Baudrate of 0 */
  DRV_UART_BAUDRATE_300 = 3ul,/*!< Baudrate of 300 */
  DRV_UART_BAUDRATE_600 = 6ul,/*!< Baudrate of 600 */
  DRV_UART_BAUDRATE_1200 = 12ul,/*!< Baudrate of 1200 */
  DRV_UART_BAUDRATE_2400 = 24ul,/*!< Baudrate of 2400 */
  DRV_UART_BAUDRATE_4800 = 48ul,/*!< Baudrate of 4800 */
  DRV_UART_BAUDRATE_9600 = 96ul,/*!< Baudrate of 9600 */
  DRV_UART_BAUDRATE_19200 = 192ul,/*!< Baudrate of 19200 */
  DRV_UART_BAUDRATE_38400 = 384ul,/*!< Baudrate of 38400 */
  DRV_UART_BAUDRATE_57600 = 576ul,/*!< Baudrate of 57600 */
  DRV_UART_BAUDRATE_115200 = 1152ul,/*!< Baudrate of 115200 */
  DRV_UART_BAUDRATE_460800 = 4608ul,/*!< Baudrate of 460800 */
  DRV_UART_BAUDRATE_921600   = 9216ul,  /*!< Baudrate of 921600 */
  DRV_UART_BAUDRATE_1000000  = 10000ul, /*!< Baudrate of 1000000 */
  DRV_UART_BAUDRATE_2000000  = 20000ul, /*!< Baudrate of 2000000 */
  DRV_UART_BAUDRATE_3000000  = 30000ul, /*!< Baudrate of 3000000 */
  DRV_UART_BAUDRATE_3125000  = 31250ul, /*!< Baudrate of 3125000 */
  DRV_UART_BAUDRATE_4000000  = 40000ul, /*!< Baudrate of 4000000 */
  DRV_UART_BAUDRATE_5000000  = 50000ul, /*!< Baudrate of 5000000 */
  DRV_UART_BAUDRATE_6000000  = 60000ul, /*!< Baudrate of 6000000 */
  DRV_UART_BAUDRATE_6250000  = 62500ul, /*!< Baudrate of 6250000 */
  DRV_UART_BAUDRATE_MAX      = DRV_UART_BAUDRATE_6250000,/*!< Baudrates over 6250000 are not allowed */
} DRV_UART_BAUDRATE_E;

/*!
 * \brief Enumeration of uart device states.
 */
typedef enum DRV_UART_STATE_Etag
{
  DRV_UART_STATE_CTS = 0x00000001ul, /*!< CTS */
  DRV_UART_STATE_DSR = 0x00000002ul, /*!< DSR */
  DRV_UART_STATE_DCD = 0x00000004ul, /*!< DCD */
  DRV_UART_STATE_BUSY = 0x00000008ul, /*!< BUSY */
  DRV_UART_STATE_RXFE = 0x00000010ul, /*!< Rx fifo empty */
  DRV_UART_STATE_TXFF = 0x00000020ul, /*!< Tx fifo full */
  DRV_UART_STATE_RXFF = 0x00000040ul, /*!< Rx fifo full */
  DRV_UART_STATE_TXFE = 0x00000080ul, /*!< Rx fifo empty */
  DRV_UART_STATE_ERROR_FRAMING = 0x00010000ul, /*!< Framing Error */
  DRV_UART_STATE_ERROR_PARITY = 0x00020000ul, /*!< Parity Error */
  DRV_UART_STATE_ERROR_BREAK = 0x00040000ul, /*!< Break Error */
  DRV_UART_STATE_ERROR_OVERRUN = 0x00080000ul, /*!< Overrun Error */
} DRV_UART_STATE_E;

/*!
 * \brief Enumeration of the available uart word lenghts.
 */
typedef enum DRV_UART_WORD_LENGTH_Etag
{
  DRV_UART_WORD_LENGTH_RESET = 0x00ul, /*!< Default uart word lenght */
  DRV_UART_WORD_LENGTH_5_BITS = 0x10ul, /*!< Word length of 5 bits */
  DRV_UART_WORD_LENGTH_6_BITS = 0x11ul, /*!< Word length of 6 bits */
  DRV_UART_WORD_LENGTH_7_BITS = 0x12ul, /*!< Word length of 7 bits */
  DRV_UART_WORD_LENGTH_8_BITS = 0x13ul, /*!< Word length of 8 bits */
  DRV_UART_WORD_LENGTH_DEFAULT = DRV_UART_WORD_LENGTH_8_BITS, /*!< Default uart word lenght */
  DRV_UART_WORD_LENGTH_MIN = DRV_UART_WORD_LENGTH_5_BITS, /*!< Minimum used for boundary checks */
  DRV_UART_WORD_LENGTH_MAX = DRV_UART_WORD_LENGTH_8_BITS, /*!< Maximum used for boundary checks */
} DRV_UART_WORD_LENGTH_E;

/*!
 * \brief Enumeration of of the line control parameters as mask.
 */
typedef enum DRV_UART_LINE_CONTROL_MASK_Etag
{
  DRV_UART_LINE_CONTROL_MASK_FIFO_ENABLE = 0x10ul, /*!< Enables the fifos. */
  DRV_UART_LINE_CONTROL_MASK_ADDITIONAL_STOP_BIT = 0x08ul, /*!< Will generate two stop bits instead of one. */
  DRV_UART_LINE_CONTROL_MASK_EVEN_PARITY = 0x06ul, /*!< Use even parity. */
  DRV_UART_LINE_CONTROL_MASK_ODD_PARITY = 0x02ul, /*!< Activate parity. */
  DRV_UART_LINE_CONTROL_MASK_SEND_BREAK = 0x01ul, /*!< Break in sending. */
  DRV_UART_LINE_CONTROL_MASK_DEFAULT = 0x00ul, /*!< Nothing */
  DRV_UART_LINE_CONTROL_MASK = 0x1ful, /*!< Masks bits choseable. */
} DRV_UART_LINE_CONTROL_MASK_E;

/*!
 * \brief Enumeration of the loop back state.
 */
typedef enum DRV_UART_LOOP_BACK_Etag
{
  DRV_UART_LOOP_BACK_INACTIVE = 0x00ul, /*!< Loopback is inactive */
  DRV_UART_LOOP_BACK_ACTIVE = 0x01ul, /*!< Loopback is active for testing purposes only */
  DRV_UART_LOOP_BACK_MAX = DRV_UART_LOOP_BACK_ACTIVE, /*!< Boundary check value */
} DRV_UART_LOOP_BACK_E;

/*!
 * \brief Enumeration of the loop back state.
 */
typedef enum DRV_UART_WATERMARK_Etag
{
  DRV_UART_WATERMARK_UNINITIALIZED = 0x00ul, /*!< Watermark of 8 */
  DRV_UART_WATERMARK_1 = 0x01ul, /*!< Watermark of 1 */
  DRV_UART_WATERMARK_2 = 0x02ul, /*!< Watermark of 2  */
  DRV_UART_WATERMARK_3 = 0x03ul, /*!< Watermark of 3  */
  DRV_UART_WATERMARK_4 = 0x04ul, /*!< Watermark of 4  */
  DRV_UART_WATERMARK_5 = 0x05ul, /*!< Watermark of 5  */
  DRV_UART_WATERMARK_6 = 0x06ul, /*!< Watermark of 6  */
  DRV_UART_WATERMARK_7 = 0x07ul, /*!< Watermark of 7  */
  DRV_UART_WATERMARK_8 = 0x08ul, /*!< Watermark of 8  */
  DRV_UART_WATERMARK_DEFAULT = DRV_UART_WATERMARK_8, /*!< Watermark default value*/
  DRV_UART_WATERMARK_9 = 0x09ul, /*!< Watermark of 9  */
  DRV_UART_WATERMARK_10 = 0x0aul, /*!< Watermark of 10 */
  DRV_UART_WATERMARK_11 = 0x0bul, /*!< Watermark of 11 */
  DRV_UART_WATERMARK_12 = 0x0cul, /*!< Watermark of 12 */
  DRV_UART_WATERMARK_13 = 0x0dul, /*!< Watermark of 13 */
  DRV_UART_WATERMARK_14 = 0x0eul, /*!< Watermark of 14 */
  DRV_UART_WATERMARK_15 = 0x0ful, /*!< Watermark of 15 */
  DRV_UART_WATERMARK_16 = 0x10ul, /*!< Watermark of 16 */
  DRV_UART_WATERMARK_MIN = DRV_UART_WATERMARK_1, /*!< Boundary check value */
  DRV_UART_WATERMARK_MAX = DRV_UART_WATERMARK_16, /*!< Boundary check value */
} DRV_UART_WATERMARK_E;

/*!
 * \brief Enumeration of the rts control parameters as mask.
 */
typedef enum DRV_UART_RTS_CONTROL_MASK_Etag
{
  DRV_UART_RTS_CONTROL_MASK_STICK_PARITY = 0x80ul, /*!< Stick parity is used. */
  DRV_UART_RTS_CONTROL_MASK_CTS_ACTIVE_HIGH = 0x40ul, /*!< CTS is active high. */
  DRV_UART_RTS_CONTROL_MASK_CTS_ENABLE = 0x20ul, /*!< CTS will be enabled. */
  DRV_UART_RTS_CONTROL_MASK_RTS_ACTIVE_HIGH = 0x10ul, /*!< RTS is active high. */
  DRV_UART_RTS_CONTROL_MASK_ALTERNATIVE_MODE = 0x08ul, /*!< Mode2 is selected. */
  DRV_UART_RTS_CONTROL_MASK_USE_SYSTEM_CLOCK = 0x04ul, /*!< The system clock is used instead of the baud rate generator. (max speed) */
  DRV_UART_RTS_CONTROL_MASK_RTS_CONTROL_BIT_SET = 0x02ul, /*!< RTS is controled by this bit. */
  DRV_UART_RTS_CONTROL_MASK_RTS_AUTOMATIC = 0x01ul, /*!< The RTS is hardware or driver driven. */
  DRV_UART_RTS_CONTROL_MASK = 0xfful, /*!< Masks bits choseable. */
} DRV_UART_RTS_CONTROL_MASK_E;

/*!
 * \brief Enumeration of the transmit modes availabe as mask.
 */
typedef enum DRV_UART_TX_MODE_MASK_Etag
{
  DRV_UART_TX_MODE_MASK_DEFAULT = 0x00ul, /*!< Transmit is active. */
  DRV_UART_TX_MODE_MASK_RECEIVE_ONLY = 0x01ul, /*!< Receive only. */
  DRV_UART_TX_MODE_MASK_RTS_ACTIVE = 0x02ul, /*!< Use RTS. */
  DRV_UART_TX_MODE_MASK = 0x03ul, /*!< Masks bits choseable. */
} DRV_UART_TX_MODE_MASK_E;

/*!
 * \brief The configuration of the driver.
 *
 * The configuration SHALL be changed before initializing the device and shall not be changed
 * afterwards.
 */
typedef struct Drv_UART_CONFIGURATION_Ttag
{
  DRV_UART_DEVICE_ID_E eDeviceID; /*!< The device to be used*/
  DRV_OPERATION_MODE_E eOperationMode; /*!< Which programming method (DMA/IRQ/POLL) is used.*/
  DRV_UART_TX_MODE_MASK_E eTxMode; /*!< What mode is chosen. */
  DRV_UART_BAUDRATE_E eBaudrate; /*!< The baudrate to be used. */
  DRV_UART_BAUDRATEMODE_E Baud_Rate_Mode; /*!< Kind of baud div.
                                           0: BAUDDIV = ( 100 MHz / (16 * BaudRate) ) – 1
                                              This formula is obsolete, as it gives inaccuracies at higher baudrates
                                           1: BAUDDIV = ( (Baud Rate * 16) / 100 MHz ) * 2^16
                                              This formula is better, so it will be used in all the cases */
  DRV_UART_WORD_LENGTH_E eWordLength; /*!< The length of the words transmitted. */
  DRV_UART_LINE_CONTROL_MASK_E eLineControl; /*!< The line control options. */
  DRV_UART_LOOP_BACK_E eLoopBack; /*!< If a loopback will be performed. */
  DRV_UART_RTS_CONTROL_MASK_E eRTSControl; /*!< The rts control options. */
  DRV_UART_WATERMARK_E eTxFifoWatermark; /*!< The transmit fifo Watermark level. */
  DRV_UART_WATERMARK_E eRxFifoWatermark; /*!< The receive fifo Watermark level. */
  DRV_DMAC_DEVICE_ID_E eDMATx;/*!< The device id of the dmac to be used for tx.*/
  DRV_DMAC_DEVICE_ID_E eDMARx;/*!< The device id of the dmac to be used for rx.*/
  DRV_DMAC_HANDLE_T* ptSequencerTx;/*!< The allocated and initialized to 0 handle of the tx dmac. */
  DRV_DMAC_HANDLE_T* ptSequencerRx;/*!< The allocated and initialized to 0 handle of the rx dmac. */
  DRV_CALLBACK_F fnRxTimeoutCallback; /*!< The callback used if the rx timeout event has occurred.*/
  void* pRxTimeoutCallbackHandle; /*!< The handle associated with the rx timeout callback.*/
  DRV_CALLBACK_F fnRxCompleteCallback; /*!< The callback used if transaction is completed.*/
  void* pRxCompleteCallbackHandle; /*!< The handle associated with the complete callback.*/
  DRV_CALLBACK_F fnTxCompleteCallback; /*!< The callback used if transaction is completed.*/
  void* pTxCompleteCallbackHandle; /*!< The handle associated with the complete callback.*/
  DRV_CALLBACK_F fnRxCallback; /*!< The callback used if the rx has received data, but no buffer is specified.*/
  void* pRxCallbackHandle; /*!< The handle associated with the rx callback.*/
  uint32_t ulDriverTimeout; /*!< Timeout limit, used in the driver for polling contexts. A software counter is used. */
} DRV_UART_CONFIGURATION_T;

/*!
 * \brief The handle of the driver.
 *
 * The configuration SHALL be changed before initializing the device and shall not be changed
 * afterwards.
 * The rest of it SHALL not be modified outside of the driver, even if it appears to be possible.
 * \struct DRV_UART_HANDLE_T
 * \struct DRV_UART_HANDLE_Ttag
 */
typedef struct DRV_UART_HANDLE_Ttag
{
  DRV_UART_DEVICE_T* ptDevice; /*!< \private The UART device register as bitfield and value unions. */
  DRV_UART_CONFIGURATION_T tConfiguration; /*!< \private Contains the configuration of the device. */
  DRV_LOCK_T tLock; /*!< \private The drivers locking variable used as internal mutex*/
  uint64_t ullFrameStartTick;/*!< \private Used for timeout detection. It is a software counter, not a timer related one. */
  void * volatile TxBuffer;/*!< \private Transmit buffer */
  size_t volatile TxBufferSize;/*!< \private Transmit size */
  size_t volatile TxBufferCounter;/*!< \private Transmit counter */
  void * volatile RxBuffer;/*!< \private Receive buffer */
  uint8_t volatile RxBufferStatic[16];/*!< \private Receive buffer */
  size_t volatile RxBufferSize;/*!< \private Receive size */
  size_t volatile RxBufferCounter;/*!< \private Receive counter */
} DRV_UART_HANDLE_T;

/*!
 * \brief Initializes the uart device and handle by the given configuration.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_Init(DRV_UART_HANDLE_T * const ptDriver);
/*!
 * \brief Deinitializes the uart device and handle.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_DeInit(DRV_UART_HANDLE_T * const ptDriver);
/*!
 * \brief Performs a transmission of the given size of the data array via the given uart.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_Transmit(DRV_UART_HANDLE_T * const ptDriver, uint8_t const * const data, size_t size);
/*!
 * \brief Receives the given size and stores it into data via the given uart.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_Receive(DRV_UART_HANDLE_T * const ptDriver, uint8_t * const data, size_t size);
/*!
 * \brief Transmits and receives data of given size via the given uart.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_TransmitReceive(DRV_UART_HANDLE_T * const ptDriver, uint8_t * const txData, uint8_t * const rxData, size_t size);
/*!
 * \brief Transmits a single char via the given uart.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_PutChar(DRV_UART_HANDLE_T * const ptDriver, unsigned char const cData);
/*!
 * \brief Receives a single char via the given uart.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_GetChar(DRV_UART_HANDLE_T * const ptDriver, unsigned char * const pcData);
/*!
 * \brief Aborts any transmissions on the given uart.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_Abort(DRV_UART_HANDLE_T * const ptDriver);
/*!
 * \brief The driver and device state are returned by this function.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_GetState(DRV_UART_HANDLE_T * const ptDriver, DRV_UART_STATE_E * const ptState);
/*!
 * \brief The driver and device state are returned by this function.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_GetRxState(DRV_UART_HANDLE_T * const ptDriver, DRV_UART_STATE_E * const ptState);
/*!
 * \brief The driver and device state are returned by this function.
 * \memberof DRV_UART_HANDLE_T
 */
DRV_STATUS_E DRV_UART_GetTxState(DRV_UART_HANDLE_T * const ptDriver, DRV_UART_STATE_E * const ptState);
/*!
 * \brief This function returns the amount of bytes received.
 * \details The function is defined as inline. It only returns the RxBufferCounter located in the handle.
 * The RxBufferCounter is the number of bytes received in the last transaction.
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return Bytes received
 */
__STATIC_FORCEINLINE uint32_t DRV_UART_GetRxCounter(DRV_UART_HANDLE_T * const ptDriver)
{
  if(ptDriver != 0)
  {
    return ptDriver->RxBufferCounter;
  }
  else
  {
    return -1;
  }
}
/*!
 * \brief This function returns the amount of bytes transmitted.
 * \details The function is defined as inline. It only returns the TxBufferCounter located in the handle.
 * The TxBufferCounter is the number of bytes transmitted in the last transaction.
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return Bytes transmitted
 */
__STATIC_FORCEINLINE uint32_t DRV_UART_GetTxCounter(DRV_UART_HANDLE_T * const ptDriver)
{
  if(ptDriver != 0)
  {
    return ptDriver->TxBufferCounter;
  }
  else
  {
    return -1;
  }
}

/*!
 * \brief This function returns the static buffer.
 * \details The function is defined as inline. It returns the rx buffer received statically.
 * The buffer content is only valid inside the fnRxCallback. The DRV_UART_GetRxCounter defines the
 * ammount of received bytes.
 * \memberof DRV_UART_HANDLE_T
 * \param[in,out] ptDriver The given handle of the drivers class
 * \return Static buffer containing the received data.
 */
__STATIC_FORCEINLINE uint8_t volatile * DRV_UART_GetStaticBuffer(DRV_UART_HANDLE_T * const ptDriver)
{
  return ptDriver->RxBufferStatic;
}

/*! \} *//* End of group UART */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __NETX90_UART_H */
