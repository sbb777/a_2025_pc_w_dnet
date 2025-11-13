/**
 * @file AppDNS_DemoApplication.h
 * @brief DeviceNet Demo Application Header
 * @date 2025-01-10
 *
 * This file contains the main DeviceNet application interface.
 * Implementation should be copied from the DeviceNet SDK.
 */

#ifndef APPDNS_DEMOAPPLICATION_H
#define APPDNS_DEMOAPPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Initialize DeviceNet Demo Application
 * @return 0 on success, negative on error
 */
int AppDNS_DemoApplication_Init(void);

/**
 * @brief Run DeviceNet Demo Application main loop
 * @return 0 on success, negative on error
 */
int AppDNS_DemoApplication_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* APPDNS_DEMOAPPLICATION_H */
