/**
 * @file AppDNS_DeviceNetTask.c
 * @brief DeviceNet FreeRTOS Task Implementation
 * @date 2025-01-13
 *
 * This file contains the FreeRTOS task implementation for DeviceNet communication.
 * Uses CMSIS-RTOS v2 API for compatibility with STM32CubeIDE generated code.
 */

/* Includes ------------------------------------------------------------------*/
#include "AppDNS_DeviceNetTask.h"
#include "AppDNS_DemoApplication.h"
#include "DeviceNet_Config.h"
#include "cmsis_os.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static osThreadId_t deviceNetTaskHandle = NULL;

/* Task attributes */
const osThreadAttr_t deviceNetTask_attributes = {
    .name = DEVICENET_TASK_NAME,
    .stack_size = DEVICENET_TASK_STACK_SIZE,
    .priority = DEVICENET_TASK_PRIORITY,
};

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief DeviceNet task function (FreeRTOS task entry point)
 * @param argument Task parameter (not used)
 */
void AppDNS_DeviceNetTask(void *argument)
{
#if ENABLE_DEVICENET
    /* FULL MODE: Complete DeviceNet operation */

    /* Initialize DeviceNet application */
    if (AppDNS_DemoApplication_Init() != 0)
    {
        /* Initialization failed - suspend task */
        osThreadSuspend(deviceNetTaskHandle);
    }

    /* Infinite loop */
    for (;;)
    {
        /* Run DeviceNet application main loop */
        AppDNS_DemoApplication_Run();

        /* Delay based on DeviceNet timing requirements (10ms cycle) */
        osDelay(10);
    }

#else
    /* STUB MODE: Minimal operation for testing */

    /* Initialize (stub) */
    AppDNS_DemoApplication_Init();

    /* Infinite loop */
    for (;;)
    {
        /* Run stub (just increments counter) */
        AppDNS_DemoApplication_Run();

        /* Longer delay in stub mode to reduce CPU usage */
        osDelay(100);
    }
#endif
}

/**
 * @brief Create and start DeviceNet FreeRTOS task
 * @return osOK on success, error code on failure
 */
osStatus_t AppDNS_DeviceNetTask_Create(void)
{
    /* Create the task using CMSIS-RTOS v2 API */
    deviceNetTaskHandle = osThreadNew(
        AppDNS_DeviceNetTask,
        NULL,
        &deviceNetTask_attributes
    );

    if (deviceNetTaskHandle == NULL)
    {
        return osError;
    }

    return osOK;
}

/**
 * @brief Get DeviceNet task handle
 * @return Task handle or NULL if not created
 */
osThreadId_t AppDNS_GetTaskHandle(void)
{
    return deviceNetTaskHandle;
}
