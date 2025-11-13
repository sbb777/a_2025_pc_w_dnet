/**************************************************************************************
 * VAT Adaptive Pressure Controller DeviceNet Communication Test
 *
 * Implementation of test functions for VAT pressure controller
 **************************************************************************************/

#include "vat_devicenet_test.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Include cifX API headers if available */
#ifdef USE_CIFX_API
#include "cifXUser.h"
#include "cifXErrors.h"
#else
/* Mock definitions for standalone compilation */
#define CIFX_NO_ERROR           0
#define CIFX_INVALID_HANDLE     -1
#define CIFX_BUFFER_TOO_SHORT   -2
typedef void* CIFXHANDLE;

/* Mock cifX functions */
static int32_t xChannelIORead(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset,
                              uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
    /* Simulate successful read with dummy data */
    memset(pvData, 0x00, ulDataLen);
    return CIFX_NO_ERROR;
}

static int32_t xChannelIOWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset,
                               uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
    /* Simulate successful write */
    return CIFX_NO_ERROR;
}
#endif

/* Printf wrapper for logging */
#ifdef ENABLE_LOGGING
#define LOG_PRINTF(...)     printf(__VA_ARGS__)
#else
#define LOG_PRINTF(...)     do {} while(0)
#endif

/*****************************************************************************/
/*! Get current timestamp in milliseconds                                    */
/*****************************************************************************/
static uint32_t VAT_GetTimestampMs(void)
{
    return (uint32_t)(clock() * 1000 / CLOCKS_PER_SEC);
}

/*****************************************************************************/
/*! Initialize test context
 *
 * \param ptContext  [in/out]  Test context to initialize
 */
/*****************************************************************************/
void VAT_Test_Init(VAT_TEST_CONTEXT_T* ptContext)
{
    if (ptContext == NULL)
        return;

    /* Clear all data */
    memset(ptContext, 0, sizeof(VAT_TEST_CONTEXT_T));

    /* Set default configuration */
    ptContext->config.node_address = 10;
    ptContext->config.baud_rate = 250000;       /* 250 kbps */
    ptContext->config.epr_ms = 100;             /* 100ms packet rate */
    ptContext->config.input_assembly = VAT_INPUT_ASSEMBLY_2;
    ptContext->config.output_assembly = VAT_OUTPUT_ASSEMBLY_7;
    ptContext->config.enable_logging = true;
    ptContext->config.enable_validation = true;

    /* Initialize status */
    ptContext->test_running = true;

    LOG_PRINTF("[VAT Test] Initialized\n");
}

/*****************************************************************************/
/*! Configure test parameters
 *
 * \param ptContext  [in/out]  Test context
 * \param ptConfig   [in]      Configuration parameters
 */
/*****************************************************************************/
void VAT_Test_Configure(VAT_TEST_CONTEXT_T* ptContext, const VAT_TEST_CONFIG_T* ptConfig)
{
    if (ptContext == NULL || ptConfig == NULL)
        return;

    memcpy(&ptContext->config, ptConfig, sizeof(VAT_TEST_CONFIG_T));

    LOG_PRINTF("[VAT Test] Configured:\n");
    LOG_PRINTF("  Node Address: %u\n", ptContext->config.node_address);
    LOG_PRINTF("  Baud Rate: %u bps\n", ptContext->config.baud_rate);
    LOG_PRINTF("  EPR: %u ms\n", ptContext->config.epr_ms);
    LOG_PRINTF("  Input Assembly: 0x%02X\n", ptContext->config.input_assembly);
    LOG_PRINTF("  Output Assembly: 0x%02X\n", ptContext->config.output_assembly);
}

/*****************************************************************************/
/*! Deinitialize test context
 *
 * \param ptContext  [in/out]  Test context to cleanup
 */
/*****************************************************************************/
void VAT_Test_Deinit(VAT_TEST_CONTEXT_T* ptContext)
{
    if (ptContext == NULL)
        return;

    ptContext->test_running = false;

    LOG_PRINTF("[VAT Test] Deinitialized\n");
    VAT_Test_PrintStats(ptContext);
}

/*****************************************************************************/
/*! Read input data from device
 *
 * \param ptContext  [in/out]  Test context
 * \param hChannel   [in]      cifX channel handle
 *
 * \return Error code (CIFX_NO_ERROR on success)
 */
