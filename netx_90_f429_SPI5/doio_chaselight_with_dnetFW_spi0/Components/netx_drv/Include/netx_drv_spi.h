/*!************************************************************************//*!
 * \file    netx_drv_spi.h
 * \brief   Header file of GPIO DRV module.
 * $Revision: 11377 $
 * $Date: 2024-06-12 16:34:59 +0300 (Wed, 12 Jun 2024) $
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
#ifndef __NETX_DRV_SPI_H
#define __NETX_DRV_SPI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*!
 * \addtogroup SPI
 * \{
 */

/*!
 * \brief Enumeration of the mosi drive state as slave.
 */
typedef enum DRV_SPI_MISO_Etag
{
  DRV_SPI_MISO_ACTIVE = 0x0u, /*!< MISO activated */
  DRV_SPI_MISO_INACTIVE = 0x1u /*!< MISO deactivated */
} DRV_SPI_MISO_E;

/*!
 * \brief Enumeration of the frame or slave select (FSS) signal pin mask.
 *
 * This signal is commonly known as frame select FS, slave select SS or SSEL, chip select CS, chip enable CE or slave transmit enable STE.
 */
typedef enum DRV_SPI_FSS_Etag
{
  DRV_SPI_FSS_NONE = 0x00u,/*!< No fss selected */
  DRV_SPI_FSS_0 = 0x1u,/*!< Drive pin 0 */
  DRV_SPI_FSS_1 = 0x2u,/*!< Drive pin 1 */
  DRV_SPI_FSS_2 = 0x4u,/*!< Drive pin 2 */
  DRV_SPI_FSS_ALL = 0x7u /*!< Drive all pins */
} DRV_SPI_FSS_E;

/*!
 * \brief Enumeration of useful spi frequencies.
 *
 * The value is a clock divider.
 */
typedef enum DRV_SPI_FREQUENCY_Etag
{
  DRV_SPI_FREQUENCY_STOPPED = 0x000u,/*!< The SPI is stopped. */
  DRV_SPI_FREQUENCY_24_4KHz = 0x001u,/*!< 24KHz */
  DRV_SPI_FREQUENCY_48_8KHz = 0x002u,/*!< 48KHz */
  DRV_SPI_FREQUENCY_97_6KHz = 0x004u,/*!< 97KHz */
  DRV_SPI_FREQUENCY_195KHz = 0x008u,/*!< 195KHz */
  DRV_SPI_FREQUENCY_390KHz = 0x010u,/*!< 390KHz */
  DRV_SPI_FREQUENCY_781KHz = 0x020u,/*!< 0.8MHz */
  DRV_SPI_FREQUENCY_1_56MHz = 0x040u,/*!< 1.5MHz */
  DRV_SPI_FREQUENCY_3_125MHz = 0x080u,/*!< 3.125MHz */
  DRV_SPI_FREQUENCY_6_25MHz = 0x100u,/*!< 6.25MHz */
  DRV_SPI_FREQUENCY_12_5MHz = 0x200u,/*!< 12.5MHz */
  DRV_SPI_FREQUENCY_25MHz = 0x400u,/*!< 25MHz */
  DRV_SPI_FREQUENCY_50MHz = 0x800u,/*!< 50MHz */
  DRV_SPI_FREQUENCY_99MHz = 0xFFFu,/*!< 99MHz */
  DRV_SPI_FREQUENCY_MAX = DRV_SPI_FREQUENCY_99MHz, /*!< SPI maximum frequency */
  DRV_SPI_FREQUENCY_MIN = DRV_SPI_FREQUENCY_STOPPED /*!< SPI minimum frequency */
} DRV_SPI_FREQUENCY_E;

/*!
 * \brief Enumeration of the master/slave mode.
 */
typedef enum DRV_SPI_BEHAVIOUR_Etag
{
  DRV_SPI_BEHAVIOUR_MASTER = 0x0u,/*!< Behaves as master. */
  DRV_SPI_BEHAVIOUR_SLAVE = 0x1u, /*!< Behaves as slave */
} DRV_SPI_BEHAVIOUR_E;

/*!
 * \brief Enumeration of the sampling mode.
 */
