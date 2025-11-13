/**************************************************************************************
VAT Explicit Message Handler
Implementation of DeviceNet Explicit Message communication for VAT EDS
**************************************************************************************/

#include "App_VAT_ExplicitHandler.h"
#include "App_VAT_Parameters.h"
#include "App_VAT_Diagnostic.h"
#include "App_VAT_CIP_Objects.h"
#include "App_DemoApplication.h"
#include <string.h>
#include <stdio.h>

/* External global variables */
extern APP_DATA_T g_tAppData;
extern VAT_PARAM_MANAGER_T g_tParamManager;

/******************************************************************************
 * CIP SERVICE HANDLER - DIRECT ENTRY (for APP_DATA_T integration)
 ******************************************************************************/
bool VAT_Explicit_HandleCipService_Direct(
    DNS_PACKET_CIP_SERVICE_IND_T* ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
    APP_DATA_T* ptAppData)
{
    uint32_t ulResDataSize = 0;
    uint32_t ulGRC = CIP_GRC_SUCCESS;
    uint32_t ulERC = 0;

    /* Extract CIP Path */
    uint8_t bClass = (uint8_t)ptInd->tData.ulClass;
    uint8_t bInstance = (uint8_t)ptInd->tData.ulInstance;
    uint8_t bAttribute = (uint8_t)ptInd->tData.ulAttribute;
    uint8_t bService = (uint8_t)ptInd->tData.ulService;

    printf("[VAT] CIP Service: 0x%02X, Class: 0x%02X, Inst: %u, Attr: %u\r\n",
           bService, bClass, bInstance, bAttribute);

    /* Route by Class */
    switch(bClass) {
        case VAT_CLASS_PARAMETER:
            ulGRC = VAT_Parameter_HandleService(NULL, ptInd, ptRes, &ulResDataSize);
            break;

        case VAT_CLASS_DIAGNOSTIC:
            ulGRC = VAT_Diagnostic_HandleService(NULL, ptInd, ptRes, &ulResDataSize);
            break;

        case CIP_CLASS_ASSEMBLY:
            ulGRC = VAT_Assembly_HandleService(NULL, ptInd, ptRes, &ulResDataSize);
            break;

        default:
            printf("[VAT] Unsupported class: 0x%02X\r\n", bClass);
            ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
            break;
    }

    /* Build response packet */
    ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
    ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE + ulResDataSize;
    ptRes->tHead.ulSta = CIFX_NO_ERROR;
    ptRes->tData.ulGRC = ulGRC;
    ptRes->tData.ulERC = ulERC;

    /* Send response */
    int32_t lRet = AppDNS_GlobalPacket_Send(ptAppData);
    if(lRet != CIFX_NO_ERROR) {
        printf("[VAT] ERROR: Failed to send CIP response: 0x%08X\r\n", (unsigned int)lRet);
        return false;
    }

    printf("[VAT] Response sent: GRC=0x%02X, DataSize=%lu\r\n", (unsigned int)ulGRC, ulResDataSize);
    return true;
}

/******************************************************************************
 * CIP SERVICE HANDLER - MAIN ENTRY POINT (Legacy - for APP_DNS_CHANNEL_HANDLER_RSC_T)
 ******************************************************************************/
