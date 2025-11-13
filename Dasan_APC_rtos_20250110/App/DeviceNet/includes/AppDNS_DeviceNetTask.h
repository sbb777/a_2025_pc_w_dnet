/**
 * @file AppDNS_DeviceNetTask.h
 * @brief DeviceNet FreeRTOS Task Header
 * @date 2025-01-13
 *
 * This file contains the FreeRTOS task definition for DeviceNet communication.
 * Uses CMSIS-RTOS v2 API for compatibility with STM32CubeIDE.
 */

#ifndef APPDNS_DEVICENETTASK_H
#define APPDNS_DEVICENETTASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Note: Actual values are defined in DeviceNet_Config.h */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Create and start DeviceNet FreeRTOS task
 * @return osOK on success, error code on failure
 */
osStatus_t AppDNS_DeviceNetTask_Create(void);

/**
 * @brief DeviceNet task function (FreeRTOS task entry point)
 * @param argument Task parameter (not used)
 */
void AppDNS_DeviceNetTask(void *argument);

/**
 * @brief Get DeviceNet task handle
 * @return Task handle or NULL if not created
 */
osThreadId_t AppDNS_GetTaskHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* APPDNS_DEVICENETTASK_H */