typedef enum DRV_SPI_MODE_Etag
{
  DRV_SPI_MODE_0 = 0x0u,/*!< CPOL 0 CPHA 0 */
  DRV_SPI_MODE_1 = 0x1u, /*!< CPOL 0 CPHA 1 */
  DRV_SPI_MODE_2 = 0x2u, /*!< CPOL 1 CPHA 0 */
  DRV_SPI_MODE_3 = 0x3u, /*!< CPOL 1 CPHA 1 */
  DRV_SPI_MODE_MIN = DRV_SPI_MODE_0, /*!< Minimum value */
  DRV_SPI_MODE_MAX = DRV_SPI_MODE_3 /*!< Maximum value */
} DRV_SPI_MODE_E;

/*!
 * \brief Enumeration of the serial clock phase.
 *
 * If the data is sampled at the rising/first or falling/second edge.
 * (DRV_SPI_SPO_CLOCK_INACTIVE_LOW)
 */
typedef enum DRV_SPI_SPH_Etag
{
  DRV_SPI_SPH_SAMPLE_AT_FIRST_EDGE = 0x0u,/*!< First edge */
  DRV_SPI_SPH_SAMPLE_AT_SECOND_EDGE = 0x1u /*!< Second edge */
} DRV_SPI_SPH_E;

/*!
 * \brief Enumeration of the duplex mode.
 *
 * Full duplex is only available for SPI mode. QSPI with 1/2/4 bit is only half duplex.
 */
typedef enum DRV_SPI_DUPLEX_Etag
{
  DRV_SPI_DUPLEX_HALF = 0x1u,/*!< First edge */
  DRV_SPI_DUPLEX_FULL = 0x0u /*!< Second edge */
} DRV_SPI_DUPLEX_E;

/*!
 * \brief Enumeration of the parallelism.
 *
 * It defines how many bits will be transmitted in parallel on each clock.
 * 2/4 Bit are only available as QSPI half duplex mode.
 */
typedef enum DRV_SPI_PARALLELISM_Etag
{
  DRV_SPI_PARALLELISM_1BIT = 0x0u,/*!< Send one bit on MOSI and receive one on MISO. Regarding duplex configuration. */
  DRV_SPI_PARALLELISM_2BIT = 0x1u,/*!< Send or receive on both MOSI and MISO simultaneously. Only half duplex allowed. */
  DRV_SPI_PARALLELISM_4BIT = 0x2u, /*!< Send or receive on all four MOSI, MISO, IO2 and IO3 simultaneously. Only half duplex allowed. */
  DRV_SPI_PARALLELISM_MAX = DRV_SPI_PARALLELISM_4BIT, /*!< Max value. */
  DRV_SPI_PARALLELISM_MIN = DRV_SPI_PARALLELISM_1BIT /*!< Min value. */
} DRV_SPI_PARALLELISM_E;


/*!
 * \brief Enumeration of the serial clock polarity.
 */
typedef enum DRV_SPI_SPO_Etag
{
  DRV_SPI_SPO_CLOCK_INACTIVE_LOW = 0x0u,/*!< low when inactive high when active*/
  DRV_SPI_SPO_CLOCK_ACTIVE_LOW = 0x1u,/*!< high when inactive low when active*/
  DRV_SPI_SPO_CLOCK_INACTIVE_HIGH = 0x1u, /*!< high when inactive low when active*/
  DRV_SPI_SPO_CLOCK_ACTIVE_HIGH = 0x0u /*!< low when inactive high when active*/
} DRV_SPI_SPO_E;

/*!
 * \brief Bitfield for the spi mode
 */
typedef struct DRV_SPI_MODE_Ttag
{
  uint32_t eSPH       : 1;  /*!< [0..0]  Is phase shifted.        */
  uint32_t eSPO       : 1;  /*!< [1..1]  Is clock inverted.       */
  uint32_t ulBfAlign0 : 30; /*!< [31..2] bitField alignment value */
} DRV_SPI_MODE_T;

/*!
 * \brief Union for the spi mode
 */
typedef union DRV_SPI_MODE_Utag
{
  DRV_SPI_MODE_E value; /*!< mode as enum value. */
  DRV_SPI_MODE_T structure; /*!< mode as structured value. */
} DRV_SPI_MODE_U;

/*!
 * \brief Enumeration of the Endianess used.
 *
 * This is necessary if more than 8 bit data size is selected.
 */
