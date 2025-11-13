#include "App_VAT_Parameters.h"
#include <string.h>

/* Global Parameter Manager */
VAT_PARAM_MANAGER_T g_tParamManager;

void VAT_Param_Init(VAT_PARAM_MANAGER_T* ptManager)
{
    memset(ptManager, 0, sizeof(VAT_PARAM_MANAGER_T));

    /**************************************************************************
     * VAT VALVE PARAMETERS - Explicit Message Access
     **************************************************************************/

    /* Param1: Pressure Setpoint (Writable) */
    ptManager->params[0].param_id = 1;
    ptManager->params[0].descriptor = PARAM_DESC_ENUM;  /* Writable */
    ptManager->params[0].data_type = CIP_DATA_TYPE_INT;
    ptManager->params[0].data_size = 2;
    strcpy(ptManager->params[0].name, "Pressure Setpoint");
    strcpy(ptManager->params[0].help, "Target pressure value (0.01 mbar units)");
    ptManager->params[0].min_value = 0;
    ptManager->params[0].max_value = 10000;  /* 0-100.00 mbar */
    ptManager->params[0].default_value = 5000;  /* 50.00 mbar */
    *((int16_t*)ptManager->params[0].data) = 5000;

    /* Param2: Position Setpoint (Writable) */
    ptManager->params[1].param_id = 2;
    ptManager->params[1].descriptor = PARAM_DESC_ENUM;
    ptManager->params[1].data_type = CIP_DATA_TYPE_INT;
    ptManager->params[1].data_size = 2;
    strcpy(ptManager->params[1].name, "Position Setpoint");
    strcpy(ptManager->params[1].help, "Target valve position (0.01% units)");
    ptManager->params[1].min_value = 0;
    ptManager->params[1].max_value = 10000;  /* 0-100.00% */
    ptManager->params[1].default_value = 5000;  /* 50.00% */
    *((int16_t*)ptManager->params[1].data) = 5000;

    /* Param3: Control Mode (Writable) */
    ptManager->params[2].param_id = 3;
    ptManager->params[2].descriptor = PARAM_DESC_ENUM;
    ptManager->params[2].data_type = CIP_DATA_TYPE_USINT;
    ptManager->params[2].data_size = 1;
    strcpy(ptManager->params[2].name, "Control Mode");
    strcpy(ptManager->params[2].help, "0=Pressure, 1=Position, 2=Open, 3=Close, 4=Hold, 5=Learn");
    ptManager->params[2].min_value = 0;
    ptManager->params[2].max_value = 5;
    ptManager->params[2].default_value = 0;  /* Pressure control */
    ptManager->params[2].data[0] = 0;

    /* Param4: Actual Pressure (Read-Only, updated from I/O) */
    ptManager->params[3].param_id = 4;
    ptManager->params[3].descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_MONITORING;
    ptManager->params[3].data_type = CIP_DATA_TYPE_INT;
    ptManager->params[3].data_size = 2;
    strcpy(ptManager->params[3].name, "Actual Pressure");
    strcpy(ptManager->params[3].help, "Current pressure reading (0.01 mbar)");
    ptManager->params[3].min_value = 0;
    ptManager->params[3].max_value = 10000;
    ptManager->params[3].default_value = 0;
    *((int16_t*)ptManager->params[3].data) = 0;

    /* Param5: Actual Position (Read-Only, updated from I/O) */
    ptManager->params[4].param_id = 5;
    ptManager->params[4].descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_MONITORING;
    ptManager->params[4].data_type = CIP_DATA_TYPE_INT;
    ptManager->params[4].data_size = 2;
    strcpy(ptManager->params[4].name, "Actual Position");
    strcpy(ptManager->params[4].help, "Current valve position (0.01%)");
    ptManager->params[4].min_value = 0;
    ptManager->params[4].max_value = 10000;
    ptManager->params[4].default_value = 0;
    *((int16_t*)ptManager->params[4].data) = 0;

    /* Param6: Device Status (Read-Only) */
    ptManager->params[5].param_id = 6;
    ptManager->params[5].descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_ENUM | PARAM_DESC_MONITORING;
    ptManager->params[5].data_type = CIP_DATA_TYPE_USINT;
    ptManager->params[5].data_size = 1;
    strcpy(ptManager->params[5].name, "Device Status");
    strcpy(ptManager->params[5].help, "0=Init, 1=Idle, 2=Controlling, 3=Error, 4=Learn");
    ptManager->params[5].min_value = 0;
    ptManager->params[5].max_value = 4;
    ptManager->params[5].default_value = 1;  /* Idle */
    ptManager->params[5].data[0] = 1;

    /* Param7: Exception Status (Read-Only) */
    ptManager->params[6].param_id = 7;
    ptManager->params[6].descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_MONITORING;
    ptManager->params[6].data_type = CIP_DATA_TYPE_USINT;
    ptManager->params[6].data_size = 1;
    strcpy(ptManager->params[6].name, "Exception Status");
    strcpy(ptManager->params[6].help, "Bit0=Pressure error, Bit1=Position error, Bit2=Comm error");
    ptManager->params[6].min_value = 0;
    ptManager->params[6].max_value = 255;
    ptManager->params[6].default_value = 0;
    ptManager->params[6].data[0] = 0;

    /* Param8: Access Mode (Read-Only) */
    ptManager->params[7].param_id = 8;
    ptManager->params[7].descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_ENUM;
    ptManager->params[7].data_type = CIP_DATA_TYPE_USINT;
    ptManager->params[7].data_size = 1;
    strcpy(ptManager->params[7].name, "Access Mode");
    strcpy(ptManager->params[7].help, "0=Local, 1=Remote");
    ptManager->params[7].min_value = 0;
    ptManager->params[7].max_value = 1;
    ptManager->params[7].default_value = 1;  /* Remote */
    ptManager->params[7].data[0] = 1;

    /* Param9: Pressure Upper Limit (Writable) */
    ptManager->params[8].param_id = 9;
    ptManager->params[8].descriptor = PARAM_DESC_ENUM;
    ptManager->params[8].data_type = CIP_DATA_TYPE_INT;
    ptManager->params[8].data_size = 2;
    strcpy(ptManager->params[8].name, "Pressure Upper Limit");
    strcpy(ptManager->params[8].help, "Maximum allowed pressure (0.01 mbar)");
    ptManager->params[8].min_value = 0;
    ptManager->params[8].max_value = 10000;
    ptManager->params[8].default_value = 10000;
    *((int16_t*)ptManager->params[8].data) = 10000;

    /* Param10: Pressure Lower Limit (Writable) */
    ptManager->params[9].param_id = 10;
    ptManager->params[9].descriptor = PARAM_DESC_ENUM;
    ptManager->params[9].data_type = CIP_DATA_TYPE_INT;
    ptManager->params[9].data_size = 2;
    strcpy(ptManager->params[9].name, "Pressure Lower Limit");
    strcpy(ptManager->params[9].help, "Minimum allowed pressure (0.01 mbar)");
    ptManager->params[9].min_value = 0;
    ptManager->params[9].max_value = 10000;
    ptManager->params[9].default_value = 0;
    *((int16_t*)ptManager->params[9].data) = 0;

    /* Additional parameters 11-99 can be added as needed */

    ptManager->param_count = 99;
}

