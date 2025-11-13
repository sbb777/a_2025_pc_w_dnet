/**
 * @file AppDNS_DemoApplication.h
 * @brief DeviceNet Demo Application Header
 * @date 2025-01-13
 *
 * This file contains the main DeviceNet application interface.
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

/**
 * @brief Get DeviceNet initialization status
 * @return 1 if initialized, 0 otherwise
 */
uint8_t AppDNS_GetInitStatus(void);

/**
 * @brief Get stub mode counter (debug)
 * @return Current counter value (stub mode only)
 */
uint32_t AppDNS_GetStubCounter(void);

#ifdef __cplusplus
}
#endif

#endif /* APPDNS_DEMOAPPLICATION_H */