typedef enum DRV_SPI_ENDIANESS_Etag
{
  DRV_SPI_ENDIANESS_LITTLE = 0x0u,/*!< Little Endian */
  DRV_SPI_ENDIANESS_BIG = 0x1u /*!< Big Endian */
} DRV_SPI_ENDIANESS_E;

/*!
 * \brief Enumeration of the frame/chip select modes.
 */
typedef enum DRV_SPI_FSS_STATIC_Etag
{
  DRV_SPI_FSS_STATIC_HARDWARE = 0x0u,/*!< FSS is Hardware controlled. Master in Motorola mode will toggle fss after every word. Slave feels addressed if fss matches.*/
  DRV_SPI_FSS_STATIC_DRIVER = 0x1u, /*!< FSS is driver controlled. It is low while IO is performed. Otherwise a DIO should been used. */
  DRV_SPI_FSS_STATIC_CALLER = 0x11u /*!< FSS is controlled manually by the caller (DRV_SPI_ChangeFss, eFFS has to be NONE). Driver will not initialize or alter FSS on its own. */
} DRV_SPI_FSS_STATIC_E;

/*!
 * \brief Enumeration of the water mark level of the fifos.
 */
typedef enum DRV_SPI_FIFO_WM_Etag
{
  DRV_SPI_FIFO_WM_RESET = 0x0u,/*!< Water mark default level of 8 will be used. */
  DRV_SPI_FIFO_WM_MIN = 0x1u,/*!< Minimal water mark level of 0*/
  DRV_SPI_FIFO_WM_DEFAULT = 0x9u,/*!< Default water mark level of 8 */
  DRV_SPI_FIFO_WM_MAX = 0x10u /*!< Maximum water mark level of 16 */
} DRV_SPI_FIFO_WM_E;

/*!
 * \brief Enumeration of the available frame formats of the spi.
 */
typedef enum DRV_SPI_FRAME_FORMAT_Etag
{
  DRV_SPI_FRAME_FORMAT_MOTOROLA = 0x0u,/*!< The motorola spi format. */
  DRV_SPI_FRAME_FORMAT_RESERVED0 = 0x1u, /*!< Reserved */
  DRV_SPI_FRAME_FORMAT_RESERVED1 = 0x2u, /*!< Reserved */
  DRV_SPI_FRAME_FORMAT_RESERVED2 = 0x3u, /*!< Reserved */
  DRV_SPI_FRAME_FORMAT_MAX = DRV_SPI_FRAME_FORMAT_MOTOROLA /*!< Used for boundary checking */
} DRV_SPI_FRAME_FORMAT_E;

/*!
 * \brief Enumeration of the modes of the early response bit generation.
 */
typedef enum DRV_SPI_SLV_SIG_EARLY_Etag
{
  DRV_SPI_SLV_SIG_NOT_EARLY = 0x0u,/*!< Generate first bit not early */
  DRV_SPI_SLV_SIG_EARLY = 0x1u/*!< Generate first bit early */
} DRV_SPI_SLV_SIG_EARLY_E;

/*!
 * \brief Enumeration of the filter state.
 */
typedef enum DRV_SPI_FILTER_Etag
{
  DRV_SPI_FILTER_INACTIVE = 0x0u,/*!< Disabled */
  DRV_SPI_FILTER_ACTIVE = 0x1u/*!< Enabled */
} DRV_SPI_FILTER_E;

/*!
 * \brief Enumeration of the loop back mode.
 */
typedef enum DRV_SPI_LOOP_BACK_MODE_Etag
{
  DRV_SPI_LOOP_BACK_MODE_INACTIVE = 0x0u,/*!< Disabled */
  DRV_SPI_LOOP_BACK_MODE_ACTIVE = 0x1u/*!< Enabled */
} DRV_SPI_LOOP_BACK_MODE_E;

/*!
 * \brief Enumeration of dummy patterns available to send when only one
 * of the full duplex lines are used.
 */
typedef enum DRV_SPI_DUMMYPATTERN_Etag
{
  DRV_SPI_DUMMYPATTERN_NULL = 0x0u,/*!< nothing at all, just 0s */
  DRV_SPI_DUMMYPATTERN_ALTERNATING_0 = 0xAAAAu,/*!< alternating starting with 0 (not available in QSPI)*/
  DRV_SPI_DUMMYPATTERN_ALTERNATING_1 = 0x5555u,/*!< alternating starting with 1 (not available in QSPI)*/
  DRV_SPI_DUMMYPATTERN_FULL = 0xFFFFu/*!< just 1s */
} DRV_SPI_DUMMYPATTERN_E;

