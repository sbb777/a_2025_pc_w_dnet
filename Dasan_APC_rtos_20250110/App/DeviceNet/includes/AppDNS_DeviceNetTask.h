/**
 * @file AppDNS_DeviceNetTask.h
 * @brief DeviceNet FreeRTOS Task Header
 * @date 2025-01-10
 *
 * This file contains the FreeRTOS task definition for DeviceNet communication.
 * Integrates DeviceNet SDK with FreeRTOS task management.
 */

#ifndef APPDNS_DEVICENETTASK_H
#define APPDNS_DEVICENETTASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define DEVICENET_TASK_PRIORITY      (tskIDLE_PRIORITY + 3)
#define DEVICENET_TASK_STACK_SIZE    (1024 * 4)  /* 4KB stack */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Create and start DeviceNet FreeRTOS task
 * @return pdPASS on success, pdFAIL on error
 */
BaseType_t AppDNS_DeviceNetTask_Create(void);

/**
 * @brief DeviceNet task function (FreeRTOS task entry point)
 * @param argument Task parameter (not used)
 */
void AppDNS_DeviceNetTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* APPDNS_DEVICENETTASK_H */
