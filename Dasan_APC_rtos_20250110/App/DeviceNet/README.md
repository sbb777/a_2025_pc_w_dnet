# DeviceNet Integration Module

This directory contains the DeviceNet protocol integration for the Dasan APC RTOS project.

## Directory Structure

```
DeviceNet/
├── includes/                        # Header files
│   ├── DNS_API/                     # DNS packet API headers (from SDK)
│   │   └── README.md               # Instructions for copying SDK files
│   ├── AppDNS_DemoApplication.h    # Main application header
│   └── AppDNS_DeviceNetTask.h      # FreeRTOS task header
│
└── Sources/                         # Source files
    ├── AppDNS_DemoApplication.c           # Main application implementation
    ├── AppDNS_DemoApplicationFunctions.c  # Helper functions
    ├── AppDNS_ExplicitMsg.c               # Explicit messaging
    └── AppDNS_DeviceNetTask.c             # FreeRTOS task implementation
```

## Overview

DeviceNet is an industrial communication protocol based on CAN (Controller Area Network). This module integrates the DeviceNet Slave (DNS) protocol stack with the FreeRTOS-based application.

## Integration Steps

### 1. Copy DeviceNet SDK Files

Copy the required files from the DeviceNet SDK:

1. **DNS_API Headers**: Copy all DNS API headers to `includes/DNS_API/`
   - These provide the low-level DeviceNet protocol interface
   - See `includes/DNS_API/README.md` for details

2. **Application Templates**: Copy the demo application source files to `Sources/`
   - `AppDNS_DemoApplication.c`
   - `AppDNS_DemoApplicationFunctions.c`
   - `AppDNS_ExplicitMsg.c`

### 2. Configure cifXToolkit

The DeviceNet stack runs on top of the cifXToolkit, which provides the communication interface to the netX 90 chip. See `../cifXToolkit/README.md` for cifXToolkit integration.

### 3. FreeRTOS Task Integration

The `AppDNS_DeviceNetTask.c` file provides a FreeRTOS task wrapper for the DeviceNet stack:

```c
/* Create DeviceNet task in main.c or freertos.c */
#include "AppDNS_DeviceNetTask.h"

void MX_FREERTOS_Init(void)
{
    /* Create DeviceNet task */
    if (AppDNS_DeviceNetTask_Create() != pdPASS)
    {
        Error_Handler();
    }
}
```

### 4. Configuration Parameters

Configure DeviceNet parameters:
- **MAC ID**: Node address on the DeviceNet network (0-63)
- **Baud Rate**: 125 kbps, 250 kbps, or 500 kbps
- **I/O Assembly**: Input/output data configuration
- **Explicit Messaging**: Command/response configuration

### 5. Network Object

The DeviceNet network object must be configured in the netX 90 firmware. This is typically done through:
- Network configuration files (`.nxd` files)
- Hilscher netX Studio tools
- Runtime configuration packets

## Task Configuration

The DeviceNet task is configured in `includes/AppDNS_DeviceNetTask.h`:

```c
#define DEVICENET_TASK_PRIORITY      (tskIDLE_PRIORITY + 3)
#define DEVICENET_TASK_STACK_SIZE    (1024 * 4)  /* 4KB stack */
```

Adjust these values based on your application requirements.

## Dependencies

- **FreeRTOS**: Task management and synchronization
- **cifXToolkit**: Communication with netX 90 chip
- **DeviceNet SDK**: Protocol stack implementation
- **STM32 HAL**: Hardware abstraction (SPI, GPIO)

## Communication Flow

```
Application (DeviceNet Task)
    ↓
DeviceNet SDK (DNS API)
    ↓
cifXToolkit (DPM Access)
    ↓
SerialDPM (SPI Protocol)
    ↓
STM32 SPI5 HAL
    ↓
netX 90 Chip (DeviceNet Protocol)
    ↓
DeviceNet Network
```

## Testing

1. **Network Initialization**: Verify DeviceNet network starts successfully
2. **I/O Data**: Test periodic I/O data exchange
3. **Explicit Messaging**: Test command/response communication
4. **Error Handling**: Verify error detection and recovery

## References

- DeviceNet Specification (ODVA)
- Hilscher DeviceNet SDK Documentation
- netX 90 User Manual
- cifXToolkit API Reference

## Notes

- The DeviceNet task priority should be higher than application tasks but lower than critical system tasks
- Stack size may need adjustment based on the complexity of your I/O assembly
- Ensure proper timing for DeviceNet network communication (typically 10ms cycle time)
