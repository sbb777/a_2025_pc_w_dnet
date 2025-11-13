/**************************************************************************************
 * VAT Adaptive Pressure Controller DeviceNet Test - Main Application
 *
 * This is a standalone test application demonstrating how to test
 * DeviceNet communication with VAT pressure controller
 **************************************************************************************/

#include "vat_devicenet_test.h"
#include <stdio.h>
#include <stdlib.h>

/* If using real cifX hardware, include these */
#ifdef USE_CIFX_API
#include "cifXUser.h"
#include "cifXErrors.h"

/*****************************************************************************/
/*! Main application entry point
 */
/*****************************************************************************/
int main(int argc, char* argv[])
{
    CIFXHANDLE hDriver = NULL;
    CIFXHANDLE hChannel = NULL;
    VAT_TEST_CONTEXT_T tTestContext;
    VAT_TEST_CONFIG_T tConfig;
    int32_t lRet = 0;
    char szDeviceName[] = "cifX0";

    printf("========================================\n");
    printf(" VAT Pressure Controller Test v1.0\n");
    printf(" DeviceNet Communication Test\n");
    printf("========================================\n\n");

    /* Initialize test context */
    VAT_Test_Init(&tTestContext);

    /* Configure test parameters */
    tConfig.node_address = 10;              /* VAT device node address */
    tConfig.baud_rate = 250000;             /* 250 kbps */
    tConfig.epr_ms = 100;                   /* 100ms expected packet rate */
    tConfig.input_assembly = VAT_INPUT_ASSEMBLY_100;    /* Full status */
    tConfig.output_assembly = VAT_OUTPUT_ASSEMBLY_8;    /* Control with mode */
    tConfig.enable_logging = true;
    tConfig.enable_validation = true;

    VAT_Test_Configure(&tTestContext, &tConfig);

    /* Open cifX driver */
    printf("Opening cifX driver...\n");
    if (CIFX_NO_ERROR != (lRet = xDriverOpen(&hDriver)))
    {
        printf("ERROR: xDriverOpen failed: 0x%08X\n", (unsigned int)lRet);
        return -1;
    }

    /* Open channel 0 */
    printf("Opening channel %s...\n", szDeviceName);
    if (CIFX_NO_ERROR != (lRet = xChannelOpen(hDriver, szDeviceName, 0, &hChannel)))
    {
        printf("ERROR: xChannelOpen failed: 0x%08X\n", (unsigned int)lRet);
        xDriverClose(hDriver);
        return -1;
    }

    /* Wait for channel ready */
    printf("Waiting for channel ready...\n");
    CHANNEL_INFORMATION tChannelInfo;
    do
    {
        xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
    }
    while (!(tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY));

    printf("Channel ready!\n\n");

    /* Run test scenarios */
    printf("========================================\n");
    printf(" Running Test Scenarios\n");
    printf("========================================\n\n");

    /* Test 1: Basic pressure control */
    VAT_Test_BasicPressureControl(&tTestContext, hChannel);

    /* Test 2: Full calibration */
    VAT_Test_FullCalibration(&tTestContext, hChannel);

    /* Test 3: Continuous monitoring loop */
    printf("\n========== Continuous Monitoring ==========\n");
    printf("Running for 10 seconds...\n");

    VAT_Test_SetControlMode(&tTestContext, VAT_CONTROL_MODE_PRESSURE);
    VAT_Test_SetPressureSetpoint(&tTestContext, 750);

    for (int i = 0; i < 100; i++)  /* 100 cycles * 100ms = 10 seconds */
    {
        /* Write control data */
        lRet = VAT_Test_WriteOutput(&tTestContext, hChannel);
        if (lRet != CIFX_NO_ERROR)
        {
            printf("Write error at cycle %d: 0x%08X\n", i, lRet);
        }

        /* Read sensor data */
        lRet = VAT_Test_ReadInput(&tTestContext, hChannel);
        if (lRet != CIFX_NO_ERROR)
        {
            printf("Read error at cycle %d: 0x%08X\n", i, lRet);
        }

        /* Check for exceptions */
        if (VAT_Test_HasException(&tTestContext))
        {
            printf("WARNING: Exception detected at cycle %d: 0x%02X\n",
                   i, VAT_Test_GetExceptionStatus(&tTestContext));
        }

        /* Delay 100ms */
        for (volatile int j = 0; j < 1000000; j++);
    }

    printf("===========================================\n\n");

    /* Print final statistics */
    VAT_Test_PrintStats(&tTestContext);

    /* Cleanup */
    printf("Closing channel...\n");
    xChannelClose(hChannel);

    printf("Closing driver...\n");
    xDriverClose(hDriver);

    VAT_Test_Deinit(&tTestContext);

    printf("\nTest completed successfully!\n");
    return 0;
}

#else /* USE_CIFX_API not defined - Standalone test mode */

/*****************************************************************************/
/*! Main application entry point (Standalone test mode)
 */
/*****************************************************************************/
int main(int argc, char* argv[])
{
    VAT_TEST_CONTEXT_T tTestContext;
    VAT_TEST_CONFIG_T tConfig;
    void* hMockChannel = (void*)0x12345678;  /* Mock channel handle */

    printf("========================================\n");
    printf(" VAT Pressure Controller Test v1.0\n");
    printf(" Standalone Test Mode (No Hardware)\n");
    printf("========================================\n\n");

    /* Initialize test context */
    VAT_Test_Init(&tTestContext);

    /* Configure test parameters */
    tConfig.node_address = 10;
    tConfig.baud_rate = 250000;
    tConfig.epr_ms = 100;
    tConfig.input_assembly = VAT_INPUT_ASSEMBLY_100;
    tConfig.output_assembly = VAT_OUTPUT_ASSEMBLY_8;
    tConfig.enable_logging = true;
    tConfig.enable_validation = true;

    VAT_Test_Configure(&tTestContext, &tConfig);

    /* Run test scenarios with mock channel */
    printf("\n========================================\n");
    printf(" Running Test Scenarios (Mock)\n");
    printf("========================================\n\n");

    /* Test 1: Basic pressure control */
    VAT_Test_BasicPressureControl(&tTestContext, hMockChannel);

    /* Test 2: Full calibration */
    VAT_Test_FullCalibration(&tTestContext, hMockChannel);

    /* Test 3: Simulate various control modes */
    printf("\n========== Control Mode Test ==========\n");

    const char* mode_names[] = {
        "CLOSED", "OPEN", "PRESSURE", "POSITION", "MANUAL", "THROTTLE"
    };

    for (int mode = VAT_CONTROL_MODE_CLOSED; mode <= VAT_CONTROL_MODE_THROTTLE; mode++)
    {
        printf("\nTesting %s mode...\n", mode_names[mode]);

        VAT_Test_SetControlMode(&tTestContext, (VAT_CONTROL_MODE_E)mode);
        VAT_Test_SetPressureSetpoint(&tTestContext, 500 + mode * 100);

        /* Run a few cycles */
        for (int i = 0; i < 3; i++)
        {
            VAT_Test_WriteOutput(&tTestContext, hMockChannel);
            VAT_Test_ReadInput(&tTestContext, hMockChannel);
        }
    }

    printf("=======================================\n\n");

    /* Print final statistics */
    VAT_Test_PrintStats(&tTestContext);

    /* Cleanup */
    VAT_Test_Deinit(&tTestContext);

    printf("\nStandalone test completed successfully!\n");
    printf("\nNOTE: To run with real hardware, compile with -DUSE_CIFX_API flag\n");

    return 0;
}

#endif /* USE_CIFX_API */