/*****************************************************************************/
int32_t VAT_Test_ReadInput(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)
{
    int32_t lRet = CIFX_NO_ERROR;
    void* pvData = NULL;
    uint32_t ulDataLen = 0;

    if (ptContext == NULL || hChannel == NULL)
        return CIFX_INVALID_HANDLE;

    /* Select input assembly based on configuration */
    switch (ptContext->config.input_assembly)
    {
        case VAT_INPUT_ASSEMBLY_1:
            pvData = &ptContext->input_asm1;
            ulDataLen = sizeof(VAT_INPUT_ASSEMBLY_1_T);
            break;

        case VAT_INPUT_ASSEMBLY_2:
            pvData = &ptContext->input_asm2;
            ulDataLen = sizeof(VAT_INPUT_ASSEMBLY_2_T);
            break;

        case VAT_INPUT_ASSEMBLY_3:
            pvData = &ptContext->input_asm3;
            ulDataLen = sizeof(VAT_INPUT_ASSEMBLY_3_T);
            break;

        case VAT_INPUT_ASSEMBLY_100:
            pvData = &ptContext->input_asm100;
            ulDataLen = sizeof(VAT_INPUT_ASSEMBLY_100_T);
            break;

        case VAT_INPUT_ASSEMBLY_101:
            pvData = &ptContext->input_asm101;
            ulDataLen = sizeof(VAT_INPUT_ASSEMBLY_101_T);
            break;

        default:
            return CIFX_BUFFER_TOO_SHORT;
    }

    /* Read data from device */
    lRet = xChannelIORead((CIFXHANDLE)hChannel, 0, 0, ulDataLen, pvData, 0);

    /* Update statistics */
    ptContext->stats.total_read_count++;
    ptContext->stats.last_read_time_ms = VAT_GetTimestampMs();

    if (lRet != CIFX_NO_ERROR)
    {
        ptContext->stats.read_error_count++;
        ptContext->stats.last_error_code = lRet;
        ptContext->input_data_valid = false;

        LOG_PRINTF("[VAT Test] Read error: 0x%08X\n", lRet);
    }
    else
    {
        ptContext->input_data_valid = true;

        /* Check for exceptions */
        if (VAT_Test_HasException(ptContext))
        {
            ptContext->stats.exception_count++;
            LOG_PRINTF("[VAT Test] Exception detected: 0x%02X\n",
                      VAT_Test_GetExceptionStatus(ptContext));
        }

        /* Log data if enabled */
        if (ptContext->config.enable_logging)
        {
            VAT_Test_LogInputData(ptContext, ptContext->config.input_assembly);
        }
    }

    return lRet;
}

/*****************************************************************************/
/*! Write output data to device
 *
 * \param ptContext  [in/out]  Test context
 * \param hChannel   [in]      cifX channel handle
 *
 * \return Error code (CIFX_NO_ERROR on success)
 */
/*****************************************************************************/
int32_t VAT_Test_WriteOutput(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)
{
    int32_t lRet = CIFX_NO_ERROR;
    void* pvData = NULL;
    uint32_t ulDataLen = 0;

    if (ptContext == NULL || hChannel == NULL)
        return CIFX_INVALID_HANDLE;

    /* Select output assembly based on configuration */
    switch (ptContext->config.output_assembly)
    {
        case VAT_OUTPUT_ASSEMBLY_7:
            pvData = &ptContext->output_asm7;
            ulDataLen = sizeof(VAT_OUTPUT_ASSEMBLY_7_T);
            break;

        case VAT_OUTPUT_ASSEMBLY_8:
            pvData = &ptContext->output_asm8;
            ulDataLen = sizeof(VAT_OUTPUT_ASSEMBLY_8_T);
            break;

        case VAT_OUTPUT_ASSEMBLY_102:
            pvData = &ptContext->output_asm102;
            ulDataLen = sizeof(VAT_OUTPUT_ASSEMBLY_102_T);
            break;

        default:
            return CIFX_BUFFER_TOO_SHORT;
    }

    /* Log data if enabled */
    if (ptContext->config.enable_logging)
    {
        VAT_Test_LogOutputData(ptContext, ptContext->config.output_assembly);
    }

    /* Write data to device */
    lRet = xChannelIOWrite((CIFXHANDLE)hChannel, 0, 0, ulDataLen, pvData, 0);

    /* Update statistics */
    ptContext->stats.total_write_count++;
    ptContext->stats.last_write_time_ms = VAT_GetTimestampMs();

    if (lRet != CIFX_NO_ERROR)
    {
        ptContext->stats.write_error_count++;
        ptContext->stats.last_error_code = lRet;
        ptContext->output_data_sent = false;

        LOG_PRINTF("[VAT Test] Write error: 0x%08X\n", lRet);
    }
    else
    {
        ptContext->output_data_sent = true;
    }

    return lRet;
}

