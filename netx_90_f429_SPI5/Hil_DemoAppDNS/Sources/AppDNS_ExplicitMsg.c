/**************************************************************************************
 * VAT DeviceNet CIP Explicit Messaging Handler
 * Handles Get/Set Attribute Single and other CIP services
 **************************************************************************************/

#include "App_VAT_Parameters.h"
#include <string.h>

/******************************************************************************
 * CIP SERVICE CODES
 ******************************************************************************/

#define CIP_SERVICE_GET_ATTRIBUTE_SINGLE    0x0E
#define CIP_SERVICE_SET_ATTRIBUTE_SINGLE    0x10
#define CIP_SERVICE_GET_ATTRIBUTE_ALL       0x01
#define CIP_SERVICE_RESET                   0x05
#define CIP_SERVICE_SAVE                    0x16

/******************************************************************************
 * CIP ERROR CODES
 ******************************************************************************/

#define CIP_ERROR_SUCCESS                   0x00
#define CIP_ERROR_PATH_SEGMENT_ERROR        0x04
#define CIP_ERROR_INVALID_ATTRIBUTE         0x09
#define CIP_ERROR_ATTRIBUTE_NOT_SETTABLE    0x0E
#define CIP_ERROR_OBJECT_STATE_CONFLICT     0x10
#define CIP_ERROR_INVALID_PARAMETER         0x15

/******************************************************************************
 * HELPER FUNCTION: Find Parameter by CIP Path
 ******************************************************************************/

static VAT_PARAMETER_T* VAT_Param_FindByPath(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id)
{
    for (uint8_t i = 0; i < g_tParamManager.param_count; i++) {
        VAT_PARAMETER_T* pParam = &g_tParamManager.params[i];
        if (pParam->class_id == class_id &&
            pParam->instance_id == instance_id &&
            pParam->attribute_id == attribute_id) {
            return pParam;
        }
    }
    return NULL;
}

/******************************************************************************
 * GET ATTRIBUTE SINGLE
 ******************************************************************************/

uint8_t CIP_HandleGetAttributeSingle(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id,
                                      uint8_t* pResponse, uint8_t* pResponseSize)
{
    /* Find parameter by CIP path */
    VAT_PARAMETER_T* pParam = VAT_Param_FindByPath(class_id, instance_id, attribute_id);

    if (!pParam) {
        return CIP_ERROR_INVALID_ATTRIBUTE;
    }

    /* Copy parameter data to response */
    memcpy(pResponse, pParam->data, pParam->data_size);
    *pResponseSize = pParam->data_size;

    return CIP_ERROR_SUCCESS;
}

/******************************************************************************
 * SET ATTRIBUTE SINGLE
 ******************************************************************************/

uint8_t CIP_HandleSetAttributeSingle(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id,
                                      const uint8_t* pData, uint8_t dataSize)
{
    /* Find parameter by CIP path */
    VAT_PARAMETER_T* pParam = VAT_Param_FindByPath(class_id, instance_id, attribute_id);

    if (!pParam) {
        return CIP_ERROR_INVALID_ATTRIBUTE;
    }

    /* Check if settable */
    if (pParam->descriptor & PARAM_DESC_READ_ONLY) {
        return CIP_ERROR_ATTRIBUTE_NOT_SETTABLE;
    }

    /* Validate size */
    if (dataSize != pParam->data_size) {
        return CIP_ERROR_INVALID_PARAMETER;
    }

    /* Validate range for numeric types */
    if (pParam->data_type == CIP_DATA_TYPE_USINT ||
        pParam->data_type == CIP_DATA_TYPE_UINT ||
        pParam->data_type == CIP_DATA_TYPE_INT) {

        int32_t value = 0;
        switch (pParam->data_type) {
            case CIP_DATA_TYPE_USINT:
                value = *(uint8_t*)pData;
                break;
            case CIP_DATA_TYPE_UINT:
                value = *(uint16_t*)pData;
                break;
            case CIP_DATA_TYPE_INT:
                value = *(int16_t*)pData;
                break;
        }

        if (value < pParam->min_value || value > pParam->max_value) {
            return CIP_ERROR_INVALID_PARAMETER;
        }
    }

    /* Set parameter value */
    memcpy(pParam->data, pData, dataSize);

    /* Mark as modified */
    VAT_Param_SetModified(&g_tParamManager, pParam->param_id);

    return CIP_ERROR_SUCCESS;
}

