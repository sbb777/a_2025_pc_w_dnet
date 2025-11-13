/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id: $:

Description:
This header provides the DeviceNet Slave specific packet definitions of the command

 - DNS_CMD_RESET_IND/RES

These definitions contribute to the LFW API of the DeviceNet Slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/
#ifndef __DNS_PACKET_RESET_H
#define __DNS_PACKET_RESET_H

#include <stdint.h>
#include "Hil_Packet.h"
#include "Hil_Compiler.h"

/* pragma pack */
#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_PACK_1(DNS_PKT_RESET)
#endif

/*!\defgroup dnsResetInd Reset Service
 *
 * Reset Service
 * =====================================
 * The Reset Indication is send from the stack to the host whenever a reset of the device is required.
 * It is only a notification about a reset that has been requested. The stack will not perform
 * the reset procedure by itself. The reset need to be performed afterwards by the host.
 * When the host receives the indication it must respond to the packet. The host can either
 * accept the reset or reject it with a corresponding error code. The reset indication can be caused
 * either by a reset service from a remote device to the identity object instance 1
 * or a stack internal requirement. In case of a reset requested via network
 * the stack will return the response to the sender.

 *@{*/

/*!\name Reset Reason
 *@{*/
#define DNS_RESET_REASON_ID_OBJECT_NET_RESET       0  /*!< The reset was requested by a master
* or commissioning tool via network to instance one of the identity object. Additionally the reset type is indicated.*/
#define DNS_RESET_REASON_DN_OBJECT_MACID_SET       1  /*!< The reset is required because a master or
* commissioning tool has set the MAC ID attribute of the DeviceNet object. This requires a reset to go online with the new MAC ID.*/
#define DNS_RESET_REASON_NP_RESUME_SWITCH_CHANGE   2  /*!< This reason can only appear
* when the switch support for MAC ID and/or Baud rate is enabled. The reset is required because
* one or both of the switch values have been changed at runtime and the 24V network power was released and resumed.*/
#define DNS_RESET_REASON_NETWOK_POWER_CHANGE       3  /*!< Network Power - 24V network power has been changed.
* The actuall value is given by the Reason parameter - DNS_RESET_REASON_NP_MISSING or DNS_RESET_REASON_NP_PRESENT */
/*!@}*/

/*!\name Reset Type
*@{*/
#define DNS_RESET_TYPE_POWER_CYCLE                 0  /*!< Power Cycle - emulate as closely as
* possible cycling power on the item the Identity Object represents.*/
#define DNS_RESET_TYPE_FACTORY_DEFAULT             1  /*!< Factory default - Return as closely as possible
* to the factory default configuration, then emulate cycling power as closely as possible.*/
#define DNS_RESET_TYPE_FACTORY_WO_COMM_PRM         2  /*!< Return to Factory Defaults except Communication Parameters. */

#define DNS_RESET_TYPE_NP_MISSING                  3  /*!< This is an informative reason to allow the application
* to monitor, when the 24V Network Power is missing or lost.
* There is no explicit need to perform a reset, just in case of application specific requirements. */
#define DNS_RESET_TYPE_NP_PRESENT                  4  /*!< This is an informative reason to allow the application
* to monitor, when the 24V Network Power becomes present or resumed.
* There is no explicit need to perform a reset, just in case of application specific requirements.*/
/*!@}*/

/************************************************************************************/
/* DNS_RESET_IND/RES                                                                */
/************************************************************************************/
/*! \brief Reset Indication Data parameters. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_RESET_IND_Ttag
{
  uint32_t ulReason; /*!< Reset reason. */
  uint32_t ulType;   /*!< Reset type. */
}DNS_RESET_IND_T;

/*! \brief Reset Indication packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_RESET_IND_Ttag
{
  HIL_PACKET_HEADER_T tHead;   /*!< Packet header. */
  DNS_RESET_IND_T     tData;   /*!< Reset Indication Data parameters. */
}DNS_PACKET_RESET_IND_T;

/*! Size of Reset Indication. */
#define DNS_RESET_IND_SIZE (sizeof(DNS_RESET_IND_T))

/*! \brief Reset Response Data parameters. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_RESET_RES_Ttag
{
  uint32_t ulReason; /*!< Reset reason */
  uint32_t ulType;   /*!< Reset type */
  uint32_t ulGRC;    /*!< Generic Error Code */
  uint32_t ulERC;    /*!< Extended Error Code */
}DNS_RESET_RES_T;

/*! \brief Reset Response packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_RESET_RES_Ttag
{
 HIL_PACKET_HEADER_T tHead;   /*!< Packet header. */
 DNS_RESET_RES_T     tData;   /*!< Reset Response Data parameters. */
}DNS_PACKET_RESET_RES_T;

/*! Size of Reset Response. */
#define DNS_RESET_RES_SIZE (sizeof(DNS_RESET_RES_T))


/* pragma unpack */
#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_UNPACK_1(DNS_PKT_RESET)
#endif

/*!@}*/
#endif /* __DNS_RESET_H */
