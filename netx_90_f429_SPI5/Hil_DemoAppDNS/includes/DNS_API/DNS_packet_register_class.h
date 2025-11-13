/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
This header provides the DeviceNet Slave specific packet definitions of the command

  - DNS_CMD_REGISTER_CLASS_REQ/CNF
  - DNS_CMD_UNREGISTER_CLASS_REQ/CNF

These definitions contribute to the LFW API of the DeviceNet Slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/
#ifndef __DNS_PACKET_REGISTER_CLASS_H
#define __DNS_PACKET_REGISTER_CLASS_H

#include <stdint.h>
#include "Hil_Packet.h"
#include "Hil_Compiler.h"

/*!\defgroup dnsRegClassService Register Class Service
 *
 * Register Class Service
 * =====================================
 * The DeviceNet Slave stack has the option to forward explicit services like Get_Attribute,
 * Set_Attribute or other to the user application. Therefore the user must register
 * the corresponding class within the stack. This must be done for each class
 * the user wants to have the explicit service indications.

 *@{*/

#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_PACK_1(DNS_PKT_REGISTER_CLASS)
#endif

/******************************************************************************/
/*                     Predefined User CIP Objects                            */
/******************************************************************************/
/*!\name Register Predefined Class Ranges
 *@{*/
/*! Start of predefined CIP Object Range 1. */
#define DNS_REGISTER_PREDEFINED_CIP_CLASS_RANGE1_START                  (0x0007)
/*! End of predefined CIP Object Range 1. */
#define DNS_REGISTER_PREDEFINED_CIP_CLASS_RANGE1_END                    (0x002A)
/*! Start of predefined CIP Object Range 2. */
#define DNS_REGISTER_PREDEFINED_CIP_CLASS_RANGE2_START                  (0x002C)
/*! End of predefined CIP Object Range 2. */
#define DNS_REGISTER_PREDEFINED_CIP_CLASS_RANGE2_END                    (0x00FF)
/*! Start of predefined CIP Object Range 3. (if Message Format 16/8 or 16/16) */
#define DNS_REGISTER_PREDEFINED_CIP_CLASS_RANGE3_START                  (0x0100)
/*! End of predefined CIP Object Range 3.   (if Message Format 16/8 or 16/16) */
#define DNS_REGISTER_PREDEFINED_CIP_CLASS_RANGE3_END                    (0xFFFF)

/*!@}*/
/******************************************************************************/
/*                     DNS_REGISTER_CLASS_REQ                                 */
/******************************************************************************/
/*! \brief Register Class Data parameters. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_REGISTER_CLASS_Ttag
{
  uint32_t ulClass;                          /*!< CIP class code            */
  uint32_t ulServiceMask;                    /*!< CIP service filter        */
}DNS_REGISTER_CLASS_T;

/*! Size of Register Class Request. */
#define DNS_REGISTER_CLASS_REQ_SIZE (sizeof(DNS_REGISTER_CLASS_T))

/*! \brief Register Class Request Packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_REGISTER_CLASS_REQ_Ttag
{
  HIL_PACKET_HEADER_T   tHead;  /*!< Packet header. */
  DNS_REGISTER_CLASS_T  tData;  /*!< Register Class Data parameters. */
}DNS_PACKET_REGISTER_CLASS_REQ_T;

/******************************************************************************/
/*                     DNS_CMD_REGISTER_CLASS_CNF                             */
/******************************************************************************/
/*! \brief Register Class Confirmation Packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_REGISTER_CLASS_CNF_Ttag
{
  HIL_PACKET_HEADER_T   tHead;   /*!< Packet header. */
  DNS_REGISTER_CLASS_T  tData;   /*!< Register Class Data parameters. */
}DNS_PACKET_REGISTER_CLASS_CNF_T;


/******************************************************************************/
/*                     DNS_UNREGISTER_CLASS_REQ                               */
/******************************************************************************/
/*! \brief Unregister Class Data parameters. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_UNREGISTER_CLASS_Ttag
{
  uint32_t ulClass;                          /*!< CIP class code            */
  uint32_t ulServiceMask;                    /*!< CIP service filter        */
}DNS_UNREGISTER_CLASS_T;

/*! Size of Unregister Class Request. */
#define DNS_UNREGISTER_CLASS_REQ_SIZE (sizeof(DNS_UNREGISTER_CLASS_T))

/*! \brief Unregister Class Request Packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_UNREGISTER_CLASS_REQ_Ttag
{
  HIL_PACKET_HEADER_T      tHead;     /*!< Packet header. */
  DNS_UNREGISTER_CLASS_T   tData;     /*!< Unregister Class Data parameters. */

}DNS_PACKET_UNREGISTER_CLASS_REQ_T;

/******************************************************************************/
/*                  DNS_CMD_UNREGISTER_CLASS_CNF                              */
/******************************************************************************/
/*! \brief Unregister Class Confirmation Packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_UNREGISTER_CLASS_CNF_Ttag
{
  HIL_PACKET_HEADER_T      tHead;   /*!< Packet header. */
  DNS_UNREGISTER_CLASS_T   tData;   /*!< Unregister Class Data parameters. */
}DNS_PACKET_UNREGISTER_CLASS_CNF_T;


#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_UNPACK_1(DNS_PKT_REGISTER_CLASS)
#endif

/*!@}*/
#endif /* __DNS_PACKET_REGISTER_CLASS_H */