/******************************************************************************
 * GET ATTRIBUTE ALL
 ******************************************************************************/

uint8_t CIP_HandleGetAttributeAll(uint8_t class_id, uint8_t instance_id,
                                   uint8_t* pResponse, uint16_t* pResponseSize)
{
    uint16_t offset = 0;

    /* Find all parameters for this class/instance */
    for (uint8_t i = 0; i < g_tParamManager.param_count; i++) {
        VAT_PARAMETER_T* pParam = &g_tParamManager.params[i];

        if (pParam->class_id == class_id && pParam->instance_id == instance_id) {
            /* Copy parameter data */
            memcpy(&pResponse[offset], pParam->data, pParam->data_size);
            offset += pParam->data_size;
        }
    }

    *pResponseSize = offset;
    return (offset > 0) ? CIP_ERROR_SUCCESS : CIP_ERROR_INVALID_ATTRIBUTE;
}

/******************************************************************************
 * SAVE SERVICE (Save parameters to Flash)
 ******************************************************************************/

uint8_t CIP_HandleSave(void)
{
    int32_t result = VAT_Param_SaveToFlash(&g_tParamManager);

    if (result == 0) {
        return CIP_ERROR_SUCCESS;
    } else {
        return CIP_ERROR_OBJECT_STATE_CONFLICT;
    }
}

/******************************************************************************
 * RESET SERVICE (Load parameters from Flash)
 ******************************************************************************/

uint8_t CIP_HandleReset(void)
{
    int32_t result = VAT_Param_LoadFromFlash(&g_tParamManager);

    if (result == 0) {
        return CIP_ERROR_SUCCESS;
    } else {
        return CIP_ERROR_OBJECT_STATE_CONFLICT;
    }
}

/******************************************************************************
 * MAIN EXPLICIT MESSAGE HANDLER
 ******************************************************************************/

uint8_t CIP_HandleExplicitMessage(uint8_t service_code, uint8_t class_id, uint8_t instance_id,
                                   uint8_t attribute_id, const uint8_t* pRequestData, uint8_t requestSize,
                                   uint8_t* pResponseData, uint16_t* pResponseSize)
{
    uint8_t error_code = CIP_ERROR_SUCCESS;

    switch (service_code) {
        case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
            {
                uint8_t size = 0;
                error_code = CIP_HandleGetAttributeSingle(class_id, instance_id, attribute_id,
                                                          pResponseData, &size);
                *pResponseSize = size;
            }
            break;

        case CIP_SERVICE_SET_ATTRIBUTE_SINGLE:
            error_code = CIP_HandleSetAttributeSingle(class_id, instance_id, attribute_id,
                                                      pRequestData, requestSize);
            *pResponseSize = 0;
            break;

        case CIP_SERVICE_GET_ATTRIBUTE_ALL:
            error_code = CIP_HandleGetAttributeAll(class_id, instance_id,
                                                   pResponseData, pResponseSize);
            break;

        case CIP_SERVICE_SAVE:
            error_code = CIP_HandleSave();
            *pResponseSize = 0;
            break;

        case CIP_SERVICE_RESET:
            error_code = CIP_HandleReset();
            *pResponseSize = 0;
            break;

        default:
            error_code = CIP_ERROR_PATH_SEGMENT_ERROR;
            *pResponseSize = 0;
            break;
    }

    return error_code;
}

/******************************************************************************
 * CIP MESSAGE PROCESSOR (Parses raw CIP message and generates response)
 ******************************************************************************/

