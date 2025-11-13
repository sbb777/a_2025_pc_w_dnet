/**
 * @file DeviceNet_Config.h
 * @brief DeviceNet Integration Configuration
 * @date 2025-01-13
 *
 * This file controls the DeviceNet integration build configuration.
 *
 * IMPORTANT: Set ENABLE_DEVICENET to 1 to activate DeviceNet functionality.
 * When disabled, all DeviceNet code will be excluded from the build.
 */

#ifndef DEVICENET_CONFIG_H
#define DEVICENET_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * DeviceNet Feature Control
 * =============================================================================
 *
 * ENABLE_DEVICENET: Master switch for DeviceNet functionality
 * - 0: DeviceNet disabled (stub mode, minimal code, no external dependencies)
 * - 1: DeviceNet enabled (full functionality, requires cifXToolkit and SDK)
 *
 * NOTE: Currently set to 0 (DISABLED) because:
 * 1. cifXToolkit library files are not yet copied
 * 2. DeviceNet SDK files are not yet copied
 * 3. SPI5 MISO pin is not configured in hardware (only SCK and MOSI present)
 *
 * To enable:
 * 1. Copy cifXToolkit files to App/cifXToolkit/Source/
 * 2. Copy DeviceNet SDK files to App/DeviceNet/includes/DNS_API/
 * 3. Configure SPI5_MISO pin in STM32CubeMX (recommend PF8)
 * 4. Change ENABLE_DEVICENET to 1
 * 5. Rebuild project
 */
#define ENABLE_DEVICENET    0    /* 0 = DISABLED, 1 = ENABLED */

/* =============================================================================
 * Hardware Configuration
 * =============================================================================
 */

#if ENABLE_DEVICENET

/* SPI Configuration */
#define DEVICENET_SPI_INSTANCE      SPI5
#define DEVICENET_SPI_TIMEOUT_MS    1000

/* GPIO Configuration - netX 90 Control Signals */
#define DEVICENET_CS_PORT           GPIOF
#define DEVICENET_CS_PIN            GPIO_PIN_6

#define DEVICENET_SRDY_PORT         GPIOF
#define DEVICENET_SRDY_PIN          GPIO_PIN_8   /* TODO: Configure in CubeMX */

/* FreeRTOS Task Configuration */
#define DEVICENET_TASK_PRIORITY     (osPriorityAboveNormal)  /* Between poll and async */
#define DEVICENET_TASK_STACK_SIZE   (1024 * 4)   /* 4KB stack */
#define DEVICENET_TASK_NAME         "DeviceNetTask"

/* DeviceNet Network Configuration */
#define DEVICENET_DEFAULT_MAC_ID    5
#define DEVICENET_DEFAULT_BAUDRATE  1    /* 0=125k, 1=250k, 2=500k */

/* I/O Assembly Configuration */
#define DEVICENET_INPUT_SIZE        8    /* 8 bytes input data */
#define DEVICENET_OUTPUT_SIZE       8    /* 8 bytes output data */

#else  /* ENABLE_DEVICENET == 0 */

/* Stub mode - minimal configuration */
#define DEVICENET_TASK_PRIORITY     (osPriorityLow)
#define DEVICENET_TASK_STACK_SIZE   (128 * 4)    /* Minimal stack */
#define DEVICENET_TASK_NAME         "DevNetStub"

#endif  /* ENABLE_DEVICENET */

/* =============================================================================
 * Build Information
 * =============================================================================
 */

#define DEVICENET_BUILD_DATE        "2025-01-13"
#define DEVICENET_VERSION_MAJOR     0
#define DEVICENET_VERSION_MINOR     1
#define DEVICENET_VERSION_PATCH     0

/* Feature flags for conditional compilation */
#if ENABLE_DEVICENET
#define DEVICENET_FEATURE_FULL      1
#define DEVICENET_FEATURE_STUB      0
#else
#define DEVICENET_FEATURE_FULL      0
#define DEVICENET_FEATURE_STUB      1
#endif

#ifdef __cplusplus
}
#endif

#endif /* DEVICENET_CONFIG_H */
