/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
  DeviceNet_Results.h
**************************************************************************************/

#ifndef __DEVICE_NET_RESULTS_H
#define __DEVICE_NET_RESULTS_H

#include<stdint.h>

/////////////////////////////////////////////////////////////////////////////////////
// DeviceNet GCI Adapter Slave.
/////////////////////////////////////////////////////////////////////////////////////
//
// MessageId: ERR_DEVNET_GCI_DNS_UNKNOWN
//
// MessageText:
//
// DevicNet GCI Adapter  Slave unknown error.
//
#define ERR_DEVNET_GCI_DNS_UNKNOWN       ((uint32_t)0xC11B0000L)

//
// MessageId: ERR_DEVNET_GCI_DNS_NETWORK_POWER_LOSS
//
// MessageText:
//
// 24V Network Power Missing.
//
#define ERR_DEVNET_GCI_DNS_NETWORK_POWER_LOSS ((uint32_t)0xC11B0001L)

//
// MessageId: ERR_DEVNET_GCI_DNS_DUPLICATE_MAC_DETECTED
//
// MessageText:
//
// Duplicate MAC ID found.
//
#define ERR_DEVNET_GCI_DNS_DUPLICATE_MAC_DETECTED ((uint32_t)0xC11B0002L)

//
// MessageId: ERR_DEVNET_GCI_DNS_CAN_BUS_OFF
//
// MessageText:
//
// Network error CAN BUS OFF detected.
//
#define ERR_DEVNET_GCI_DNS_CAN_BUS_OFF   ((uint32_t)0xC11B0003L)

//
// MessageId: ERR_DEVNET_GCI_DNS_WRONG_OR_MISSING_CONFIGURATION
//
// MessageText:
//
// The configuration is missing or not correct.
//
#define ERR_DEVNET_GCI_DNS_WRONG_OR_MISSING_CONFIGURATION ((uint32_t)0xC11B0004L)

//
// MessageId: ERR_DEVNET_GCI_DNS_CONFIGURED_BY_DATABASE
//
// MessageText:
//
// The device is already configured by a database file.
//
#define ERR_DEVNET_GCI_DNS_CONFIGURED_BY_DATABASE ((uint32_t)0xC11B0005L)

/////////////////////////////////////////////////////////////////////////////////////
// DeviceNet Object Library.
/////////////////////////////////////////////////////////////////////////////////////
//
// MessageId: ERR_DEVNET_OBJECT_UNKNOWN
//
// MessageText:
//
// DeviceNet Object unknown error.
//
#define ERR_DEVNET_OBJECT_UNKNOWN        ((uint32_t)0xC11A0000L)

//
// MessageId: ERR_DEVNET_OBJECT_USER_OBJECT_ALREADY_REGISTERED
//
// MessageText:
//
// DeviceNet Object already registered.
//
#define ERR_DEVNET_OBJECT_USER_OBJECT_ALREADY_REGISTERED ((uint32_t)0xC11A0001L)

//
// MessageId: ERR_DEVNET_OBJECT_USER_OBJECT_REGISTER_LIMIT_REACHED
//
// MessageText:
//
// The maximum number of objects that can be registered is reached.
//
#define ERR_DEVNET_OBJECT_USER_OBJECT_REGISTER_LIMIT_REACHED ((uint32_t)0xC11A0002L)

/////////////////////////////////////////////////////////////////////////////////////
// DeviceNet Core.
/////////////////////////////////////////////////////////////////////////////////////
//
// MessageId: ERR_DEVNET_CORE_UNKNOWN
//
// MessageText:
//
// DeviceNet core unknown error.
//
#define ERR_DEVNET_CORE_UNKNOWN          ((uint32_t)0xC1190000L)

//
// MessageId: ERR_DEVNET_CORE_INVALID_PRODUCED_SIZE
//
// MessageText:
//
// Invalid produce size.
//
#define ERR_DEVNET_CORE_INVALID_PRODUCED_SIZE ((uint32_t)0xC1190001L)

//
// MessageId: ERR_DEVNET_CORE_INVALID_CONSUMED_SIZE
//
// MessageText:
//
// Invalid consume size.
//
#define ERR_DEVNET_CORE_INVALID_CONSUMED_SIZE ((uint32_t)0xC1190002L)

//
// MessageId: ERR_DEVNET_CORE_NO_BUS_COMMUNICATION
//
// MessageText:
//
// No network communication.
//
#define ERR_DEVNET_CORE_NO_BUS_COMMUNICATION ((uint32_t)0xC1190003L)

#endif  /* __DEVICE_NET_RESULTS_H */