uint8_t CIP_ProcessExplicitMessage(const uint8_t* pRequest, uint32_t reqLen,
                                    uint8_t* pResponse, uint32_t* pRespLen)
{
    /* Minimum request: Service (1) + Path Size (1) = 2 bytes */
    if (reqLen < 2) {
        /* Invalid message format */
        pResponse[0] = 0x80;  /* Generic error reply */
        pResponse[1] = 0x00;
        pResponse[2] = CIP_ERROR_PATH_SEGMENT_ERROR;
        pResponse[3] = 0x00;
        *pRespLen = 4;
        return CIP_ERROR_PATH_SEGMENT_ERROR;
    }

    uint8_t service = pRequest[0];
    uint8_t path_size = pRequest[1];  /* Path size in words (2 bytes each) */

    /* Validate path size */
    uint32_t path_bytes = path_size * 2;
    if (reqLen < (2 + path_bytes)) {
        /* Path size exceeds message length */
        pResponse[0] = service | 0x80;
        pResponse[1] = 0x00;
        pResponse[2] = CIP_ERROR_PATH_SEGMENT_ERROR;
        pResponse[3] = 0x00;
        *pRespLen = 4;
        return CIP_ERROR_PATH_SEGMENT_ERROR;
    }

    /* Parse CIP Path (Logical Segments) */
    /* Format: 20 <class> 24 <instance> 30 <attribute> */
    uint8_t class_id = 0;
    uint8_t instance_id = 0;
    uint8_t attribute_id = 0;

    uint32_t path_offset = 2;  /* Start after Service and Path Size */

    /* Parse path segments */
    for (uint32_t i = 0; i < path_size; i++) {
        uint8_t segment_type = pRequest[path_offset];
        uint8_t segment_value = pRequest[path_offset + 1];

        if (segment_type == 0x20) {
            /* Logical Class ID (8-bit) */
            class_id = segment_value;
        } else if (segment_type == 0x24) {
            /* Logical Instance ID (8-bit) */
            instance_id = segment_value;
        } else if (segment_type == 0x30) {
            /* Logical Attribute ID (8-bit) */
            attribute_id = segment_value;
        }

        path_offset += 2;  /* Each segment is 2 bytes */
    }

    /* Data starts after path */
    const uint8_t* pData = (reqLen > path_offset) ? &pRequest[path_offset] : NULL;
    uint32_t dataLen = (reqLen > path_offset) ? (reqLen - path_offset) : 0;

    /* Process service */
    uint8_t error_code = CIP_ERROR_SUCCESS;
    uint8_t data_size = 0;

    switch (service) {
        case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
            /* Get Attribute Single */
            error_code = CIP_HandleGetAttributeSingle(class_id, instance_id, attribute_id,
                                                       pResponse + 4, &data_size);

            /* Build response header */
            pResponse[0] = 0x8E;  /* Reply service (0x0E | 0x80) */
            pResponse[1] = 0x00;  /* Reserved */
            pResponse[2] = error_code;  /* General Status */
            pResponse[3] = 0x00;  /* Additional Status Size */

            *pRespLen = 4 + data_size;
            break;

        case CIP_SERVICE_SET_ATTRIBUTE_SINGLE:
            /* Set Attribute Single */
            error_code = CIP_HandleSetAttributeSingle(class_id, instance_id, attribute_id,
                                                       pData, (uint8_t)dataLen);

            /* Build response header */
            pResponse[0] = 0x90;  /* Reply service (0x10 | 0x80) */
            pResponse[1] = 0x00;  /* Reserved */
            pResponse[2] = error_code;  /* General Status */
            pResponse[3] = 0x00;  /* Additional Status Size */

            *pRespLen = 4;
            break;

        case CIP_SERVICE_GET_ATTRIBUTE_ALL:
            /* Get Attribute All */
            {
                uint16_t size = 0;
                error_code = CIP_HandleGetAttributeAll(class_id, instance_id,
                                                       pResponse + 4, &size);

                /* Build response header */
                pResponse[0] = 0x81;  /* Reply service (0x01 | 0x80) */
                pResponse[1] = 0x00;
                pResponse[2] = error_code;
                pResponse[3] = 0x00;

                *pRespLen = 4 + size;
            }
            break;

        case CIP_SERVICE_SAVE:
            /* Save to Flash */
            error_code = CIP_HandleSave();

            pResponse[0] = 0x96;  /* Reply service (0x16 | 0x80) */
            pResponse[1] = 0x00;
            pResponse[2] = error_code;
            pResponse[3] = 0x00;

            *pRespLen = 4;
            break;

        case CIP_SERVICE_RESET:
            /* Reset from Flash */
            error_code = CIP_HandleReset();

            pResponse[0] = 0x85;  /* Reply service (0x05 | 0x80) */
            pResponse[1] = 0x00;
            pResponse[2] = error_code;
            pResponse[3] = 0x00;

            *pRespLen = 4;
            break;

        default:
            /* Unsupported service */
            error_code = CIP_ERROR_PATH_SEGMENT_ERROR;

            pResponse[0] = service | 0x80;
            pResponse[1] = 0x00;
            pResponse[2] = error_code;
            pResponse[3] = 0x00;

            *pRespLen = 4;
            break;
    }

    return error_code;
}
