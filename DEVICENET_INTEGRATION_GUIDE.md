# DeviceNet Integration Guide

This document describes the DeviceNet integration structure for the Dasan APC RTOS project.

## Complete Directory Structure

```
Dasan_APC_rtos_20250110/
├── App/
│   ├── backup/                      # Existing backup files
│   ├── common/                      # Existing common files
│   ├── logic/                       # 🔄 Existing application logic (PRESERVED)
│   │   └── cmd/
│   ├── model/                       # 🔄 Existing application model (PRESERVED)
│   │   └── driver/
│   │
│   ├── DeviceNet/                   # 🆕 DeviceNet integration module
│   │   ├── includes/                # Header files
│   │   │   ├── DNS_API/             # DNS packet API (to be copied from SDK)
│   │   │   │   └── README.md
│   │   │   ├── AppDNS_DemoApplication.h
│   │   │   └── AppDNS_DeviceNetTask.h   # FreeRTOS task header
│   │   ├── Sources/                 # Source files
│   │   │   ├── AppDNS_DemoApplication.c
│   │   │   ├── AppDNS_DemoApplicationFunctions.c
│   │   │   ├── AppDNS_ExplicitMsg.c
│   │   │   └── AppDNS_DeviceNetTask.c   # FreeRTOS task implementation
│   │   └── README.md                # DeviceNet integration guide
│   │
│   └── cifXToolkit/                 # 🆕 cifXToolkit library
│       ├── Source/                  # cifX core sources (to be copied)
│       │   └── README.md
│       ├── SerialDPM/               # SPI DPM interface (to be copied)
│       │   └── README.md
│       ├── OSAbstraction/           # OS abstraction layer (FreeRTOS adapted)
│       │   ├── README.md
│       │   ├── OS_Custom.c          # ✅ FreeRTOS implementation (COMPLETE)
│       │   └── OS_SPICustom.c       # ✅ SPI5 HAL implementation (COMPLETE)
│       └── README.md                # cifXToolkit integration guide
│
├── Core/                            # STM32 core files
├── Drivers/                         # STM32 HAL drivers
└── Middlewares/                     # FreeRTOS and other middleware
```

## What's New vs. What's Preserved

### ✅ Preserved (Existing Structure)
- `App/backup/` - Existing backup files
- `App/common/` - Existing common files
- `App/logic/` - **Existing application logic (PRESERVED)**
- `App/model/` - **Existing application model (PRESERVED)**
- All other existing directories

### 🆕 New Additions
- `App/DeviceNet/` - DeviceNet protocol integration
- `App/cifXToolkit/` - cifX communication toolkit

The new DeviceNet and cifXToolkit directories **coexist** with the existing logic and model folders under the App directory.

## Integration Overview

### Layer Architecture

```
┌─────────────────────────────────────────────────┐
│  Application Layer (App/logic, App/model)      │
│  - Existing business logic                     │
│  - Sensor management                           │
│  - Control algorithms                          │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  DeviceNet Layer (App/DeviceNet)                │
│  - DeviceNet protocol stack                    │
│  - I/O assembly handling                       │
│  - Explicit messaging                          │
│  - FreeRTOS task wrapper                       │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  cifXToolkit Layer (App/cifXToolkit)            │
│  - DPM access API                              │
│  - Packet handling                             │
│  - FreeRTOS abstraction                        │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  SerialDPM Layer (App/cifXToolkit/SerialDPM)   │
│  - SPI DPM protocol                            │
│  - Handshaking (SRDY/CS)                       │
│  - netX 90 specific timing                    │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  STM32 HAL Layer (OS_SPICustom.c)              │
│  - SPI5 interface                              │
│  - GPIO control                                │
│  - Hardware abstraction                        │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  Hardware (STM32F429 ←SPI5→ netX 90)          │
└─────────────────────────────────────────────────┘
```

## Integration Steps

### Phase 1: Setup Directory Structure ✅ COMPLETE

