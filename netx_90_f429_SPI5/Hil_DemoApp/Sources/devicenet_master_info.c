/**************************************************************************************
 * DeviceNet Master Information Query via CIP Explicit Messaging
 *
 * This module provides functions to query DeviceNet Master device information
 * using CIP Explicit Messaging (Get_Attribute_Single service)
 **************************************************************************************/

#include "devicenet_master_info.h"
#include <stdio.h>
#include <string.h>
#include "DNS_packet_cip_service.h"
#include "DNS_packet_commands.h"
#include "CIP_common_definitions.h"

/* CIP Identity Object Class Attributes */
#define CIP_IDENTITY_ATTR_VENDOR_ID        1
#define CIP_IDENTITY_ATTR_DEVICE_TYPE      2
#define CIP_IDENTITY_ATTR_PRODUCT_CODE     3
#define CIP_IDENTITY_ATTR_REVISION         4
#define CIP_IDENTITY_ATTR_STATUS           5
#define CIP_IDENTITY_ATTR_SERIAL_NUMBER    6
#define CIP_IDENTITY_ATTR_PRODUCT_NAME     7

/* Packet timeout in milliseconds */
#define PACKET_TIMEOUT_MS                  5000

/*****************************************************************************/
/*! Get specific attribute from Master Identity Object                     */
/*****************************************************************************/
int32_t DeviceNet_GetMasterAttribute(CIFXHANDLE hChannel,
                                     uint8_t masterNodeId,
                                     uint8_t attributeId,
                                     void* pData,
                                     uint32_t dataSize,
                                     uint32_t* pReceivedSize)
{
    int32_t lRet;
    DNS_PACKET_CIP_SERVICE_REQ_T tSendPkt;
    DNS_PACKET_CIP_SERVICE_CNF_T tRecvPkt;
    uint32_t ulRecvLen = sizeof(tRecvPkt);

    if(!hChannel || !pData || !pReceivedSize)
        return CIFX_INVALID_POINTER;

    /* Initialize request packet */
    memset(&tSendPkt, 0, sizeof(tSendPkt));

    /* Setup packet header */
    tSendPkt.tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
    tSendPkt.tHead.ulSrc = 0;
    tSendPkt.tHead.ulDestId = 0;
    tSendPkt.tHead.ulSrcId = 0;
    tSendPkt.tHead.ulLen = sizeof(DNS_CIP_SERVICE_REQ_T);
    tSendPkt.tHead.ulId = 0;
    tSendPkt.tHead.ulSta = 0;
    tSendPkt.tHead.ulCmd = DNS_CMD_CIP_SERVICE_REQ;
    tSendPkt.tHead.ulExt = 0;
    tSendPkt.tHead.ulRout = 0;

    /* Setup CIP service request */
    tSendPkt.tData.ulService = CIP_SERVICE_GET_ATTRIBUTE_SINGLE;
    tSendPkt.tData.ulClass = CIP_CLASS_IDENTITY;
    tSendPkt.tData.ulInstance = masterNodeId;  /* Master node ID as instance */
    tSendPkt.tData.ulAttribute = attributeId;
    tSendPkt.tData.ulMember = 0;

    /* Send request packet */
    lRet = xChannelPutPacket(hChannel, (CIFX_PACKET*)&tSendPkt, PACKET_TIMEOUT_MS);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("Error: Failed to send CIP service request (0x%08X)\r\n", (unsigned int)lRet);
        return lRet;
    }

    /* Receive response packet */
    memset(&tRecvPkt, 0, sizeof(tRecvPkt));
    lRet = xChannelGetPacket(hChannel, sizeof(tRecvPkt), (CIFX_PACKET*)&tRecvPkt, PACKET_TIMEOUT_MS);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("Error: Failed to receive CIP service response (0x%08X)\r\n", (unsigned int)lRet);
        return lRet;
    }

    /* Check response status */
    if(tRecvPkt.tHead.ulSta != CIFX_NO_ERROR)
    {
        printf("Error: CIP service response error (0x%08X)\r\n", (unsigned int)tRecvPkt.tHead.ulSta);
        return tRecvPkt.tHead.ulSta;
    }

    /* Copy received data */
    uint32_t ulDataLen = tRecvPkt.tHead.ulLen - DNS_CIP_SERVICE_CNF_SIZE;
    if(ulDataLen > dataSize)
        ulDataLen = dataSize;

    memcpy(pData, tRecvPkt.tData.abData, ulDataLen);
    *pReceivedSize = ulDataLen;

    return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Query DeviceNet Master Identity Object                                 */
