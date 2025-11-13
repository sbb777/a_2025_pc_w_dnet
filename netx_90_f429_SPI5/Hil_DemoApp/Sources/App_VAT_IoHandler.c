/**************************************************************************************
 * VAT I/O Assembly Handler
 * Handles Input/Output Assembly data processing
 **************************************************************************************/

#include "App_VAT_IoHandler.h"
#include "App_VAT_Parameters.h"
#include "App_VAT_Conversion.h"
#include <string.h>

/* External references - these should be provided by the main application */
extern int16_t g_current_pressure;      /* Current pressure reading */
extern int16_t g_current_position;      /* Current valve position */
extern uint8_t g_device_status;         /* Device status */
extern uint8_t g_access_mode;           /* Access mode */
extern uint8_t g_exception_status;      /* Exception status */
extern uint8_t g_discrete_inputs;       /* Discrete input states */

extern int16_t g_control_setpoint;      /* Control setpoint */
extern uint8_t g_control_mode;          /* Control mode */
extern uint8_t g_control_instance;      /* Control instance */

/******************************************************************************
 * INPUT ASSEMBLY UPDATE FUNCTIONS (Slave → Master)
 ******************************************************************************/

void VAT_UpdateInputAssembly100(INPUT_ASSEMBLY_100_T* ptInput)
{
    ptInput->exception_status = g_exception_status;
    ptInput->pressure = g_current_pressure;
    ptInput->position = g_current_position;
    ptInput->device_status = g_device_status;
    ptInput->access_mode = g_access_mode;
}

void VAT_UpdateInputAssembly101(INPUT_ASSEMBLY_101_T* ptInput)
{
    ptInput->exception_status = g_exception_status;
    ptInput->pressure = g_current_pressure;
    ptInput->position = g_current_position;
    ptInput->discrete_inputs = g_discrete_inputs;
    ptInput->device_status = g_device_status;
}

void VAT_UpdateInputAssembly105(INPUT_ASSEMBLY_105_T* ptInput)
{
    /* Convert INT to REAL (float) */
    ptInput->exception_status = g_exception_status;
    ptInput->fp_pressure = VAT_ConvertPressureIntToReal(g_current_pressure, PRESSURE_UNIT_TORR, 1000.0f);
    ptInput->fp_position = VAT_ConvertPositionIntToReal(g_current_position, POSITION_UNIT_PERCENT);
    ptInput->device_status = g_device_status;
    ptInput->access_mode = g_access_mode;
}

/******************************************************************************
 * OUTPUT ASSEMBLY PROCESS FUNCTIONS (Master → Slave)
 ******************************************************************************/

void VAT_ProcessOutputAssembly8(OUTPUT_ASSEMBLY_8_T* ptOutput)
{
    g_control_mode = ptOutput->control_mode;
    g_control_setpoint = ptOutput->control_setpoint;
    g_control_instance = ptOutput->control_instance;
}

void VAT_ProcessOutputAssembly24(OUTPUT_ASSEMBLY_24_T* ptOutput)
{
    /* Convert REAL to INT */
    g_control_mode = ptOutput->control_mode;
    g_control_setpoint = VAT_ConvertPressureRealToInt(ptOutput->fp_control_setpoint, PRESSURE_UNIT_TORR, 1000.0f);
    g_control_instance = ptOutput->control_instance;
}

void VAT_ProcessOutputAssembly102(OUTPUT_ASSEMBLY_102_T* ptOutput)
{
    g_control_mode = ptOutput->control_mode;
    g_control_setpoint = ptOutput->control_setpoint;
    g_control_instance = ptOutput->control_instance;

    /* Additional parameters for Output Assembly 102 */
    /* Auto learn, calibration scale, zero adjust would be processed here */
}

/******************************************************************************
 * GENERIC I/O ASSEMBLY HANDLERS
 ******************************************************************************/