/*****************************************************************************/
/*! Set control mode
 *
 * \param ptContext  [in/out]  Test context
 * \param eMode      [in]      Control mode to set
 */
/*****************************************************************************/
void VAT_Test_SetControlMode(VAT_TEST_CONTEXT_T* ptContext, VAT_CONTROL_MODE_E eMode)
{
    if (ptContext == NULL)
        return;

    /* Update output assembly with control mode */
    ptContext->output_asm8.control_mode = (uint8_t)eMode;
    ptContext->output_asm102.control_mode = (uint8_t)eMode;

    LOG_PRINTF("[VAT Test] Control mode set to: %u\n", eMode);
}

/*****************************************************************************/
/*! Set pressure setpoint
 *
 * \param ptContext  [in/out]  Test context
 * \param pressure   [in]      Pressure setpoint value
 */
/*****************************************************************************/
void VAT_Test_SetPressureSetpoint(VAT_TEST_CONTEXT_T* ptContext, int16_t pressure)
{
    if (ptContext == NULL)
        return;

    /* Update all output assemblies */
    ptContext->output_asm7.control_setpoint = pressure;
    ptContext->output_asm8.control_setpoint = pressure;
    ptContext->output_asm102.control_setpoint = pressure;

    LOG_PRINTF("[VAT Test] Pressure setpoint set to: %d\n", pressure);
}

/*****************************************************************************/
/*! Check if device is ready
 *
 * \param ptContext  [in]  Test context
 *
 * \return true if device ready, false otherwise
 */
/*****************************************************************************/
bool VAT_Test_IsDeviceReady(const VAT_TEST_CONTEXT_T* ptContext)
{
    if (ptContext == NULL || !ptContext->input_data_valid)
        return false;

    uint8_t status = VAT_Test_GetDeviceStatus(ptContext);
    return (status & VAT_DEVICE_STATUS_READY) != 0;
}

/*****************************************************************************/
/*! Check if device has exception
 *
 * \param ptContext  [in]  Test context
 *
 * \return true if exception present, false otherwise
 */
/*****************************************************************************/
bool VAT_Test_HasException(const VAT_TEST_CONTEXT_T* ptContext)
{
    if (ptContext == NULL || !ptContext->input_data_valid)
        return false;

    return VAT_Test_GetExceptionStatus(ptContext) != 0;
}

/*****************************************************************************/
/*! Get exception status
 *
 * \param ptContext  [in]  Test context
 *
 * \return Exception status byte
 */
/*****************************************************************************/
uint8_t VAT_Test_GetExceptionStatus(const VAT_TEST_CONTEXT_T* ptContext)
{
    if (ptContext == NULL)
        return 0;

    /* Exception status is in first byte of assemblies 2, 3, 100, 101 */
    switch (ptContext->config.input_assembly)
    {
        case VAT_INPUT_ASSEMBLY_2:
            return ptContext->input_asm2.exception_status;

        case VAT_INPUT_ASSEMBLY_3:
            return ptContext->input_asm3.exception_status;

        case VAT_INPUT_ASSEMBLY_100:
            return ptContext->input_asm100.exception_status;

        case VAT_INPUT_ASSEMBLY_101:
            return ptContext->input_asm101.exception_status;

        default:
            return 0;
    }
}

/*****************************************************************************/
/*! Get device status
 *
 * \param ptContext  [in]  Test context
 *
 * \return Device status byte
 */
