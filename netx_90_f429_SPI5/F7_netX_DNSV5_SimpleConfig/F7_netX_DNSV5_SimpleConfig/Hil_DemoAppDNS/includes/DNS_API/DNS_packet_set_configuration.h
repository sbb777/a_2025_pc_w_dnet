/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
This header provides the DeviceNet Slave specific packet definitions of the command

  - DNS_CMD_SET_CONFIGURATION_REQ

These definitions contribute to the LFW API of the DeviceNet Slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/
#ifndef __DNS_PACKET_SET_CONIGURATION_H
#define __DNS_PACKET_SET_CONIGURATION_H

#include <stdint.h>
#include "Hil_Packet.h"
#include "Hil_Compiler.h"

/* pragma pack */
#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_PACK_1(DNS_PKT_SET_CONFIG)
#endif

/*!\defgroup dnsConfiguration Set Configuration Service
 *
 * Set Configuration Service
 * =====================================
 * Before the stack can be used it should be configured first.
 *@{*/


/*!\name System flags
 * \anchor SysFlags
 * The System Flags defines the startup behaviour of the stack.
 * It also defines if the hardware switches are enabled.
 *@{*/
#define DNS_SYS_FLG_MANUAL_START                                     0x00000001  /*!< Manual Start Enable: \n
 * 0: Automatic. Network connections are opened automatically regardless of the state of the host application. \n
 * 1: Application controlled. The firmware is forced to wait for the host application
 * to set the Application Ready flag in the communication change of state register.
    */
#define DNS_SYS_FLG_ADR_SW_ENABLE                                    0x00000010  /*!< Address Switch Enable: \n
  * If this bit is set, the handling mechanism of address switch is activated and
  * attribute 6 and 8 of the DeviceNet object are enabled.  */
#define DNS_SYS_FLG_BAUD_SW_ENABLE                                   0x00000020  /*!< Baud rate Switch Enable: \n
  * If this bit is set, the handling mechanism of baud rate switch is activated
  * and attribute 7 and 9 of the DeviceNet object are enabled.*/
#define DNS_SYS_FLG_RESERVED                                         0xFFFFFFCE  /*!< Reserved Values.         */
/*!@}*/
/*****************************************************************************/
/*                             DNS Baudrates                                 */
/*****************************************************************************/
 /*!\name Baudrate
  * \anchor Baudrate
  * @{*/
/*! Baudrate 125 kBit/s (default) */
#define   DNS_BAUDRATE_125kB                                                 0
/*! Baudrate 250 kBit/s */
#define   DNS_BAUDRATE_250kB                                                 1
/*! Baudrate 500 kBit/s */
#define   DNS_BAUDRATE_500kB                                                 2
/*!@}*/

 /*!\name Configuration flags
  * @{*/
#define DNS_CFG_FLAG_INDICATION_NETWORK_POWER                        0x80000000 /*!< 24V NP Indication Enable: \n
 * If this bit is set, the 24V Network Power status is send to host within the
 * reset indication packet. */
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_MSK                         0x00000300 /*!< Message Body Format Mask */
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_8_8                         0x00000000 /*!<  8 Bit Class  8 Bit Instance (default) */
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_8_16                        0x00000100 /*!<  8 Bit Class 16 Bit Instance */
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_16_16                       0x00000200 /*!< 16 Bit Class 16 Bit Instance */
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_16_8                        0x00000300 /*!< 16 Bit Class  8 Bit Instance */


#define DNS_CFG_FLAG_CONNECTION_DISABLE_MSK                          0x0000F000 /*!< Connection disable Mask */
#define DNS_CFG_FLAG_CONNECTION_DISABLE_POLL                         0x00001000 /*!< Disable POLL connection */
#define DNS_CFG_FLAG_CONNECTION_DISABLE_STROBE                       0x00002000 /*!< Disable BIT STROBE connection */
#define DNS_CFG_FLAG_CONNECTION_DISABLE_COS                          0x00004000 /*!< Disable CHANGE OF STATE connection */
#define DNS_CFG_FLAG_CONNECTION_DISABLE_CYC                          0x00008000 /*!< Disable CYCLIC connection */

#define DNS_CFG_FLAG_ENABLE_TXD_UPDATE_CONTROLLED_POLL_RES           0x00010000

#define DNS_CFG_FLAG_RESERVED                                        0x7FFE0CFF
/*! Object Configuration flags reserved. */
#define DNS_OBJ_FLAG_RESERVED                                        0xFFFFFFFF
/*!@}*/

/*! Configuration structure version */
#define DNS_CONFIGURATION_V1                                                  1

