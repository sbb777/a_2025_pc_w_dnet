/**
 * @file AppDNS_DeviceNetTask.c
 * @brief DeviceNet FreeRTOS Task Implementation
 * @date 2025-01-10
 *
 * This file contains the FreeRTOS task implementation for DeviceNet communication.
 * Integrates DeviceNet SDK with FreeRTOS task management.
 */

/* Includes ------------------------------------------------------------------*/
#include "AppDNS_DeviceNetTask.h"
#include "AppDNS_DemoApplication.h"
#include "FreeRTOS.h"
#include "task.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static TaskHandle_t xDeviceNetTaskHandle = NULL;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief DeviceNet task function (FreeRTOS task entry point)
 * @param argument Task parameter (not used)
 */
void AppDNS_DeviceNetTask(void *argument)
{
    /* Initialize DeviceNet application */
    if (AppDNS_DemoApplication_Init() != 0)
    {
        /* Initialization failed - suspend task */
        vTaskSuspend(NULL);
    }

    /* Infinite loop */
    for (;;)
    {
        /* Run DeviceNet application main loop */
        AppDNS_DemoApplication_Run();

        /* Add small delay to prevent CPU hogging */
        /* Adjust delay based on DeviceNet timing requirements */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief Create and start DeviceNet FreeRTOS task
 * @return pdPASS on success, pdFAIL on error
 */
BaseType_t AppDNS_DeviceNetTask_Create(void)
{
    BaseType_t xReturned;

    /* Create the task */
    xReturned = xTaskCreate(
        AppDNS_DeviceNetTask,        /* Task function */
        "DeviceNetTask",              /* Task name */
        DEVICENET_TASK_STACK_SIZE,   /* Stack size */
        NULL,                         /* Task parameter */
        DEVICENET_TASK_PRIORITY,     /* Task priority */
        &xDeviceNetTaskHandle        /* Task handle */
    );

    return xReturned;
}