/*****************************************************************************/
int32_t DeviceNet_GetMasterIdentity(CIFXHANDLE hChannel,
                                    uint8_t masterNodeId,
                                    DEVICENET_MASTER_IDENTITY_T* ptIdentity)
{
    int32_t lRet;
    uint32_t ulRecvSize;
    uint8_t abBuffer[256];

    if(!hChannel || !ptIdentity)
        return CIFX_INVALID_POINTER;

    /* Initialize identity structure */
    memset(ptIdentity, 0, sizeof(DEVICENET_MASTER_IDENTITY_T));
    ptIdentity->bDataValid = false;

    printf("\r\n=== Querying DeviceNet Master Identity (Node %d) ===\r\n", masterNodeId);

    /* Query Vendor ID (Attribute 1) */
    lRet = DeviceNet_GetMasterAttribute(hChannel, masterNodeId,
                                        CIP_IDENTITY_ATTR_VENDOR_ID,
                                        abBuffer, sizeof(abBuffer), &ulRecvSize);
    if(lRet == CIFX_NO_ERROR && ulRecvSize >= 2)
    {
        ptIdentity->usVendorID = *(uint16_t*)abBuffer;
        printf("Vendor ID: 0x%04X\r\n", ptIdentity->usVendorID);
    }
    else
    {
        printf("Failed to read Vendor ID\r\n");
        return lRet;
    }

    /* Query Device Type (Attribute 2) */
    lRet = DeviceNet_GetMasterAttribute(hChannel, masterNodeId,
                                        CIP_IDENTITY_ATTR_DEVICE_TYPE,
                                        abBuffer, sizeof(abBuffer), &ulRecvSize);
    if(lRet == CIFX_NO_ERROR && ulRecvSize >= 2)
    {
        ptIdentity->usDeviceType = *(uint16_t*)abBuffer;
        printf("Device Type: 0x%04X\r\n", ptIdentity->usDeviceType);
    }

    /* Query Product Code (Attribute 3) */
    lRet = DeviceNet_GetMasterAttribute(hChannel, masterNodeId,
                                        CIP_IDENTITY_ATTR_PRODUCT_CODE,
                                        abBuffer, sizeof(abBuffer), &ulRecvSize);
    if(lRet == CIFX_NO_ERROR && ulRecvSize >= 2)
    {
        ptIdentity->usProductCode = *(uint16_t*)abBuffer;
        printf("Product Code: 0x%04X\r\n", ptIdentity->usProductCode);
    }

    /* Query Revision (Attribute 4) */
    lRet = DeviceNet_GetMasterAttribute(hChannel, masterNodeId,
                                        CIP_IDENTITY_ATTR_REVISION,
                                        abBuffer, sizeof(abBuffer), &ulRecvSize);
    if(lRet == CIFX_NO_ERROR && ulRecvSize >= 2)
    {
        ptIdentity->bRevisionMajor = abBuffer[0];
        ptIdentity->bRevisionMinor = abBuffer[1];
        printf("Revision: %d.%d\r\n", ptIdentity->bRevisionMajor, ptIdentity->bRevisionMinor);
    }

    /* Query Status (Attribute 5) */
    lRet = DeviceNet_GetMasterAttribute(hChannel, masterNodeId,
                                        CIP_IDENTITY_ATTR_STATUS,
                                        abBuffer, sizeof(abBuffer), &ulRecvSize);
    if(lRet == CIFX_NO_ERROR && ulRecvSize >= 2)
    {
        ptIdentity->usStatus = *(uint16_t*)abBuffer;
        printf("Status: 0x%04X\r\n", ptIdentity->usStatus);
    }

    /* Query Serial Number (Attribute 6) */
    lRet = DeviceNet_GetMasterAttribute(hChannel, masterNodeId,
                                        CIP_IDENTITY_ATTR_SERIAL_NUMBER,
                                        abBuffer, sizeof(abBuffer), &ulRecvSize);
    if(lRet == CIFX_NO_ERROR && ulRecvSize >= 4)
    {
        ptIdentity->ulSerialNumber = *(uint32_t*)abBuffer;
        printf("Serial Number: %u\r\n", (unsigned int)ptIdentity->ulSerialNumber);
    }

    /* Query Product Name (Attribute 7) */
    lRet = DeviceNet_GetMasterAttribute(hChannel, masterNodeId,
                                        CIP_IDENTITY_ATTR_PRODUCT_NAME,
                                        abBuffer, sizeof(abBuffer), &ulRecvSize);
    if(lRet == CIFX_NO_ERROR && ulRecvSize > 0)
    {
        /* First byte is string length */
        ptIdentity->bProductNameLength = abBuffer[0];
        if(ptIdentity->bProductNameLength > 0 && ptIdentity->bProductNameLength < 64)
        {
            memcpy(ptIdentity->szProductName, &abBuffer[1], ptIdentity->bProductNameLength);
            ptIdentity->szProductName[ptIdentity->bProductNameLength] = '\0';
            printf("Product Name: %s\r\n", ptIdentity->szProductName);
        }
    }

    /* Mark data as valid if at least vendor ID was successfully read */
    if(ptIdentity->usVendorID != 0)
    {
        ptIdentity->bDataValid = true;
        printf("=== Master Identity Query Complete ===\r\n\r\n");
        return CIFX_NO_ERROR;
    }
    else
    {
        printf("=== Master Identity Query Failed ===\r\n\r\n");
        return CIFX_DEV_NOT_READY;
    }
}

