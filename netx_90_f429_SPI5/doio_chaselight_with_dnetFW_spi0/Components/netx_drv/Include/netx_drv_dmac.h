/*!************************************************************************//*!
 * \file    netx_drv_dmac.h
 * \brief   Header file of DMAC DRV module.
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
#ifndef NETX_DRV_DMAC_H_
#define NETX_DRV_DMAC_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*!
 * \addtogroup DMAC
 * \{
 */

/*!***************************************************************************/

/*!
 * \brief State of the dmachannel. This is not used at the moment.
 */
typedef enum DRV_DMAC_STATE_Etag
{
  DRV_DMAC_STATUS_UNKNOWN = 0x0u, /*!< Because it is not used, it is unknown. */
} DRV_DMAC_STATE_E;

/*!
 * \brief Type definition for the DMAC flow direction and who is controlling it.
 */
typedef enum DRV_DMAC_FLOW_Etag
{
  DRV_DMAC_FLOW_MEM_TO_MEM_BY_DMAC = 0u, /*!< Copy from memory to memory controlled by the DMAC. */
  DRV_DMAC_FLOW_MEM_TO_PER_BY_DMAC = 1u, /*!< Copy from memory to peripheral controlled by the DMAC. */
  DRV_DMAC_FLOW_PER_TO_MEM_BY_DMAC = 2u, /*!< Copy from peripheral to memory controlled by the DMAC. */
  DRV_DMAC_FLOW_PER_TO_PER_BY_DMAC = 3u, /*!< NOT SUPPORTED */
  DRV_DMAC_FLOW_PER_TO_PER_BY_DPER = 4u, /*!< NOT SUPPORTED */
  DRV_DMAC_FLOW_MEM_TO_PER_BY_PER = 5u, /*!< Copy from memory to peripheral controlled by the peripheral. */
  DRV_DMAC_FLOW_PER_TO_MEM_BY_PER = 6u, /*!< Copy from peripheral to memory controlled by the peripheral. */
  DRV_DMAC_FLOW_PER_TO_PER_BY_SPER = 7u, /*!< NOT SUPPORTED */
} DRV_DMAC_FLOW__E;

///////////////////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief A burst transfers size.
 */
typedef enum __ALIGNED(4) DRV_DMAC_BURST_TRANSFER_SIZE_Etag
{
  DRV_DMAC_BURST_TRANSFER_SIZE_1 = 0ul, /*!< Size of a burst transfer of 1. */
  DRV_DMAC_BURST_TRANSFER_SIZE_4 = 1ul, /*!< Size of a burst transfer of 4. */
  DRV_DMAC_BURST_TRANSFER_SIZE_8 = 2ul, /*!< Size of a burst transfer of 8. */
  DRV_DMAC_BURST_TRANSFER_SIZE_16 = 3ul, /*!< Size of a burst transfer of 16. */
  DRV_DMAC_BURST_TRANSFER_SIZE_32 = 4ul, /*!< Size of a burst transfer of 32. */
  DRV_DMAC_BURST_TRANSFER_SIZE_64 = 5ul, /*!< Size of a burst transfer of 64. */
  DRV_DMAC_BURST_TRANSFER_SIZE_128 = 6ul, /*!< Size of a burst transfer of 128. */
  DRV_DMAC_BURST_TRANSFER_SIZE_256 = 7ul /*!< Size of a burst transfer of 256. */
} DRV_DMAC_BURST_TRANSFER_SIZE_E;

/*!
 * \brief The width of a dmac transfer.
 */
typedef enum __ALIGNED(4) DRV_DMAC_TRANSFER_WIDTH_Etag
{
  DRV_DMAC_TRANSFER_WIDTH_8b = 0u, /*!< DMAC transfer of one byte. */
  DRV_DMAC_TRANSFER_WIDTH_16b = 1u, /*!< DMAC transfer of a half word. */
  DRV_DMAC_TRANSFER_WIDTH_32b = 2u, /*!< DMAC transfer of a full word. */
  DRV_DMAC_TRANSFER_WIDTH_MIN = DRV_DMAC_TRANSFER_WIDTH_8b, /*!< Minimal enumeration value used for boundary checks. */
  DRV_DMAC_TRANSFER_WIDTH_MAX = DRV_DMAC_TRANSFER_WIDTH_32b /*!< Maximum enumeration value used for boundary checks. */
} DRV_DMAC_TRANSFER_WIDTH_E;