/*! \brief Version 1 Configuration parameters. */

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_CONFIGURATION_V1_Ttag
{
  uint32_t ulSystemFlags; /*!< \ref SysFlags "System Flags" defines the startup behaviour of the stack.
  * It also defines if the hardware switches are enabled. */
  uint32_t ulWdgTime; /*!< Host Watchdog Time in ms. \n 0 = watchdog is disabled \n
                          min = 20; default = 1000; max = 65535. */
  uint32_t ulNodeId; /*!< Node ID: MAC ID of the DeviceNet Slave in the network. */
  uint32_t ulBaudrate; /*!< \ref Baudrate */

  uint32_t ulConfigFlags; /*!< Configuration flags. These flags are not used, set 0. */
  uint32_t ulObjectFlags; /*!< Object Configuration flags. These flags are not used, set 0. */

  uint16_t usVendorId; /*!< DeviceNet specific unique number which is fixed by the ODVA for each DeviceNet manufacturer.
  * The DNS task itself uses this ID during the Duplicate MAC-ID check phase and within each sent Duplicate MAC-Id check response.
  * The value range of this variable is not limited. The Hilscher ID is 283 decimal.*/
  uint16_t usDeviceType; /*!< Identification of the general type of product.
  * The Hilscher standard value is 12 which is a Communications Adapter. */
  uint16_t usProductCode; /*!< Identification of a particular product within a defined device type. */
  uint8_t  bMinorRev; /*!< First part of the revision which identifies the revision of the DNS device.
  * The revision attribute consists of Major and Minor Revisions and they are typically displayed as major.minor.*/
  uint8_t  bMajorRev; /*!< Second part of the revision. The Major Revision attribute is limited to 7 bits.
  * The eighth bit is reserved by DeviceNet and must have a default value of zero.*/
  uint32_t ulSerialNumber; /*!< Unique serial number of the device. If the value set 0 the stack will
  * take the serial from a device internal source like security memory or device data provider.
  * If no internal source is available the stack will set the serial number to 1. */
  uint8_t  abReserved[3]; /*!< Reserved, set to 0. */
  uint8_t  bProductNameLen; /*!< Length of abProdName string. The maximum number of characters in this string is 32. */
  uint8_t  abProductName[32]; /*!< ASCII text string that should represent a short description of the product/product family.
  * The maximum number of characters in this string is 32. The number of characters must be set in the variable bProdNameLen. */

  uint32_t ulProduceAsInstance; /*!< Instance number of input assembly (Master to Slave). */
  uint32_t ulProduceAsFlags; /*!< Produce Assembly Flags. */
  uint32_t ulProduceAsSize; /*!< Number of input bytes the DNS task shall produce in the view of a master for each established connection.
  * The bytes which shall be produced then must be handed over in the send data area of the dual-port memory.*/

  uint32_t ulConsumeAsInstance; /*!< Instance number of output assembly (Slave to Master). */
  uint32_t ulConsumeAsFlags; /*!<Consume Assembly Flags. */
  uint32_t ulConsumeAsSize; /*!< Number of output bytes the DNS task shall consume in the view of a master for each established connection.
  * The bytes which are received are handed over in the receive data area of the dual-port memory.*/
}DNS_CONFIGURATION_V1_T;

/*! \brief Request Packet Data. */
typedef __HIL_PACKED_PRE union __HIL_PACKED_POST DNS_SET_CONFIGURATION_REQ_Utag
{
  DNS_CONFIGURATION_V1_T tV1;           /*!< Version 1 Configuration parameters. */
}DNS_SET_CONFIGURATION_REQ_U;

/*! \brief Set Configuration Data Structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_SET_CONFIGURATION_REQ_Ttag
{
  uint32_t ulVersion;                   /*!< Version of Set Configuration.  */
  DNS_SET_CONFIGURATION_REQ_U unCfg;    /*!< Configuration union. */
}DNS_SET_CONFIGURATION_REQ_T;

/*! \brief Set Configuration Packet Structure. */
/* \anchor CfgPacket */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_SET_CONFIGURATION_REQ_Ttag
{
  HIL_PACKET_HEADER_T          tHead;   /*!< Packet header. */
  DNS_SET_CONFIGURATION_REQ_T  tData;   /*!< Set Configuration Data Structure. */
}DNS_PACKET_SET_CONFIGURATION_REQ_T;

#define DNS_SET_CONFIGURATION_V1_REQ_SIZE  (sizeof(DNS_CONFIGURATION_V1_T)+4)

/*! \brief Basic Parameters.
 * Basic configuration parameters. */
/* Confirmation Packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST DNS_PACKET_SET_CONFIGURATION_CNF_Ttag
{
  HIL_PACKET_HEADER_T              tHead;   /*!< Packet header. */
}DNS_PACKET_SET_CONFIGURATION_CNF_T;

#define DNS_SET_CONFIGURATION_CNF_SIZE   0

/*!@}*/

/* pragma unpack */
#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_UNPACK_1(DNS_PKT_SET_CONFIG)
#endif



#endif /* __DNS_PACKET_SET_CONIGURATION_H */