bool VAT_Explicit_HandleCipService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd)
{
    DNS_PACKET_CIP_SERVICE_RES_T* ptRes =
        (DNS_PACKET_CIP_SERVICE_RES_T*)ptInd;

    uint32_t ulResDataSize = 0;
    uint32_t ulGRC = CIP_GRC_SUCCESS;
    uint32_t ulERC = 0;

    /* Extract CIP Path */
    uint8_t bClass = (uint8_t)ptInd->tData.ulClass;
    uint8_t bInstance = (uint8_t)ptInd->tData.ulInstance;
    uint8_t bAttribute = (uint8_t)ptInd->tData.ulAttribute;
    uint8_t bService = (uint8_t)ptInd->tData.ulService;

    printf("[VAT] CIP Service: 0x%02X, Class: 0x%02X, Inst: %u, Attr: %u\r\n",
           bService, bClass, bInstance, bAttribute);

    /* Route by Class */
    switch(bClass) {
        case VAT_CLASS_PARAMETER:
            ulGRC = VAT_Parameter_HandleService(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
            break;

        case VAT_CLASS_DIAGNOSTIC:
            ulGRC = VAT_Diagnostic_HandleService(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
            break;

        case CIP_CLASS_ASSEMBLY:
            ulGRC = VAT_Assembly_HandleService(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
            break;

        default:
            printf("[VAT] Unsupported class: 0x%02X\r\n", bClass);
            ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
            break;
    }

    /* Build response packet */
    ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
    ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE + ulResDataSize;
    ptRes->tHead.ulSta = CIFX_NO_ERROR;
    ptRes->tData.ulGRC = ulGRC;
    ptRes->tData.ulERC = ulERC;

    /* Send response - Not supported in this mode */
    printf("[VAT] WARNING: Cannot send response without APP_DATA_T\r\n");
    return false;
}

/******************************************************************************
 * VAT PARAMETER OBJECT HANDLER (Class 0x64)
 ******************************************************************************/
uint32_t VAT_Parameter_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize)
{
    uint32_t ulGRC = CIP_GRC_SUCCESS;
    uint8_t bService = (uint8_t)ptInd->tData.ulService;
    uint8_t bInstance = (uint8_t)ptInd->tData.ulInstance;
    uint8_t bAttribute = (uint8_t)ptInd->tData.ulAttribute;

    /* Validate instance range (1-99) */
    if(bInstance == 0 || bInstance > VAT_PARAM_COUNT_MAX) {
        printf("[VAT] Invalid parameter instance: %u\r\n", bInstance);
        return CIP_GRC_OBJECT_DOES_NOT_EXIST;
    }

    /* Service dispatch */
    switch(bService) {
        case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
        {
            /* Read single attribute */
            uint8_t bSize = 0;
            uint8_t abData[32] = {0};

            /* Map attribute to parameter ID */
            uint8_t bParamId = bInstance;

            int32_t lRet = VAT_Param_Get(&g_tParamManager, bParamId, abData, &bSize);

            if(lRet == 0) {
                /* Success - copy data to response */
                memcpy(ptRes->tData.abData, abData, bSize);
                *pulResDataSize = bSize;
                ulGRC = CIP_GRC_SUCCESS;

                printf("[VAT] GET Param%u: ", bInstance);
                for(uint8_t i = 0; i < bSize; i++) {
                    printf("%02X ", abData[i]);
                }
                printf("\r\n");
            } else {
                ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
            }
            break;
        }

        case CIP_SERVICE_SET_ATTRIBUTE_SINGLE:
        {
            /* Write single attribute */
            uint32_t ulDataLength = ptInd->tHead.ulLen - DNS_CIP_SERVICE_IND_SIZE;

            if(ulDataLength == 0) {
                /* No data - reset attribute */
                ulGRC = CIP_GRC_SUCCESS;
            } else {
                /* Map attribute to parameter ID */
                uint8_t bParamId = bInstance;

                int32_t lRet = VAT_Param_Set(&g_tParamManager, bParamId,
                                             ptInd->tData.abData, (uint8_t)ulDataLength);

                if(lRet == 0) {
                    ulGRC = CIP_GRC_SUCCESS;

                    printf("[VAT] SET Param%u: ", bInstance);
                    for(uint32_t i = 0; i < ulDataLength; i++) {
                        printf("%02X ", ptInd->tData.abData[i]);
                    }
                    printf("\r\n");
                } else if(lRet == -1) {
                    ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
                } else if(lRet == -2) {
                    ulGRC = CIP_GRC_TOO_MUCH_DATA;
                } else if(lRet == -3) {
                    ulGRC = CIP_GRC_INVALID_ATTRIBUTE_VALUE;
                } else {
                    ulGRC = CIP_GRC_NOT_ENOUGH_DATA;
                }
            }
            break;
        }

        case CIP_SERVICE_GET_ATTRIBUTES_ALL:
        {
            /* Get all attributes */
            ulGRC = VAT_Parameter_GetAll(bInstance, ptRes, pulResDataSize);
            break;
        }

        case CIP_SERVICE_RESET:
        {
            /* Reset parameter to default */
            ulGRC = VAT_Parameter_Reset(bInstance);
            break;
        }

        default:
            ulGRC = CIP_GRC_SERVICE_NOT_SUPPORTED;
            break;
    }

    return ulGRC;
}

/******************************************************************************
 * VAT ASSEMBLY OBJECT HANDLER (Class 0x04)
 ******************************************************************************/
uint32_t VAT_Assembly_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize)
{
    uint32_t ulGRC = CIP_GRC_SUCCESS;
    uint8_t bService = (uint8_t)ptInd->tData.ulService;
    uint8_t bInstance = (uint8_t)ptInd->tData.ulInstance;

    printf("[VAT] Assembly service not fully implemented: Inst=%u\r\n", bInstance);

    /* For now, return service not supported */
    /* TODO: Implement assembly data read/write if needed */
    ulGRC = CIP_GRC_SERVICE_NOT_SUPPORTED;

    return ulGRC;
}

/******************************************************************************
 * PARAMETER HELPER FUNCTIONS
 ******************************************************************************/
uint32_t VAT_Parameter_GetAll(
    uint8_t bInstance,
    DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
    uint32_t* pulResDataSize)
{
    if(bInstance == 0 || bInstance > VAT_PARAM_COUNT_MAX) {
        return CIP_GRC_OBJECT_DOES_NOT_EXIST;
    }

    VAT_PARAMETER_T* ptParam = &g_tParamManager.params[bInstance - 1];

    /* Copy all parameter data */
    uint8_t* pbDst = ptRes->tData.abData;

    /* Data value */
    memcpy(pbDst, ptParam->data, ptParam->data_size);
    pbDst += ptParam->data_size;

    /* Min value */
    memcpy(pbDst, &ptParam->min_value, sizeof(int32_t));
    pbDst += sizeof(int32_t);

    /* Max value */
    memcpy(pbDst, &ptParam->max_value, sizeof(int32_t));
    pbDst += sizeof(int32_t);

    /* Default value */
    memcpy(pbDst, &ptParam->default_value, sizeof(int32_t));
    pbDst += sizeof(int32_t);

    /* Data type */
    *pbDst++ = ptParam->data_type;

    /* Descriptor */
    memcpy(pbDst, &ptParam->descriptor, sizeof(uint16_t));
    pbDst += sizeof(uint16_t);

    *pulResDataSize = (uint32_t)(pbDst - ptRes->tData.abData);

    printf("[VAT] GET_ALL Param%u: %lu bytes\r\n", bInstance, *pulResDataSize);

    return CIP_GRC_SUCCESS;
}

uint32_t VAT_Parameter_Reset(uint8_t bInstance)
{
    if(bInstance == 0 || bInstance > VAT_PARAM_COUNT_MAX) {
        return CIP_GRC_OBJECT_DOES_NOT_EXIST;
    }

    VAT_PARAMETER_T* ptParam = &g_tParamManager.params[bInstance - 1];

    /* Reset to default value */
    memcpy(ptParam->data, &ptParam->default_value, ptParam->data_size);

    /* Set modified flag */
    VAT_Param_SetModified(&g_tParamManager, bInstance);

    printf("[VAT] Reset Param%u to default: %ld\r\n", bInstance, ptParam->default_value);

    return CIP_GRC_SUCCESS;
}

/******************************************************************************
 * EXPLICIT READ/WRITE HANDLERS (Optional - for future use)
 ******************************************************************************/
bool VAT_Explicit_HandleRead(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    CIFX_PACKET* ptPacket)
{
    printf("[VAT] Explicit Read not implemented\r\n");

    /* Return error response */
    ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
    ptPacket->tHeader.ulCmd |= 0x1;  /* Convert to confirmation */
    AppDNS_GlobalPacket_Send(ptDnsRsc);

    return true;
}

bool VAT_Explicit_HandleWrite(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    CIFX_PACKET* ptPacket)
{
    printf("[VAT] Explicit Write not implemented\r\n");

    /* Return error response */
    ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
    ptPacket->tHeader.ulCmd |= 0x1;  /* Convert to confirmation */
    AppDNS_GlobalPacket_Send(ptDnsRsc);

    return true;
}

/******************************************************************************
 * SYNCHRONIZATION FUNCTIONS
 ******************************************************************************/
void VAT_Sync_ParametersToIoData(void)
{
    /* Sync Param1 (Pressure Setpoint) to Output Assembly */
    int16_t sPressureSetpoint = 0;
    uint8_t bSize = 0;

    if(VAT_Param_Get(&g_tParamManager, 1, &sPressureSetpoint, &bSize) == 0) {
        /* Assuming OUTPUT_ASSEMBLY_8_T structure */
        /* output[1:2] = control_setpoint */
        memcpy(&g_tAppData.tInputData.input[1], &sPressureSetpoint, 2);
    }

    /* Sync Param3 (Controller Mode) to Output Assembly */
    uint8_t bControlMode = 0;
    if(VAT_Param_Get(&g_tParamManager, 3, &bControlMode, &bSize) == 0) {
        /* output[0] = control_mode */
        g_tAppData.tInputData.input[0] = bControlMode;
    }
}

void VAT_Sync_IoDataToParameters(void)
{
    /* Sync Input Assembly to Read-Only Parameters */

    /* Param8 (Current Pressure) from input[1:2] */
    int16_t sPressure = 0;
    memcpy(&sPressure, &g_tAppData.tOutputData.output[1], 2);
    VAT_Param_Set(&g_tParamManager, 8, &sPressure, 2);

    /* Param11 (Current Position) from input[3:4] */
    int16_t sPosition = 0;
    memcpy(&sPosition, &g_tAppData.tOutputData.output[3], 2);
    VAT_Param_Set(&g_tParamManager, 11, &sPosition, 2);

    /* Param5 (Device Status) from input[5] */
    uint8_t bDeviceStatus = g_tAppData.tOutputData.output[5];
    VAT_Param_Set(&g_tParamManager, 5, &bDeviceStatus, 1);

    /* Param6 (Exception Status) from input[0] */
    uint8_t bExceptionStatus = g_tAppData.tOutputData.output[0];
    VAT_Param_Set(&g_tParamManager, 6, &bExceptionStatus, 1);
}