/*!
 * \brief Enumeration of the data size of a word transmitted.
 * When the device is SQI, some modes ignore this setting
 */
typedef enum DRV_SPI_DATA_SIZE_SELECT_Etag
{
  DRV_SPI_DATA_SIZE_SELECT_RESET    = 0x0ul,/*!< Default value will be 8 bits */
  DRV_SPI_DATA_SIZE_SELECT_4b       = 0x03ul,/*!< Size of 4 bits*/
  DRV_SPI_DATA_SIZE_SELECT_MIN      = DRV_SPI_DATA_SIZE_SELECT_4b,/*!< Smallest size is 4 bits*/
  DRV_SPI_DATA_SIZE_SELECT_5b       = 0x04ul,/*!< Size of 5 bits*/
  DRV_SPI_DATA_SIZE_SELECT_6b       = 0x05ul,/*!< Size of 6 bits*/
  DRV_SPI_DATA_SIZE_SELECT_7b       = 0x06ul,/*!< Size of 7 bits*/
  DRV_SPI_DATA_SIZE_SELECT_8b       = 0x07ul,/*!< Size of 8 bits*/
  DRV_SPI_DATA_SIZE_SELECT_DEFAULT  = DRV_SPI_DATA_SIZE_SELECT_8b,/*!< Default value will be 8 bits */
  DRV_SPI_DATA_SIZE_SELECT_9b       = 0x08ul,/*!< Size of 9 bits*/
  DRV_SPI_DATA_SIZE_SELECT_10b      = 0x09ul,/*!< Size of 10 bits*/
  DRV_SPI_DATA_SIZE_SELECT_11b      = 0x0aul,/*!< Size of 11 bits*/
  DRV_SPI_DATA_SIZE_SELECT_12b      = 0x0bul,/*!< Size of 12 bits*/
  DRV_SPI_DATA_SIZE_SELECT_13b      = 0x0cul,/*!< Size of 13 bits*/
  DRV_SPI_DATA_SIZE_SELECT_14b      = 0x0dul,/*!< Size of 14 bits*/
  DRV_SPI_DATA_SIZE_SELECT_15b      = 0x0eul,/*!< Size of 15 bits*/
  DRV_SPI_DATA_SIZE_SELECT_16b      = 0x0ful,/*!< Size of 16 bits*/
  DRV_SPI_DATA_SIZE_SELECT_MAX      = DRV_SPI_DATA_SIZE_SELECT_16b, /*!< Largest dynamic size is 16 bits*/
} DRV_SPI_DATA_SIZE_SELECT_E;

/*!
 * \brief Enumeration of the spi state informations.
 */
