/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
This header defines all specific packet command codes of the DeviceNet slave stack.

These definitions contribute to the LFW API of the DeviceNet slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/

#ifndef DNS_PACKET_COMMANDS_H_
#define DNS_PACKET_COMMANDS_H_

/*!\defgroup dnsPacketInterface DNS Packet Interface

DNS Packet Interface
=====================================
Stack specific commands for packet based interaction.
The packet interface allows the user to:
  - configure the stack,
  - to exchange data with the stack, and
  - to be notified about stack events.
@{*/

/*! \name Supported Commands
Command Ids supported by the stack - Requests and Indications.
@{*/
/*!Set Configuration Request Command.
 * Before the stack can be used it should be configured first. */
#define DNS_CMD_SET_CONFIGURATION_REQ        0x0000B100 /*!< \ref dnsConfiguration */
 /*!Set Configuration Confirmation Command. */
#define DNS_CMD_SET_CONFIGURATION_CNF        0x0000B101 /*!< \ref dnsConfiguration */

/*!This service is used to perform a CIP service from the host application
* to any object of the local DeviceNet Slave stack. */
#define DNS_CMD_CIP_SERVICE_REQ              0x0000B102 /*!< \ref dnsCipServiceReq */
 /*! CIP Service Confirmation Command. */
#define DNS_CMD_CIP_SERVICE_CNF              0x0000B103 /*!< \ref dnsCipServiceReq */

/*! This packet indicates that the remote DeviceNet Master has requested a service from the Slave.
 * The user receives the service indication only for classes
 * that have been registered to the DeviceNet Slave stack. */
#define DNS_CMD_CIP_SERVICE_IND              0x0000B104 /*!< \ref dnsCipServiceInd */
 /*! CIP Service Response Command. */
#define DNS_CMD_CIP_SERVICE_RES              0x0000B105 /*!< \ref dnsCipServiceInd */

/*! The DeviceNet Slave stack has the option to forward explicit services
 * like Get_Attribute or Set_Attribute or other services to the user application.
 * Therefore the user must register the corresponding class within the stack to get these services. */
#define DNS_CMD_REGISTER_CLASS_REQ           0x0000B106 /*!< \ref dnsRegClassService */
 /*! Register Class Confirmation Command. */
#define DNS_CMD_REGISTER_CLASS_CNF           0x0000B107 /*!< \ref dnsRegClassService */

/*! This command will unregister a previously registered class.
 * If unregistering successfully, the service to the class will be (no longer)
 * passed through to the host application. */
#define DNS_CMD_UNREGISTER_CLASS_REQ         0x0000B108 /*!< \ref dnsRegClassService */
 /*! Unregister Class Confirmation Command.*/
#define DNS_CMD_UNREGISTER_CLASS_CNF         0x0000B109 /*!< \ref dnsRegClassService */

/*! This service allows to create additional assembly objects.
 * Creating additional assembly objects can be used
 * when more than the two default input and output assembly shall be supported. */
#define DNS_CMD_CREATE_ASSEMBLY_REQ          0x0000B10A /*!< \ref dnsCreateAsm */
 /*! Create Assembly Confirmation Command. */
#define DNS_CMD_CREATE_ASSEMBLY_CNF          0x0000B10B /*!< \ref dnsCreateAsm */

/*! The reset indication is send from the stack to the host whenever a reset of the device is required.
 * It is only a notification about a reset that has been requested.*/
#define DNS_CMD_RESET_IND                    0x0000B10C /*!< \ref dnsResetInd */
 /*! Reset Response Command .*/
#define DNS_CMD_RESET_RES                    0x0000B10D /*!< \ref dnsResetInd */

#define DNS_CMD_DIAG_REQ                     0x0000B10E
#define DNS_CMD_DIAG_CNF                     0x0000B10F
/*!@}*/
/*!@}*/
#endif /* DNS_PACKET_COMMANDS_H_ */