/*****************************************************************************/
uint8_t VAT_Test_GetDeviceStatus(const VAT_TEST_CONTEXT_T* ptContext)
{
    if (ptContext == NULL)
        return 0;

    /* Device status is available in assemblies 100, 101 */
    switch (ptContext->config.input_assembly)
    {
        case VAT_INPUT_ASSEMBLY_100:
            return ptContext->input_asm100.device_status;

        case VAT_INPUT_ASSEMBLY_101:
            return ptContext->input_asm101.device_status;

        default:
            return 0;
    }
}

/*****************************************************************************/
/*! Get test statistics
 *
 * \param ptContext  [in]   Test context
 * \param ptStats    [out]  Statistics structure to fill
 */
/*****************************************************************************/
void VAT_Test_GetStats(const VAT_TEST_CONTEXT_T* ptContext, VAT_TEST_STATS_T* ptStats)
{
    if (ptContext == NULL || ptStats == NULL)
        return;

    memcpy(ptStats, &ptContext->stats, sizeof(VAT_TEST_STATS_T));
}

/*****************************************************************************/
/*! Reset test statistics
 *
 * \param ptContext  [in/out]  Test context
 */
/*****************************************************************************/
void VAT_Test_ResetStats(VAT_TEST_CONTEXT_T* ptContext)
{
    if (ptContext == NULL)
        return;

    memset(&ptContext->stats, 0, sizeof(VAT_TEST_STATS_T));
    LOG_PRINTF("[VAT Test] Statistics reset\n");
}

/*****************************************************************************/
/*! Print test statistics
 *
 * \param ptContext  [in]  Test context
 */
/*****************************************************************************/
void VAT_Test_PrintStats(const VAT_TEST_CONTEXT_T* ptContext)
{
    if (ptContext == NULL)
        return;

    printf("\n========== VAT Test Statistics ==========\n");
    printf("Total Read Operations:   %u\n", ptContext->stats.total_read_count);
    printf("Total Write Operations:  %u\n", ptContext->stats.total_write_count);
    printf("Read Errors:             %u\n", ptContext->stats.read_error_count);
    printf("Write Errors:            %u\n", ptContext->stats.write_error_count);
    printf("Exception Count:         %u\n", ptContext->stats.exception_count);
    printf("Timeout Count:           %u\n", ptContext->stats.timeout_count);
    printf("Last Error Code:         0x%08X\n", ptContext->stats.last_error_code);
    printf("=========================================\n\n");
}

/*****************************************************************************/
/*! Log input data
 *
 * \param ptContext    [in]  Test context
 * \param assembly_id  [in]  Assembly instance ID
 */
/*****************************************************************************/
void VAT_Test_LogInputData(const VAT_TEST_CONTEXT_T* ptContext, uint8_t assembly_id)
{
    if (ptContext == NULL || !ptContext->config.enable_logging)
        return;

    printf("[VAT Input 0x%02X] ", assembly_id);

    switch (assembly_id)
    {
        case VAT_INPUT_ASSEMBLY_1:
            printf("PV=%d\n", ptContext->input_asm1.process_variable);
            break;

        case VAT_INPUT_ASSEMBLY_2:
            printf("Exception=0x%02X, Pressure=%d\n",
                   ptContext->input_asm2.exception_status,
                   ptContext->input_asm2.pressure);
            break;

        case VAT_INPUT_ASSEMBLY_3:
            printf("Exception=0x%02X, Pressure=%d, Position=%d\n",
                   ptContext->input_asm3.exception_status,
                   ptContext->input_asm3.pressure,
                   ptContext->input_asm3.position);
            break;

        case VAT_INPUT_ASSEMBLY_100:
            printf("Exception=0x%02X, Pressure=%d, Position=%d, Status=0x%02X, Access=0x%02X\n",
                   ptContext->input_asm100.exception_status,
                   ptContext->input_asm100.pressure,
                   ptContext->input_asm100.position,
                   ptContext->input_asm100.device_status,
                   ptContext->input_asm100.access_mode);
            break;

        case VAT_INPUT_ASSEMBLY_101:
            printf("Exception=0x%02X, Pressure=%d, Position=%d, Discrete=0x%02X, Status=0x%02X\n",
                   ptContext->input_asm101.exception_status,
                   ptContext->input_asm101.pressure,
                   ptContext->input_asm101.position,
                   ptContext->input_asm101.discrete_inputs,
                   ptContext->input_asm101.device_status);
            break;

        default:
            printf("Unknown assembly\n");
            break;
    }
}