/*!
 * \brief Enumeration of the possible types of the flow control.
 */
typedef enum DRV_DMAC_FLOW_CONTROLLER_Etag
{

  DRV_DMAC_FLOW_CONTROLLER_DMAC = 0u, /*!< The dmac is the flow controller. */
  DRV_DMAC_FLOW_CONTROLLER_PER = 1u, /*!< The peripheral is the flow controller. */
} DRV_DMAC_FLOW_CONTROLLER_E;

/*!
 * \brief Typedef for the option if the pointer for the copy data is iterated or not.
 */
typedef enum DRV_DMAC_INCREMENTATION_Etag
{
  DRV_DMAC_INCREMENTATION_INACTIVE = 0x0u, /*!< The pointer will be iterated. */
  DRV_DMAC_INCREMENTATION_ACTIVE = 0x1u, /*!< The pointer will not be iterated. */
} DRV_DMAC_INCREMENTATION_E;

/*!
 * \brief Typedef to define if the dmac will generate an interrupt after a list
 * element was processed.
 */
typedef enum DRV_DMAC_GENERATE_TERMINAL_INTERRUPT_Etag
{
  DRV_DMAC_GENERATE_TERMINAL_INTERRUPT_INACTIVE = 0x0u, /*!< No interrupt is generated. */
  DRV_DMAC_GENERATE_TERMINAL_INTERRUPT_ACTIVE = 0x1u /*!< An interrupt is generated at the end. */
} DRV_DMAC_GENERATE_TERMINAL_INTERRUPT_E;

/*!
 * \brief Defines the burst mode. In arm environments, burst accesses shall be used in default.
 */
typedef enum DRV_DMAC_ARM_EQ_Etag
{
  DRV_DMAC_ARM_EQ_SINGLE_REQUESTS = 0x0u, /*!< Requests a single request */
  DRV_DMAC_ARM_EQ_BURST_REQUESTS = 0x1u, /*!< Requests a burst request */
  DRV_DMAC_ARM_EQ_DEFAULT = DRV_DMAC_ARM_EQ_BURST_REQUESTS /*!< The defaul value of the field. */
} DRV_DMAC_ARM_EQ_E;

/*!
 * \brief Defines the control structure for a list item, defining a transfer element.
 *
 * The message of packed bitfield compatibility has to be ignored for some versions of gcc.
 * The DRV_DMAC_BURST_TRANSFER_SIZE_E is now aligned at 4B and it should be backward compatible.
 */
typedef __PACKED_STRUCT DRV_DMAC_CONTROL_Ttag
{
  uint32_t ulTransferSize             :12; /*!< The ammount of transfers to be performed (depends on source byte size). */
  uint32_t eSourceBurstSize           :3;  /*!< How large the burst access on the source shall be. */
  uint32_t eDestBurstSize             :3;  /*!< How large the burst access on the target shall be. */
  uint32_t eSourceTransferWidth       :3;  /*!< How many bytes shall be read from the source address. */
  uint32_t eDestTransferWidth         :3;  /*!< How many bytes shall be read from the destination address. */
  uint32_t eArmEq                     :1;  /*!< The burst access mode. */
  uint32_t reserved1                  :1;  /*!< Reserved bit. */
  uint32_t eSourceIncrementation      :1;  /*!< Shall the source address pointer be incremented? */
  uint32_t eDestIncrementation        :1;  /*!< Shall the destination address pointer be incremented? */
  uint32_t ulProtection               :3;  /*!< Has the burst transfer to be protected? */
  uint32_t eGenerateTerminalInterrupt :1;  /*!< Shall an interrupt be generated at the end? */
} DRV_DMAC_CONTROL_T;

/*!
 * \brief Type definition of the dmacs linked list element type for self referencing
 */
typedef struct DRV_DMAC_LINKED_LIST_ELEMENT_Ttag DRV_DMAC_LINKED_LIST_ELEMENT_T;

/*!
 * \brief Definition of the dmacs linked list element control union
 */
typedef union DRV_DMAC_LINKED_LIST_ELEMENT_CONTROL_Etag
{
  volatile DRV_DMAC_CONTROL_T bf; /*!< The control structure. */
  volatile uint32_t val; /*!< The control field as value. */
} DRV_DMAC_LINKED_LIST_ELEMENT_CONTROL_E;

/*!
 * \brief The dmac linked list element.
 */