The directory structure has been created with:
- DeviceNet skeleton files and headers
- cifXToolkit OS abstraction implementation
- README files with detailed instructions

### Phase 2: Copy External Libraries (TO DO)

1. **Copy DeviceNet SDK files**:
   ```bash
   # Copy DNS API headers
   cp -r <DeviceNet_SDK>/DNS_API/* \
         Dasan_APC_rtos_20250110/App/DeviceNet/includes/DNS_API/

   # Copy application sources (replace placeholders)
   cp <DeviceNet_SDK>/AppDNS_DemoApplication.c \
      Dasan_APC_rtos_20250110/App/DeviceNet/Sources/
   ```

2. **Copy cifXToolkit files**:
   ```bash
   # Copy core toolkit sources
   cp -r <cifXToolkit>/Source/* \
         Dasan_APC_rtos_20250110/App/cifXToolkit/Source/

   # Copy SerialDPM (use netX 90 version)
   cp -r <cifXToolkit>/SerialDPM/NetX90_* \
         Dasan_APC_rtos_20250110/App/cifXToolkit/SerialDPM/
   ```

### Phase 3: STM32CubeMX Configuration (TO DO)

1. **Configure SPI5**:
   - Mode: Full Duplex Master
   - Data Size: 8 bits
   - Clock: Up to 20 MHz
   - CPOL: Low, CPHA: 1 Edge

2. **Configure GPIO**:
   - CS (Chip Select): Output
   - SRDY (Service Ready): Input with pull-up

3. **Update pin definitions** in `App/cifXToolkit/OSAbstraction/OS_SPICustom.c`

### Phase 4: Build System Integration (TO DO)

1. **Update .cproject** (Eclipse/STM32CubeIDE):
   - Add include paths:
     ```
     App/DeviceNet/includes
     App/DeviceNet/includes/DNS_API
     App/cifXToolkit/Source
     App/cifXToolkit/SerialDPM
     App/cifXToolkit/OSAbstraction
     ```

   - Add source paths:
     ```
     App/DeviceNet/Sources
     App/cifXToolkit/Source
     App/cifXToolkit/SerialDPM
     App/cifXToolkit/OSAbstraction
     ```

2. **Update Makefile** (if using make):
   - Add directories to `C_INCLUDES`
   - Add `.c` files to `C_SOURCES`

### Phase 5: FreeRTOS Integration (TO DO)

Add to `main.c` or `freertos.c`:

```c
#include "AppDNS_DeviceNetTask.h"

void MX_FREERTOS_Init(void)
{
    /* Create existing tasks... */

    /* Create DeviceNet task */
    if (AppDNS_DeviceNetTask_Create() != pdPASS)
    {
        Error_Handler();
    }
}
```

### Phase 6: Testing and Validation (TO DO)

1. **Unit Tests**:
   - SPI communication
   - GPIO handshaking
   - FreeRTOS task creation

2. **Integration Tests**:
   - cifX device initialization
   - DeviceNet network startup
   - I/O data exchange
   - Explicit messaging

3. **System Tests**:
   - Network communication
   - Real-time performance
   - Error recovery

## Hardware Requirements

### STM32F429 Configuration
- SPI5 peripheral enabled
- GPIO pins for CS and SRDY
- Minimum 64KB RAM for cifX buffers
- FreeRTOS with ~16KB heap

### netX 90 Configuration
- DeviceNet firmware loaded
- SPI DPM interface enabled
- Network configuration (MAC ID, baud rate)
- I/O assembly configured

### Connections
| STM32F429 | netX 90 | Function |
|-----------|---------|----------|
| SPI5_SCK  | SPI_CLK | SPI Clock |
| SPI5_MOSI | SPI_MOSI| SPI Data Out |
| SPI5_MISO | SPI_MISO| SPI Data In |
| GPIO (CS) | SPI_CS  | Chip Select |
| GPIO      | SRDY    | Service Ready |
| GND       | GND     | Ground |

