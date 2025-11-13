/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
This header provides the DeviceNet Slave specific packet definitions of the command

  - DNS_CMD_CIP_SERVICE_REQ/CNF
  - DNS_CMD_CIP_SERVICE_IND/RES

These definitions contribute to the LFW API of the DeviceNet Slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/
#ifndef __DNS_PACKET_CIP_SERVICE_H
#define __DNS_PACKET_CIP_SERVICE_H

#include <stdint.h>
#include "Hil_Packet.h"
#include "Hil_Compiler.h"

/*!\defgroup dnsCipServiceReq CIP Service Request
 *
 * CIP Service Request Packet
 * =====================================
 * This packet is used to perform a CIP service from the host application
 * to any object of the local DeviceNet Slave stack.
 *@{*/

#define DNS_CIP_SERVICE_MAX_DATA_LEN                                            255 /*!< CIP Service maximal data lenght. */

/* pragma pack */
#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_PACK_1(DNS_PKT_CIP_SERVICE)
#endif


/************************************************************************************/
/*                      DNS_CIP_SERVICE_REQ/CNF                                  */
/************************************************************************************/
/*! \brief CIP Request Data parameters. */

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_CIP_SERVICE_REQ_Ttag
{
  uint32_t    ulService;                            /*!< CIP service code          */
  uint32_t    ulClass;                              /*!< CIP class ID              */
  uint32_t    ulInstance;                           /*!< CIP instance number       */
  uint32_t    ulAttribute;                          /*!< CIP attribute number      */
  uint32_t    ulMember;                             /*!< CIP member number         */
  uint8_t     abData[DNS_CIP_SERVICE_MAX_DATA_LEN]; /*!< CIP Service data.         */
} DNS_CIP_SERVICE_REQ_T;

/*! \brief CIP Request Packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_CIP_SERVICE_REQ_Ttag
{
  HIL_PACKET_HEADER_T         tHead;  /*!< Packet header. */
  DNS_CIP_SERVICE_REQ_T       tData;  /*!< CIP Request Data parameters. */
}DNS_PACKET_CIP_SERVICE_REQ_T;

/*! Size of the CIP Service Request excluding CIP Service data. */
#define DNS_CIP_SERVICE_REQ_SIZE     (sizeof(DNS_CIP_SERVICE_REQ_T) - DNS_CIP_SERVICE_MAX_DATA_LEN)

/*! \brief CIP Confirmation Data parameters. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_CIP_SERVICE_CNF_Ttag
{
  uint32_t    ulService;                            /*!< CIP service code          */
  uint32_t    ulClass;                              /*!< CIP class ID              */
  uint32_t    ulInstance;                           /*!< CIP instance number       */
  uint32_t    ulAttribute;                          /*!< CIP attribute number      */
  uint32_t    ulMember;                             /*!< CIP member number         */
  uint32_t    ulGRC;                                /*!< Generic Error Code        */
  uint32_t    ulERC;                                /*!< Extended Error Code       */
  uint8_t     abData[DNS_CIP_SERVICE_MAX_DATA_LEN]; /*!< CIP service data.         */

} DNS_CIP_SERVICE_CNF_T;

/*! \brief CIP Confirmation Packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_CIP_SERVICE_CNF_Ttag
{
  HIL_PACKET_HEADER_T         tHead;  /*!< Packet header. */
  DNS_CIP_SERVICE_CNF_T       tData;  /*!< CIP Confirmation Data parameters. */
}DNS_PACKET_CIP_SERVICE_CNF_T;

/*! Size of the CIP Service Confirmation excluding CIP Service data. */
#define DNS_CIP_SERVICE_CNF_SIZE      (sizeof(DNS_CIP_SERVICE_CNF_T) - DNS_CIP_SERVICE_MAX_DATA_LEN)
/*!@}*/

/************************************************************************************/
/*                      DNS_CIP_SERVICE_IND/RES                                     */
/************************************************************************************/
/*!\defgroup dnsCipServiceInd CIP Service Indication
 *
 * CIP Service Indication Packet
 * =====================================
 * This packet indicates that the remote DeviceNet Master has requested a service from the Slave.
 * The user receives the service indication only for classes that have been registered to the DeviceNet Slave stack.
 *
 *@{*/

/*! CIP Indication Data parameters. Same as CIP Service Request Data parameters.*/
#define DNS_CIP_SERVICE_IND_T DNS_CIP_SERVICE_REQ_T

/*! \brief CIP Indication Packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_CIP_SERVICE_IND_Ttag
{
  HIL_PACKET_HEADER_T         tHead;   /*!< Packet header. */
  DNS_CIP_SERVICE_IND_T       tData;   /*!< CIP Indication Data parameters. */
}DNS_PACKET_CIP_SERVICE_IND_T;

/*! Size of the CIP Service Indication excluding CIP Service data. */
#define DNS_CIP_SERVICE_IND_SIZE     (sizeof(DNS_CIP_SERVICE_IND_T) - DNS_CIP_SERVICE_MAX_DATA_LEN)

/*! CIP Response Data parameters. Same as CIP Service Confirmation Data parameters.*/
#define DNS_CIP_SERVICE_RES_T DNS_CIP_SERVICE_CNF_T

/*! \brief CIP Response Packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_CIP_SERVICE_RES_Ttag
{
  HIL_PACKET_HEADER_T         tHead;    /*!< Packet header. */
  DNS_CIP_SERVICE_RES_T       tData;    /*!< CIP Response Data parameters. */
}DNS_PACKET_CIP_SERVICE_RES_T;

#define DNS_CIP_SERVICE_RES_SIZE      (sizeof(DNS_CIP_SERVICE_RES_T) - DNS_CIP_SERVICE_MAX_DATA_LEN)

/* pragma unpack */
#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_UNPACK_1(DNS_PKT_CIP_SERVICE)
#endif

/*!@}*/
#endif /* __DNS_PACKET_CIP_SERVICE_H */