struct __PACKED DRV_DMAC_LINKED_LIST_ELEMENT_Ttag
{
  void * volatile ptSource; /*!< Contains the pointer to the source. */
  void * volatile ptDest; /*!< Contains the pointer to the destination */
  DRV_DMAC_LINKED_LIST_ELEMENT_T * volatile ptNextElement; /*!< Contains the pointer to the next list element */
  DRV_DMAC_LINKED_LIST_ELEMENT_CONTROL_E tControl; /*!< Contains the control structure defining the list elements behaviour. */
};

///////////////////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The configuration of the driver.
 *
 * This structure shall be modified to change configuration parameters.
 * It is evaluated during the init routine and used during the handles lifetime.
 * The structure shall not be modified after initialization of the device.
 */
typedef struct DRV_DMAC_CONFIGURATION_Ttag
{
  DRV_DMAC_DEVICE_ID_E eDeviceID; /*!< The device to be used. */
  DRV_DMAC_FLOW_CONTROLLER_E eFlowController; /*!< Defines who controlls the data flow. */
  DRV_DMAC_TRANSFER_WIDTH_E eTransferWidthSource; /*!< Defines the width of a transfer access to the source address. */
  DRV_DMAC_TRANSFER_WIDTH_E eTransferWidthDest; /*!< Defines the width of a transfer access to the destination address. */
  DRV_DMAC_INCREMENTATION_E eIncrementationSource; /*!< Defines if the source address is incremented or not. */
  DRV_DMAC_INCREMENTATION_E eIncrementationDest; /*!< Defines if the destination address is incremented or not. */
  DRV_CALLBACK_F fCallbackComplete; /*!< The callback to be executed at list completion. */
  void* ptCallbackHandleComplete; /*!< The users handle associated with the callback */
  DRV_DMAC_PERIPHERAL_E ePeripheralSource; /*!< The peripheral from where the data is copied from.  */
  DRV_DMAC_PERIPHERAL_E ePeripheralDest; /*!< The peripheral to where the data is copied. */
} DRV_DMAC_CONFIGURATION_T;

/*!
 * \brief The handle of the DMAC driver.
 *
 * It SHALL not be modified outside of the driver, even if it appears to be possible.
 * The configuration might be copied or modified before initialization of the driver.
 */
typedef struct DRV_DMAC_HANDLE_Ttag
{
  DRV_DMAC_CH_DEVICE_T* ptDevice; /*!< \private The DMAC device register as bitfield and value unions.*/
  DRV_DMAC_CONFIGURATION_T tConfiguration; /*!< Contains the dmacs configuration attributes. */
  DRV_DMAC_LINKED_LIST_ELEMENT_T atLinkedList[DRV_DMAC_LIST_LENGTH]; /*!< \private list of linked elements for transmitting more than 4092 ticks (8b 16b or 32b depending on source size)*/
  DRV_LOCK_T tLock; /*!< \private The drivers locking variable used as internal mutex*/
} DRV_DMAC_HANDLE_T;

/*!
 * \brief Initializes the dmac sequencer and its handle by the given configuration.
 * \memberof DRV_DMAC_HANDLE_T
 */
DRV_STATUS_E DRV_DMAC_Init(DRV_DMAC_HANDLE_T * const ptSequencer);

/*!
 * \brief Deinitializes the dmac sequencer and its handle.
 * \memberof DRV_DMAC_HANDLE_T
 */
DRV_STATUS_E DRV_DMAC_DeInit(DRV_DMAC_HANDLE_T * const ptSequencer);

/*!
 * \brief Starts the given dmac sequencer.
 * \memberof DRV_DMAC_HANDLE_T
 */
DRV_STATUS_E DRV_DMAC_Start(DRV_DMAC_HANDLE_T * const ptSequencer, void * const ptBufferSource, void * const ptBufferDestination, size_t size);

/*!
 * \brief Aborts the given dmac sequencer.
 * \memberof DRV_DMAC_HANDLE_T
 */
DRV_STATUS_E DRV_DMAC_Abort(DRV_DMAC_HANDLE_T * const ptSequencer);

/*!
 * \brief Aquires the state of the given dmac sequencer.
 * \memberof DRV_DMAC_HANDLE_T
 */
DRV_STATUS_E DRV_DMAC_GetState(DRV_DMAC_HANDLE_T * const ptSequencer, DRV_DMAC_STATE_E * const ptState);

/*! \} *//* End of group DMAC */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif

#endif /* NETX_DRV_DMAC_H_ */