/*****************************************************************************/
/*! Log output data
 *
 * \param ptContext    [in]  Test context
 * \param assembly_id  [in]  Assembly instance ID
 */
/*****************************************************************************/
void VAT_Test_LogOutputData(const VAT_TEST_CONTEXT_T* ptContext, uint8_t assembly_id)
{
    if (ptContext == NULL || !ptContext->config.enable_logging)
        return;

    printf("[VAT Output 0x%02X] ", assembly_id);

    switch (assembly_id)
    {
        case VAT_OUTPUT_ASSEMBLY_7:
            printf("Setpoint=%d, Instance=%u\n",
                   ptContext->output_asm7.control_setpoint,
                   ptContext->output_asm7.control_instance);
            break;

        case VAT_OUTPUT_ASSEMBLY_8:
            printf("Mode=%u, Setpoint=%d, Instance=%u\n",
                   ptContext->output_asm8.control_mode,
                   ptContext->output_asm8.control_setpoint,
                   ptContext->output_asm8.control_instance);
            break;

        case VAT_OUTPUT_ASSEMBLY_102:
            printf("Mode=%u, Setpoint=%d, Instance=%u, Learn=%u, Cal=%u, Zero=%u\n",
                   ptContext->output_asm102.control_mode,
                   ptContext->output_asm102.control_setpoint,
                   ptContext->output_asm102.control_instance,
                   ptContext->output_asm102.auto_learn,
                   ptContext->output_asm102.calibration_scale,
                   ptContext->output_asm102.zero_adjust);
            break;

        default:
            printf("Unknown assembly\n");
            break;
    }
}

/*****************************************************************************/
/*! Basic pressure control test
 *
 * \param ptContext  [in/out]  Test context
 * \param hChannel   [in]      cifX channel handle
 */
/*****************************************************************************/
void VAT_Test_BasicPressureControl(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)
{
    printf("\n========== Basic Pressure Control Test ==========\n");

    /* Configure for basic pressure control */
    ptContext->config.input_assembly = VAT_INPUT_ASSEMBLY_2;
    ptContext->config.output_assembly = VAT_OUTPUT_ASSEMBLY_7;

    /* Set pressure setpoint to 500 (scaled units) */
    VAT_Test_SetPressureSetpoint(ptContext, 500);

    /* Test cycle */
    for (int i = 0; i < 10; i++)
    {
        /* Write setpoint */
        VAT_Test_WriteOutput(ptContext, hChannel);

        /* Read feedback */
        VAT_Test_ReadInput(ptContext, hChannel);

        /* Small delay */
        for (volatile int j = 0; j < 100000; j++);
    }

    printf("=================================================\n\n");
}

/*****************************************************************************/
/*! Full calibration test
 *
 * \param ptContext  [in/out]  Test context
 * \param hChannel   [in]      cifX channel handle
 */
/*****************************************************************************/
void VAT_Test_FullCalibration(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)
{
    printf("\n========== Full Calibration Test ==========\n");

    /* Configure for full control */
    ptContext->config.input_assembly = VAT_INPUT_ASSEMBLY_100;
    ptContext->config.output_assembly = VAT_OUTPUT_ASSEMBLY_102;

    /* Enable auto-learn mode */
    VAT_Test_SetControlMode(ptContext, VAT_CONTROL_MODE_PRESSURE);
    ptContext->output_asm102.auto_learn = 1;
    ptContext->output_asm102.calibration_scale = 100;
    ptContext->output_asm102.zero_adjust = 0;

    /* Test cycle */
    for (int i = 0; i < 5; i++)
    {
        VAT_Test_WriteOutput(ptContext, hChannel);
        VAT_Test_ReadInput(ptContext, hChannel);

        /* Check device status */
        if (VAT_Test_IsDeviceReady(ptContext))
        {
            printf("Device is ready\n");
        }

        for (volatile int j = 0; j < 100000; j++);
    }

    printf("===========================================\n\n");
}
