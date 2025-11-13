#include "App_VAT_Assemblies.h"
#include "App_VAT_Parameters.h"
#include <string.h>

/******************************************************************************
 * ASSEMBLY SIZE LOOKUP TABLES
 ******************************************************************************/

/* Input Assembly Size Map */
typedef struct ASSEMBLY_SIZE_MAP_Ttag {
    uint8_t instance;
    uint8_t size;
    uint16_t io_type_mask;
} ASSEMBLY_SIZE_MAP_T;

static const ASSEMBLY_SIZE_MAP_T g_atInputAssemblySizeMap[] = {
    {1,   2,  IO_TYPE_ALL},
    {2,   3,  IO_TYPE_ALL},
    {3,   5,  IO_TYPE_ALL},
    {4,   5,  IO_TYPE_ALL},
    {5,   7,  IO_TYPE_ALL},
    {6,   8,  IO_TYPE_ALL},
    {10,  1,  IO_TYPE_ALL},
    {11,  6,  IO_TYPE_ALL},
    {17,  4,  IO_TYPE_ALL},
    {18,  5,  IO_TYPE_ALL},
    {19,  9,  IO_TYPE_NO_STROBE},
    {20,  9,  IO_TYPE_NO_STROBE},
    {21,  13, IO_TYPE_NO_STROBE},
    {22,  14, IO_TYPE_NO_STROBE},
    {26,  10, IO_TYPE_NO_STROBE},
    {100, 7,  IO_TYPE_ALL},
    {101, 7,  IO_TYPE_ALL},
    {104, 23, IO_TYPE_NO_STROBE},
    {105, 11, IO_TYPE_NO_STROBE},
    {106, 11, IO_TYPE_NO_STROBE},
    {109, 29, IO_TYPE_NO_STROBE},
    {111, 34, IO_TYPE_NO_STROBE},
    {113, 34, IO_TYPE_NO_STROBE},
    {150, 9,  IO_TYPE_NO_STROBE}
};

static const ASSEMBLY_SIZE_MAP_T g_atOutputAssemblySizeMap[] = {
    {7,   4,  IO_TYPE_NO_STROBE},
    {8,   5,  IO_TYPE_NO_STROBE},
    {23,  6,  IO_TYPE_NO_STROBE},
    {24,  7,  IO_TYPE_NO_STROBE},
    {32,  17, IO_TYPE_NO_STROBE},
    {102, 8,  IO_TYPE_NO_STROBE},
    {103, 6,  IO_TYPE_NO_STROBE},
    {107, 12, IO_TYPE_NO_STROBE},
    {108, 8,  IO_TYPE_NO_STROBE},
    {110, 17, IO_TYPE_NO_STROBE},
    {112, 18, IO_TYPE_NO_STROBE}
};

#define INPUT_ASSEMBLY_COUNT  (sizeof(g_atInputAssemblySizeMap) / sizeof(ASSEMBLY_SIZE_MAP_T))
#define OUTPUT_ASSEMBLY_COUNT (sizeof(g_atOutputAssemblySizeMap) / sizeof(ASSEMBLY_SIZE_MAP_T))

/******************************************************************************
 * ASSEMBLY MANAGER INITIALIZATION
 ******************************************************************************/

void VAT_Assembly_Init(ASSEMBLY_MANAGER_T* ptManager)
{
    if (ptManager == NULL) {
        return;
    }

    /* Clear all buffers */
    memset(ptManager, 0, sizeof(ASSEMBLY_MANAGER_T));

    /* Set default assemblies */
    ptManager->active_input_instance = 100;   /* Default Input Assembly 100 */
    ptManager->active_output_instance = 8;    /* Default Output Assembly 8 */
    ptManager->io_connection_type = IO_TYPE_POLL;
    ptManager->data_type = CIP_DATA_TYPE_INT;  /* Default: INT type */

    /* Initialize metadata for all assemblies */
    uint8_t idx = 0;

    /* Input Assembly metadata */
    for (uint8_t i = 0; i < INPUT_ASSEMBLY_COUNT; i++) {
        ptManager->metadata[idx].instance_number = g_atInputAssemblySizeMap[i].instance;
        ptManager->metadata[idx].size = g_atInputAssemblySizeMap[i].size;
        ptManager->metadata[idx].direction = ASSEMBLY_DIR_INPUT;
        ptManager->metadata[idx].io_type_mask = g_atInputAssemblySizeMap[i].io_type_mask;
        idx++;
    }

    /* Output Assembly metadata */
    for (uint8_t i = 0; i < OUTPUT_ASSEMBLY_COUNT; i++) {
        ptManager->metadata[idx].instance_number = g_atOutputAssemblySizeMap[i].instance;
        ptManager->metadata[idx].size = g_atOutputAssemblySizeMap[i].size;
        ptManager->metadata[idx].direction = ASSEMBLY_DIR_OUTPUT;
        ptManager->metadata[idx].io_type_mask = g_atOutputAssemblySizeMap[i].io_type_mask;
        idx++;
    }
}

/******************************************************************************
 * ASSEMBLY VALIDATION FUNCTIONS
 ******************************************************************************/

uint8_t VAT_Assembly_IsValidInput(uint8_t instance)
{
    for (uint8_t i = 0; i < INPUT_ASSEMBLY_COUNT; i++) {
        if (g_atInputAssemblySizeMap[i].instance == instance) {
            return 1;
        }
    }
    return 0;
}

uint8_t VAT_Assembly_IsValidOutput(uint8_t instance)
{
    for (uint8_t i = 0; i < OUTPUT_ASSEMBLY_COUNT; i++) {
        if (g_atOutputAssemblySizeMap[i].instance == instance) {
            return 1;
        }
    }
    return 0;
}

uint8_t VAT_Assembly_GetInputSize(uint8_t instance)
{
    for (uint8_t i = 0; i < INPUT_ASSEMBLY_COUNT; i++) {
        if (g_atInputAssemblySizeMap[i].instance == instance) {
            return g_atInputAssemblySizeMap[i].size;
        }
    }
    return 0;
}

uint8_t VAT_Assembly_GetOutputSize(uint8_t instance)
{
    for (uint8_t i = 0; i < OUTPUT_ASSEMBLY_COUNT; i++) {
        if (g_atOutputAssemblySizeMap[i].instance == instance) {
            return g_atOutputAssemblySizeMap[i].size;
        }
    }
    return 0;
}

/******************************************************************************
 * ASSEMBLY SELECTION FUNCTIONS
 ******************************************************************************/

int32_t VAT_Assembly_SelectInput(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance)
{
    if (!VAT_Assembly_IsValidInput(instance)) {
        return -1;  /* Invalid instance */
    }

    ptManager->active_input_instance = instance;
    return 0;
}

int32_t VAT_Assembly_SelectOutput(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance)
{
    if (!VAT_Assembly_IsValidOutput(instance)) {
        return -1;  /* Invalid instance */
    }

    ptManager->active_output_instance = instance;
    return 0;
}