typedef enum DRV_SPI_STATE_Etag
{
  DRV_SPI_STATE_RX_FIFO_UNDERRUN  = 0x1u << 31u,/*!< Rx fifo had an underrun */
  DRV_SPI_STATE_RX_FIFO_OVERFLOW  = 0x1u << 30u,/*!< Tx fifo had an overflow */
  DRV_SPI_STATE_RX_FIFO_LVL_16b   = 0x10u << 24u,/*!< Rx fifo level mask 16Bytes */
  DRV_SPI_STATE_RX_FIFO_LVL_08b   = 0x8u << 24u,/*!< Rx fifo level mask 8Bytes */
  DRV_SPI_STATE_RX_FIFO_LVL_04b   = 0x4u << 24u,/*!< Rx fifo level mask 4Bytes */
  DRV_SPI_STATE_RX_FIFO_LVL_02b   = 0x2u << 24u,/*!< Rx fifo level mask 2Bytes */
  DRV_SPI_STATE_RX_FIFO_LVL_01b   = 0x1u << 24u,/*!< Rx fifo level mask 1Bytes */
  DRV_SPI_STATE_TX_FIFO_UNDERRUN  = 0x1u << 23u,/*!< Tx fifo had an underrun */
  DRV_SPI_STATE_TX_FIFO_OVERFLOW  = 0x1u << 22u,/*!< Tx fifo had an overflow */
  DRV_SPI_STATE_TX_FIFO_LVL_16b   = 0x10u << 16u,/*!< Tx fifo level mask 16Bytes */
  DRV_SPI_STATE_TX_FIFO_LVL_08b   = 0x8u << 16u,/*!< Tx fifo level mask 8Bytes */
  DRV_SPI_STATE_TX_FIFO_LVL_04b   = 0x4u << 16u,/*!< Tx fifo level mask 4Bytes */
  DRV_SPI_STATE_TX_FIFO_LVL_02b   = 0x2u << 16u,/*!< Tx fifo level mask 2Bytes */
  DRV_SPI_STATE_TX_FIFO_LVL_01b   = 0x1u << 16u,/*!< Tx fifo level mask 1Bytes */
  DRV_SPI_STATE_BUSY              = 0x1u << 4u,/*!< Set if the device is not idle */
  DRV_SPI_STATE_RX_FIFO_FULL      = 0x1u << 3u,/*!< The Rx fifo is full */
  DRV_SPI_STATE_RX_FIFO_NOT_EMPTY = 0x1u << 2u,/*!< The Rx fifo is not empty */
  DRV_SPI_STATE_TX_FIFO_NOT_FULL  = 0x1u << 1u,/*!< The Tx fifo is not full */
  DRV_SPI_STATE_TX_FIFO_EMPTY     = 0x1u << 0/*!< The Tx fifo is empty */
} DRV_SPI_STATE_E;

/*!
 * \brief The configuration structure of the SPI device driver.
 *
 * This structure shall be modified to change configuration parameters.
 * It is evaluated during the init routine and used during the handles lifetime.
 * The structure shall not be modified after initialization of the device.
 */
typedef struct DRV_SPI_CONFIGURATION_Ttag
{
  DRV_SPI_DEVICE_ID_E eSPIDeviceID; /*!< The id of the device used. */
  DRV_SPI_MISO_E eMISO;/*!< The mosi state. */
  DRV_OPERATION_MODE_E eOperationMode;/*!< Which programming method (DMA/IRQ/POLL) is used.*/
  DRV_SPI_FSS_E eFSS;/*!< The frame/chip select to be used. */
  DRV_SPI_FREQUENCY_E eFrequency;/*!< The frequency of the SPI. */
  DRV_SPI_FRAME_FORMAT_E eFrameFormat;/*!< Motorola SPI frame format only.*/
  DRV_SPI_MODE_U uMode;/*!< The spi sampling mode.*/
  DRV_SPI_DUPLEX_E eDuplex;/*!< Half or full duplex.*/
  DRV_SPI_PARALLELISM_E eParallelism;/*!< How many bits are transferred parallel.*/
  DRV_SPI_BEHAVIOUR_E eBehaviour; /*!< Defines if the device is master or slave.*/
  DRV_SPI_SLV_SIG_EARLY_E eSlaveSigEarly;/*!< Is slave miso generated earlier.*/
  DRV_SPI_FILTER_E eFILTER;/*!< Is rx data input filter (majority around clk edge).*/
  DRV_SPI_ENDIANESS_E eEndianess;/*!< Is big endian.*/
  DRV_SPI_FSS_STATIC_E eFSSStatic;/*!< Is fss static.*/
  DRV_SPI_FIFO_WM_E eRxFiFoWm;/*!< Receive FiFo watermark. [0..0b1111] (0b1000)*/
  DRV_SPI_FIFO_WM_E eTxFiFoWm;/*!< Tranmit FiFo watermark. [0..0b1111] (0b1000)*/
  DRV_SPI_FIFO_WM_E eTxFiFoRefillLevel;/*!< Tranmit FiFo Refill Level. [0..0b1111] (0b1000)*/
  DRV_SPI_LOOP_BACK_MODE_E eLoopBackMode;/*!< Is loop back mode active.*/
  DRV_SPI_DATA_SIZE_SELECT_E eDataSize;/*!< Size of a word.*/
  DRV_SPI_DUMMYPATTERN_E uDummyPattern;/*!< Pattern used as dummy in empty Tx paths.*/
  DRV_DMAC_DEVICE_ID_E eDMATx;/*!< The device id of the dmac to be used for tx.*/
  DRV_DMAC_DEVICE_ID_E eDMARx;/*!< The device id of the dmac to be used for rx.*/
  DRV_DMAC_HANDLE_T* ptSequencerTx;/*!< The allocated and initialized to 0 handle of the tx dmac.*/
  DRV_DMAC_HANDLE_T* ptSequencerRx;/*!< The allocated and initialized to 0 handle of the rx dmac.*/
  DRV_CALLBACK_F fnCompleteCallback;/*!< The callback used if transaction is completed.*/
  void* pCompleteCallbackHandle;/*!< The handle associated with the callback.*/
  uint32_t ulDriverTimeout; /*!< Timeout used in the driver for polling contexts. */
} DRV_SPI_CONFIGURATION_T;

