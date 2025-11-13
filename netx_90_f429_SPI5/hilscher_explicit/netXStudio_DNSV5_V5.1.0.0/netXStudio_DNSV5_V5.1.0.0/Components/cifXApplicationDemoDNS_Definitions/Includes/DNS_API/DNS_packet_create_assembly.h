/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
This header provides the DeviceNet Slave specific packet definitions of the command

  - DNS_CMD_CREATE_ASSEMBLY_REQ/CNF

These definitions contribute to the LFW API of the DeviceNet Slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/
#ifndef __DNS_PACKET_CREATE_ASSEMBLY_H
#define __DNS_PACKET_CREATE_ASSEMBLY_H

#include <stdint.h>
#include "Hil_Packet.h"
#include "Hil_Compiler.h"


#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_PACK_1(DNS_PKT_CREATE_ASSEMBLY)
#endif

/*!\defgroup dnsCreateAsm Create Assembly Service
 *
 * Create Assembly Service
 * =====================================
 * This service allows to create additional assembly objects.
 * Creating additional assembly objects can be used when more than the two default input and output
 * assemblies shall be supported. Creating assembly objects is only allowed at configuration phase
 * when the DeviceNet Slave stack is in Stop state.
 *@{*/

/*!\name Assembly Configuration Flags
 *@{*/
#define DNS_AS_TYPE_CONSUME           (0x00000000) /*!< Assembly type - If this flag is not set
* the created assembly instance is a consuming assembly. The data received from the network are place
* in the input area of the DPM image. */

#define DNS_AS_TYPE_PRODUCE           (0x00000001) /*!< Assembly type - If this flag is set
* the created assembly instance is a producing assembly. The data are transmitted to the network and
* take from the output area of the DPM image. */

#define DNS_AS_OPTION_MAP_RCEIVE_IDLE (0x00000100) /*!< Assembly option - If this flag is set
* "receive/idle" information will be placed into the input image of the DPM for this assembly. */

#define DNS_AS_OPTION_MAP_RCEIVE_SEQ_COUNT (0x00000200) /*!< Assembly option - If this flag is set
* "sequence counter" information will be placed into the input image of the DPM for this assembly. */

/*! Assembly reserved:  Reserved flags must be set 0. */
#define DNS_AS_CONS_FLAGS_RESERVED         0xFFFFFCFF   /*!< Assembly reserved:  Reserved flags must be set 0. */
#define DNS_AS_PROD_FLAGS_RESERVED         0xFFFFFFFE   /*!< Assembly reserved:  Reserved flags must be set 0. */
/*!@}*/


/*!\name Assembly Data Status
 *@{*/
#define DNS_AS_DATA_STATUS_ZERO               (0x00000000) /*!< Data are in safe state zero.   */
#define DNS_AS_DATA_STATUS_RECEIVE_RUN        (0x00000001) /*!< Data are valid.                */
#define DNS_AS_DATA_STATUS_RECEIVE_IDLE       (0x00000002) /*!< Data are in receive idle mode. */
#define DNS_AS_DATA_STATUS_RECEIVE_IDLE_ZERO  (0x00000003) /*!< Data are in receive idle zero. */
#define DNS_AS_DATA_STATUS_HOLD_LAST_STATE    (0x00000004) /*!< Data are in hold last state.   */
/*!@}*/

/*!\name Default Assembly Instances
 *@{*/
#define DNS_AS_DEFAULT_PRODUCE_INSTANCE       (101) /*!< Hil default produce assembly instance. */
#define DNS_AS_DEFAULT_CONSUME_INSTANCE       (100) /*!< Hil default consume assembly instance. */
/*!@}*/

/******************************************************************************/
/*                     DNS_CREATE_ASSEMBLY_REQ                                */
/******************************************************************************/
/*! \brief Create Assembly Data parameters. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_CREATE_ASSEMBLY_Ttag
{
  uint32_t ulInstance;
  uint32_t ulFlags;
  uint32_t ulSize;
  uint32_t ulOffset;
}DNS_CREATE_ASSEMBLY_T;

#define DNS_CREATE_ASSEMBLY_REQ_SIZE (sizeof(DNS_CREATE_ASSEMBLY_T))

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_CREATE_ASSEMBLY_REQ_Ttag
{
  HIL_PACKET_HEADER_T    tHead;   /*!< Packet header. */
  DNS_CREATE_ASSEMBLY_T  tData;   /*!< Create Assembly Data parameters. */
}DNS_PACKET_CREATE_ASSEMBLY_REQ_T;

/******************************************************************************/
/*                     DNS_CMD_CREATE_ASSEMBLY_CNF                            */
/******************************************************************************/
#define DNS_CREATE_ASSEMBLY_CNF_SIZE (sizeof(DNS_CREATE_ASSEMBLY_T))

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_CREATE_ASSEMBLY_CNF_Ttag
{
  HIL_PACKET_HEADER_T   tHead;  /*!< Packet header. */
  DNS_CREATE_ASSEMBLY_T tData;  /*!< Create Assembly Data parameters. */
}DNS_PACKET_CREATE_ASSEMBLY_CNF_T;


#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_UNPACK_1(DNS_PKT_CREATE_ASSEMBLY)
#endif

/*!@}*/
#endif /* __DNS_PACKET_CREATE_ASSEMBLY */