## Software Dependencies

- **STM32 HAL**: SPI, GPIO, system functions
- **FreeRTOS**: Task management, semaphores, heap
- **cifXToolkit**: Hilscher communication library
- **DeviceNet SDK**: Protocol stack implementation
- **Standard C Library**: string.h, stdint.h

## File Summary

### Created Files (Ready to Use)
- ✅ `App/DeviceNet/includes/AppDNS_DemoApplication.h` - DeviceNet API header
- ✅ `App/DeviceNet/includes/AppDNS_DeviceNetTask.h` - FreeRTOS task header
- ✅ `App/DeviceNet/Sources/AppDNS_DeviceNetTask.c` - FreeRTOS task implementation
- ✅ `App/cifXToolkit/OSAbstraction/OS_Custom.c` - FreeRTOS OS layer (COMPLETE)
- ✅ `App/cifXToolkit/OSAbstraction/OS_SPICustom.c` - SPI HAL layer (COMPLETE)

### Placeholder Files (To Be Replaced with SDK Files)
- ⏳ `App/DeviceNet/Sources/AppDNS_DemoApplication.c`
- ⏳ `App/DeviceNet/Sources/AppDNS_DemoApplicationFunctions.c`
- ⏳ `App/DeviceNet/Sources/AppDNS_ExplicitMsg.c`

### Directories for External Files
- 📁 `App/DeviceNet/includes/DNS_API/` - Copy DNS API headers here
- 📁 `App/cifXToolkit/Source/` - Copy cifX core sources here
- 📁 `App/cifXToolkit/SerialDPM/` - Copy SPI DPM implementation here

## Next Steps

1. ✅ **Directory structure created** - COMPLETE
2. ⏳ **Copy DeviceNet SDK files** - See `App/DeviceNet/includes/DNS_API/README.md`
3. ⏳ **Copy cifXToolkit files** - See `App/cifXToolkit/Source/README.md`
4. ⏳ **Configure STM32CubeMX** - SPI5 and GPIO setup
5. ⏳ **Update build system** - Add include/source paths
6. ⏳ **Test SPI communication** - Verify hardware connection
7. ⏳ **Initialize cifX** - Test device initialization
8. ⏳ **Test DeviceNet** - Verify network communication

## References

### Documentation
- `App/DeviceNet/README.md` - DeviceNet integration details
- `App/cifXToolkit/README.md` - cifXToolkit integration details
- `App/cifXToolkit/OSAbstraction/README.md` - OS abstraction guidelines

### External Resources
- Hilscher cifXToolkit API Reference
- DeviceNet Specification (ODVA)
- netX 90 User Manual
- STM32F429 Reference Manual

## Support and Troubleshooting

### Common Issues

1. **SPI Communication Failure**
   - Check GPIO pin assignments
   - Verify SPI clock configuration
   - Monitor SRDY signal with oscilloscope

2. **DeviceNet Network Timeout**
   - Verify netX 90 firmware is loaded
   - Check network configuration (MAC ID, baud rate)
   - Ensure proper network termination

3. **FreeRTOS Task Issues**
   - Increase heap size if allocation fails
   - Adjust task priority if timing issues occur
   - Check stack size (use stack overflow detection)

### Debug Tips

- Enable cifX debug output
- Use logic analyzer on SPI signals
- Monitor SRDY signal timing
- Check DPM structure alignment
- Verify netX firmware version compatibility

## Conclusion

This integration structure provides a clean, modular approach to adding DeviceNet support to the existing Dasan APC RTOS project while **preserving all existing application logic and model code**.

The key benefits of this structure:
- **Separation of concerns**: DeviceNet/cifX code isolated from application
- **Reusability**: OS abstraction allows easy porting
- **Maintainability**: Clear directory structure and documentation
- **Backwards compatibility**: Existing code remains unchanged
