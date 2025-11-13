/**************************************************************************************
 * DeviceNet Master Information Query via CIP Explicit Messaging
 *
 * This module provides functions to query DeviceNet Master device information
 * using CIP Explicit Messaging (Get_Attribute_Single service)
 **************************************************************************************/

#ifndef DEVICENET_MASTER_INFO_H_
#define DEVICENET_MASTER_INFO_H_

#include <stdint.h>
#include <stdbool.h>
#include "cifXUser.h"
#include "cifXErrors.h"

/*****************************************************************************/
/*! DeviceNet Master Identity Information                                   */
/*****************************************************************************/
typedef struct DEVICENET_MASTER_IDENTITY_Ttag
{
    uint16_t usVendorID;              /* Vendor ID */
    uint16_t usDeviceType;            /* Device Type */
    uint16_t usProductCode;           /* Product Code */
    uint8_t  bRevisionMajor;          /* Revision Major */
    uint8_t  bRevisionMinor;          /* Revision Minor */
    uint16_t usStatus;                /* Device Status */
    uint32_t ulSerialNumber;          /* Serial Number */
    uint8_t  bProductNameLength;      /* Product Name Length */
    char     szProductName[64];       /* Product Name String */
    bool     bDataValid;              /* Data validity flag */
} DEVICENET_MASTER_IDENTITY_T;

/*****************************************************************************/
/*! Function Prototypes                                                     */
/*****************************************************************************/

/**
 * @brief Query DeviceNet Master Identity Object
 *
 * @param hChannel       cifX channel handle
 * @param masterNodeId   Master node address (usually 0 or 63)
 * @param ptIdentity     Pointer to identity structure
 *
 * @return CIFX_NO_ERROR on success, error code otherwise
 */
int32_t DeviceNet_GetMasterIdentity(CIFXHANDLE hChannel,
                                    uint8_t masterNodeId,
                                    DEVICENET_MASTER_IDENTITY_T* ptIdentity);

/**
 * @brief Get specific attribute from Master Identity Object
 *
 * @param hChannel       cifX channel handle
 * @param masterNodeId   Master node address
 * @param attributeId    CIP Identity attribute ID (1-7)
 * @param pData          Buffer for attribute data
 * @param dataSize       Size of data buffer
 * @param pReceivedSize  Actual received data size
 *
 * @return CIFX_NO_ERROR on success, error code otherwise
 */
int32_t DeviceNet_GetMasterAttribute(CIFXHANDLE hChannel,
                                     uint8_t masterNodeId,
                                     uint8_t attributeId,
                                     void* pData,
                                     uint32_t dataSize,
                                     uint32_t* pReceivedSize);

/**
 * @brief Print Master Identity Information
 *
 * @param ptIdentity     Pointer to identity structure
 */
void DeviceNet_PrintMasterIdentity(const DEVICENET_MASTER_IDENTITY_T* ptIdentity);

/**
 * @brief Scan DeviceNet network to find Master node
 *
 * @param hChannel       cifX channel handle
 * @param pMasterNodeId  Pointer to store found master node ID
 *
 * @return CIFX_NO_ERROR on success, error code otherwise
 */
int32_t DeviceNet_ScanForMaster(CIFXHANDLE hChannel, uint8_t* pMasterNodeId);

#endif /* DEVICENET_MASTER_INFO_H_ */