/*****************************************************************************/
/*! Print Master Identity Information                                       */
/*****************************************************************************/
void DeviceNet_PrintMasterIdentity(const DEVICENET_MASTER_IDENTITY_T* ptIdentity)
{
    if(!ptIdentity)
        return;

    printf("\r\n╔════════════════════════════════════════════════════╗\r\n");
    printf("║     DeviceNet Master Identity Information         ║\r\n");
    printf("╠════════════════════════════════════════════════════╣\r\n");

    if(!ptIdentity->bDataValid)
    {
        printf("║  Status: Data Not Valid                           ║\r\n");
        printf("╚════════════════════════════════════════════════════╝\r\n");
        return;
    }

    printf("║  Vendor ID:      0x%04X                            ║\r\n", ptIdentity->usVendorID);
    printf("║  Device Type:    0x%04X                            ║\r\n", ptIdentity->usDeviceType);
    printf("║  Product Code:   0x%04X                            ║\r\n", ptIdentity->usProductCode);
    printf("║  Revision:       %d.%d                              ║\r\n",
           ptIdentity->bRevisionMajor, ptIdentity->bRevisionMinor);
    printf("║  Status:         0x%04X                            ║\r\n", ptIdentity->usStatus);
    printf("║  Serial Number:  %u                                ║\r\n",
           (unsigned int)ptIdentity->ulSerialNumber);

    if(ptIdentity->bProductNameLength > 0)
    {
        printf("║  Product Name:   %-32s ║\r\n", ptIdentity->szProductName);
    }

    printf("╚════════════════════════════════════════════════════╝\r\n\r\n");
}

/*****************************************************************************/
/*! Scan DeviceNet network to find Master node                             */
/*****************************************************************************/
int32_t DeviceNet_ScanForMaster(CIFXHANDLE hChannel, uint8_t* pMasterNodeId)
{
    int32_t lRet;
    DEVICENET_MASTER_IDENTITY_T tIdentity;

    if(!hChannel || !pMasterNodeId)
        return CIFX_INVALID_POINTER;

    printf("\r\n=== Scanning DeviceNet Network for Master ===\r\n");

    /* Common master node addresses */
    uint8_t abCommonMasterNodes[] = {0, 63, 1, 62};

    for(uint32_t i = 0; i < sizeof(abCommonMasterNodes); i++)
    {
        uint8_t nodeId = abCommonMasterNodes[i];
        printf("Trying Node %d...\r\n", nodeId);

        lRet = DeviceNet_GetMasterIdentity(hChannel, nodeId, &tIdentity);
        if(lRet == CIFX_NO_ERROR && tIdentity.bDataValid)
        {
            printf("Master found at Node %d\r\n", nodeId);
            *pMasterNodeId = nodeId;
            DeviceNet_PrintMasterIdentity(&tIdentity);
            return CIFX_NO_ERROR;
        }
    }

    printf("No master found on common addresses, scanning all nodes...\r\n");

    /* Scan all possible node addresses (0-63) */
    for(uint8_t nodeId = 0; nodeId < 64; nodeId++)
    {
        /* Skip already checked common addresses */
        bool bAlreadyChecked = false;
        for(uint32_t i = 0; i < sizeof(abCommonMasterNodes); i++)
        {
            if(nodeId == abCommonMasterNodes[i])
            {
                bAlreadyChecked = true;
                break;
            }
        }

        if(bAlreadyChecked)
            continue;

        lRet = DeviceNet_GetMasterIdentity(hChannel, nodeId, &tIdentity);
        if(lRet == CIFX_NO_ERROR && tIdentity.bDataValid)
        {
            printf("Master found at Node %d\r\n", nodeId);
            *pMasterNodeId = nodeId;
            DeviceNet_PrintMasterIdentity(&tIdentity);
            return CIFX_NO_ERROR;
        }
    }

    printf("=== No Master Found on Network ===\r\n");
    return CIFX_DEV_NOT_READY;
}