void VAT_UpdateInputAssembly(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance)
{
    /* Find the assembly buffer index */
    uint8_t buffer_idx = 0;

    /* Get pointer to the appropriate buffer */
    void* pBuffer = NULL;

    for (uint8_t i = 0; i < 24; i++) {
        if (ptManager->metadata[i].instance_number == instance) {
            pBuffer = &ptManager->input_buffers[i].data;
            buffer_idx = i;
            break;
        }
    }

    if (pBuffer == NULL) {
        return; /* Invalid instance */
    }

    /* Update based on instance number */
    switch (instance) {
        case 1:
            ((INPUT_ASSEMBLY_1_T*)pBuffer)->process_variable = g_current_pressure;
            break;

        case 2:
            {
                INPUT_ASSEMBLY_2_T* p = (INPUT_ASSEMBLY_2_T*)pBuffer;
                p->exception_status = g_exception_status;
                p->pressure = g_current_pressure;
            }
            break;

        case 3:
            {
                INPUT_ASSEMBLY_3_T* p = (INPUT_ASSEMBLY_3_T*)pBuffer;
                p->exception_status = g_exception_status;
                p->pressure = g_current_pressure;
                p->position = g_current_position;
            }
            break;

        case 100:
            VAT_UpdateInputAssembly100((INPUT_ASSEMBLY_100_T*)pBuffer);
            break;

        case 101:
            VAT_UpdateInputAssembly101((INPUT_ASSEMBLY_101_T*)pBuffer);
            break;

        case 105:
            VAT_UpdateInputAssembly105((INPUT_ASSEMBLY_105_T*)pBuffer);
            break;

        /* Add other assemblies as needed */

        default:
            /* Unknown assembly - clear buffer */
            memset(pBuffer, 0, ptManager->metadata[buffer_idx].size);
            break;
    }

    /* Update valid size */
    ptManager->input_buffers[buffer_idx].valid_size = VAT_Assembly_GetInputSize(instance);
}

void VAT_ProcessOutputAssembly(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance)
{
    /* Find the assembly buffer index */
    uint8_t buffer_idx = 0;

    /* Get pointer to the appropriate buffer */
    void* pBuffer = NULL;

    for (uint8_t i = 0; i < 11; i++) {
        if (ptManager->metadata[24 + i].instance_number == instance) {
            pBuffer = &ptManager->output_buffers[i].data;
            buffer_idx = i;
            break;
        }
    }

    if (pBuffer == NULL) {
        return; /* Invalid instance */
    }

    /* Process based on instance number */
    switch (instance) {
        case 7:
            {
                OUTPUT_ASSEMBLY_7_T* p = (OUTPUT_ASSEMBLY_7_T*)pBuffer;
                g_control_setpoint = p->control_setpoint;
                g_control_instance = p->control_instance;
            }
            break;

        case 8:
            VAT_ProcessOutputAssembly8((OUTPUT_ASSEMBLY_8_T*)pBuffer);
            break;

        case 24:
            VAT_ProcessOutputAssembly24((OUTPUT_ASSEMBLY_24_T*)pBuffer);
            break;

        case 102:
            VAT_ProcessOutputAssembly102((OUTPUT_ASSEMBLY_102_T*)pBuffer);
            break;

        /* Add other assemblies as needed */

        default:
            /* Unknown assembly */
            break;
    }
}

/******************************************************************************
 * I/O CONNECTION TYPE HANDLERS
 ******************************************************************************/

void VAT_HandleCyclicIo(ASSEMBLY_MANAGER_T* ptManager)
{
    /* Cyclic I/O: Automatically update at regular intervals */

    /* Update Input Assembly (Slave → Master) */
    VAT_UpdateInputAssembly(ptManager, ptManager->active_input_instance);

    /* Process Output Assembly (Master → Slave) */
    VAT_ProcessOutputAssembly(ptManager, ptManager->active_output_instance);
}

void VAT_HandlePollIo(ASSEMBLY_MANAGER_T* ptManager)
{
    /* Poll I/O: Update only when polled by master */

    /* Same as cyclic, but triggered by poll request */
    VAT_UpdateInputAssembly(ptManager, ptManager->active_input_instance);
    VAT_ProcessOutputAssembly(ptManager, ptManager->active_output_instance);
}

void VAT_HandleCosIo(ASSEMBLY_MANAGER_T* ptManager)
{
    /* Change of State I/O: Update only when data changes */

    /* This would require comparing current vs previous values */
    /* For now, implement same as cyclic */
    VAT_UpdateInputAssembly(ptManager, ptManager->active_input_instance);
    VAT_ProcessOutputAssembly(ptManager, ptManager->active_output_instance);
}
