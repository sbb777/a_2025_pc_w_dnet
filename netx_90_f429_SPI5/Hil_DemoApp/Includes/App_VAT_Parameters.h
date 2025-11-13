#ifndef APP_VAT_PARAMETERS_H_
#define APP_VAT_PARAMETERS_H_

#include "Hil_Compiler.h"
#include <stdint.h>

/******************************************************************************
 * CIP PARAMETER DEFINITIONS
 ******************************************************************************/

/* Maximum parameters */
#define VAT_PARAM_COUNT_MAX     99

/* CIP Data Types */
#define CIP_DATA_TYPE_USINT     0xC6  /* Unsigned Short INT (1 byte) */
#define CIP_DATA_TYPE_UINT      0xC7  /* Unsigned INT (2 bytes) */
#define CIP_DATA_TYPE_INT       0xC3  /* Signed INT (2 bytes) */
#define CIP_DATA_TYPE_REAL      0xCA  /* REAL (4 bytes float) */
#define CIP_DATA_TYPE_STRING    0xDA  /* SHORT_STRING */

/* Parameter Descriptor Flags */
#define PARAM_DESC_ENUM         0x0002  /* Enumerated String */
#define PARAM_DESC_READ_ONLY    0x0010  /* Read Only */
#define PARAM_DESC_MONITORING   0x0020  /* Monitoring Attribute */

/* Pressure Units (Param9) */
#define PRESSURE_UNIT_COUNTS    0x1001  /* Raw Counts */
#define PRESSURE_UNIT_PERCENT   0x1007  /* Percent */
#define PRESSURE_UNIT_PSI       0x1300  /* psi */
#define PRESSURE_UNIT_TORR      0x1301  /* Torr */
#define PRESSURE_UNIT_MTORR     0x1302  /* mTorr */
#define PRESSURE_UNIT_BAR       0x1307  /* Bar */
#define PRESSURE_UNIT_MBAR      0x1308  /* mBar */
#define PRESSURE_UNIT_PA        0x1309  /* Pa */
#define PRESSURE_UNIT_ATM       0x130B  /* atm */

/* Position Units (Param10) */
#define POSITION_UNIT_COUNTS    0x1001  /* Raw Counts */
#define POSITION_UNIT_PERCENT   0x1007  /* Percent */
#define POSITION_UNIT_DEGREES   0x1703  /* Degrees */

/* Controller Mode Enum (Param6) */
typedef enum CONTROLLER_MODE_Etag {
    CTRL_MODE_INIT              = 0,
    CTRL_MODE_SYNC              = 1,
    CTRL_MODE_POSITION          = 2,
    CTRL_MODE_CLOSE             = 3,
    CTRL_MODE_OPEN              = 4,
    CTRL_MODE_PRESSURE          = 5,
    CTRL_MODE_HOLD              = 6,
    CTRL_MODE_AUTOLEARNING      = 7,
    CTRL_MODE_OPEN_INTERLOCK    = 8,
    CTRL_MODE_CLOSE_INTERLOCK   = 9,
    CTRL_MODE_MAINT_OPEN        = 10,
    CTRL_MODE_MAINT_CLOSE       = 11,
    CTRL_MODE_POWER_FAIL        = 12,
    CTRL_MODE_FATAL_ERROR       = 14
} CONTROLLER_MODE_E;

/* Device Status Enum (Param5) */
typedef enum DEVICE_STATUS_Etag {
    DEV_STATUS_UNDEFINED        = 0,
    DEV_STATUS_SELF_TESTING     = 1,
    DEV_STATUS_IDLE             = 2,
    DEV_STATUS_SELF_TEST_EXCEPT = 3,
    DEV_STATUS_EXECUTING        = 4,
    DEV_STATUS_ABORT            = 5,
    DEV_STATUS_CRITICAL_FAULT   = 6
} DEVICE_STATUS_E;

/* Access Mode Enum (Param7) */
typedef enum ACCESS_MODE_Etag {
    ACCESS_MODE_LOCAL           = 0,
    ACCESS_MODE_REMOTE          = 1,
    ACCESS_MODE_LOCKED          = 2
} ACCESS_MODE_E;

/* Parameter structure */
typedef struct VAT_PARAMETER_Ttag {
    uint8_t  param_id;          /* Parameter ID (1-99) */
    uint8_t  class_id;          /* CIP Class ID */
    uint8_t  instance_id;       /* CIP Instance ID */
    uint8_t  attribute_id;      /* CIP Attribute ID */

    uint16_t descriptor;        /* Parameter descriptor */
    uint8_t  data_type;         /* CIP Data Type */
    uint8_t  data_size;         /* Data size in bytes */

    char name[64];              /* Parameter name */
    char units[16];             /* Units */
    char help[128];             /* Help text */

    int32_t min_value;          /* Minimum value */
    int32_t max_value;          /* Maximum value */
    int32_t default_value;      /* Default value */

    uint8_t data[32];           /* Current value (max 32 bytes for strings) */
} VAT_PARAMETER_T;

/* Parameter manager */
typedef struct VAT_PARAM_MANAGER_Ttag {
    VAT_PARAMETER_T params[VAT_PARAM_COUNT_MAX];
    uint8_t param_count;
    uint8_t modified[13];       /* Modified flags (99/8 = 13 bytes) */
} VAT_PARAM_MANAGER_T;

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/* Parameter Manager Initialization */
void VAT_Param_Init(VAT_PARAM_MANAGER_T* ptManager);

/* Parameter Access by ID */
int32_t VAT_Param_Get(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, void* pData, uint8_t* pSize);
int32_t VAT_Param_Set(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, const void* pData, uint8_t size);

/* Parameter Access by CIP Path */
int32_t VAT_Param_GetByPath(VAT_PARAM_MANAGER_T* ptManager, uint8_t class_id, uint8_t instance_id, uint8_t attribute_id, void* pData, uint8_t* pSize);
int32_t VAT_Param_SetByPath(VAT_PARAM_MANAGER_T* ptManager, uint8_t class_id, uint8_t instance_id, uint8_t attribute_id, const void* pData, uint8_t size);

/* Modified Flags */
uint8_t VAT_Param_IsModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id);
void VAT_Param_ClearModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id);
void VAT_Param_SetModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id);

/* Flash Storage */
int32_t VAT_Param_SaveToFlash(VAT_PARAM_MANAGER_T* ptManager);
int32_t VAT_Param_LoadFromFlash(VAT_PARAM_MANAGER_T* ptManager);

/******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Global Parameter Manager instance */
extern VAT_PARAM_MANAGER_T g_tParamManager;

#endif /* APP_VAT_PARAMETERS_H_ */