/*!
 * \brief The handle of the driver.
 *
 * The configuration SHALL be changed before initializing the device and shall not be changed
 * afterwards.
 * The rest of it SHALL not be modified outside of the driver, even if it appears to be possible.
 * \struct DRV_SPI_HANDLE_T
 * \struct DRV_SPI_HANDLE_Ttag
 */
typedef struct DRV_SPI_HANDLE_Ttag
{
  DRV_SPI_DEVICE_U ptDevice;/*!< \private The SPI device register as bitfield and value unions.*/
  DRV_SPI_CONFIGURATION_T tConfiguration;/*!< The configuration of the spi device. */
  uint64_t ullFrameStartTick;/*!< \private Used for timeout detection. */
  void * volatile TxBuffer;/*!< \private Transmit buffer */
  size_t volatile TxBufferSize;/*!< \private Transmit size */
  size_t volatile TxBufferCounter;/*!< \private Transmit counter */
  void * volatile RxBuffer;/*!< \private Receive buffer */
  size_t volatile RxBufferSize;/*!< \private Receive size */
  size_t volatile RxBufferCounter;/*!< \private Receive counter */
  DRV_LOCK_T tLock;/*!< \private The drivers locking variable used as internal mutex*/
} DRV_SPI_HANDLE_T;

/*!
 * \brief This function initializes the DRV_SPI_HANDLE_T object.
 * \memberof DRV_SPI_HANDLE_T
 */
DRV_STATUS_E DRV_SPI_Init(DRV_SPI_HANDLE_T * const ptDriver);

/*!
 * \brief This function disables the DRV_SPI_HANDLE_T object.
 * \memberof DRV_SPI_HANDLE_T
 */
DRV_STATUS_E DRV_SPI_DeInit(DRV_SPI_HANDLE_T * const ptDriver);

/*!
 * \brief Function for transmitting data of given size via the SPI device.
 * \memberof DRV_SPI_HANDLE_T
 */
DRV_STATUS_E DRV_SPI_Transmit(DRV_SPI_HANDLE_T * const ptDriver, uint8_t* pcData, size_t size);

/*!
 * \brief Function for receiveing data of given size by the SPI device.
 * \memberof DRV_SPI_HANDLE_T
 */
DRV_STATUS_E DRV_SPI_Receive(DRV_SPI_HANDLE_T * const ptDriver, uint8_t* pcData, size_t size);

/*!
 * \brief Function for transmitting and receiveing data at the same time of given size by the SPI device.
 * \memberof DRV_SPI_HANDLE_T
 */
DRV_STATUS_E DRV_SPI_TransmitReceive(DRV_SPI_HANDLE_T * const ptDriver, uint8_t* txData, uint8_t* rxData, size_t size);

/*!
 * \brief Function for changing the fss between transactions.
 * \memberof DRV_SPI_HANDLE_T
 */
DRV_STATUS_E DRV_SPI_ChangeFss(DRV_SPI_HANDLE_T * const ptDriver, DRV_SPI_FSS_E eFss);

/*!
 * \brief Function for aborting the current data transfer on the SPI device.
 * \memberof DRV_SPI_HANDLE_T
 */
DRV_STATUS_E DRV_SPI_Abort(DRV_SPI_HANDLE_T * const ptDriver);

/*!
 * \brief Function returning a driver spi state enumeration and a return code.
 * \memberof DRV_SPI_HANDLE_T
 */
DRV_STATUS_E DRV_SPI_GetState(DRV_SPI_HANDLE_T * const ptDriver, DRV_SPI_STATE_E * const ptState);

/*! \} *//* End of group SPI */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __NETX_DRV_SPI_H */