int32_t VAT_Param_Get(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, void* pData, uint8_t* pSize)
{
    if (param_id < 1 || param_id > 99) {
        return -1;
    }

    VAT_PARAMETER_T* pParam = &ptManager->params[param_id - 1];

    if (pSize) {
        *pSize = pParam->data_size;
    }

    if (pData) {
        memcpy(pData, pParam->data, pParam->data_size);
    }

    return 0;
}

int32_t VAT_Param_Set(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, const void* pData, uint8_t size)
{
    if (param_id < 1 || param_id > 99) {
        return -1;
    }

    VAT_PARAMETER_T* pParam = &ptManager->params[param_id - 1];

    /* Check Read-Only */
    if (pParam->descriptor & PARAM_DESC_READ_ONLY) {
        return -2;  /* Read-only parameter */
    }

    /* Validate size */
    if (size != pParam->data_size) {
        return -3;  /* Invalid size */
    }

    /* Validate range (for numeric types) */
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
            return -4;  /* Out of range */
        }
    }

    /* Set data */
    memcpy(pParam->data, pData, size);

    /* Mark as modified */
    VAT_Param_SetModified(ptManager, param_id);

    return 0;
}

int32_t VAT_Param_GetByPath(VAT_PARAM_MANAGER_T* ptManager, uint8_t class_id, uint8_t instance_id, uint8_t attribute_id, void* pData, uint8_t* pSize)
{
    /* Find parameter by CIP path */
    for (uint8_t i = 0; i < ptManager->param_count; i++) {
        VAT_PARAMETER_T* pParam = &ptManager->params[i];
        if (pParam->class_id == class_id &&
            pParam->instance_id == instance_id &&
            pParam->attribute_id == attribute_id) {

            if (pSize) {
                *pSize = pParam->data_size;
            }
            if (pData) {
                memcpy(pData, pParam->data, pParam->data_size);
            }
            return 0;
        }
    }
    return -1;  /* Not found */
}

int32_t VAT_Param_SetByPath(VAT_PARAM_MANAGER_T* ptManager, uint8_t class_id, uint8_t instance_id, uint8_t attribute_id, const void* pData, uint8_t size)
{
    /* Find parameter by CIP path */
    for (uint8_t i = 0; i < ptManager->param_count; i++) {
        VAT_PARAMETER_T* pParam = &ptManager->params[i];
        if (pParam->class_id == class_id &&
            pParam->instance_id == instance_id &&
            pParam->attribute_id == attribute_id) {

            return VAT_Param_Set(ptManager, pParam->param_id, pData, size);
        }
    }
    return -1;  /* Not found */
}

uint8_t VAT_Param_IsModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id)
{
    if (param_id < 1 || param_id > 99) {
        return 0;
    }

    uint8_t byte_index = (param_id - 1) / 8;
    uint8_t bit_index = (param_id - 1) % 8;

    return (ptManager->modified[byte_index] & (1 << bit_index)) ? 1 : 0;
}

void VAT_Param_ClearModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id)
{
    if (param_id < 1 || param_id > 99) {
        return;
    }

    uint8_t byte_index = (param_id - 1) / 8;
    uint8_t bit_index = (param_id - 1) % 8;

    ptManager->modified[byte_index] &= ~(1 << bit_index);
}

void VAT_Param_SetModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id)
{
    if (param_id < 1 || param_id > 99) {
        return;
    }

    uint8_t byte_index = (param_id - 1) / 8;
    uint8_t bit_index = (param_id - 1) % 8;

    ptManager->modified[byte_index] |= (1 << bit_index);
}

/* Flash storage functions are implemented in App_VAT_Flash.c */
