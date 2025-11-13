# VAT DeviceNet Communication Test

Complete DeviceNet communication test suite for **VAT Adaptive Pressure Controller** (Product Code 650).

## Quick Start

### Without Hardware (Standalone Mode)

```bash
cd test
make standalone
./vat_test_standalone
```

### With cifX Hardware

```bash
cd test
make hardware
sudo ./vat_test_hardware
```

## Features

✅ **24 Input Assemblies** - Complete input data structures (2-34 bytes)
✅ **11 Output Assemblies** - Full control capabilities (4-18 bytes)
✅ **6 Control Modes** - CLOSED, OPEN, PRESSURE, POSITION, MANUAL, THROTTLE
✅ **Exception Handling** - 8 exception status flags with detailed logging
✅ **Statistics Tracking** - Read/write counts, errors, timeouts
✅ **Test Scenarios** - Pre-built test functions for common operations
✅ **Mock Support** - Test without hardware using standalone mode

## File Structure

```
test/
├── vat_devicenet_test.h           # Header (data structures, API)
├── vat_devicenet_test.c           # Implementation (test functions)
├── vat_test_main.c                # Main application
├── Makefile                       # Build script
├── README.md                      # This file
└── 20251027_VAT_DeviceNet_Test_Guide.md  # Detailed documentation
```

## Recommended Assembly Combination

| Direction | Assembly | Size | Description |
|-----------|----------|------|-------------|
| **Input** | Assembly 100 (0x64) | 7 bytes | Full status (Exception, Pressure, Position, Status, Access) |
| **Output** | Assembly 8 (0x08) | 5 bytes | Control with mode (Mode, Setpoint, Instance) |

## Control Modes

```c
VAT_CONTROL_MODE_CLOSED    = 0x00  // Valve fully closed
VAT_CONTROL_MODE_OPEN      = 0x01  // Valve fully open
VAT_CONTROL_MODE_PRESSURE  = 0x02  // Pressure control (PID)
VAT_CONTROL_MODE_POSITION  = 0x03  // Position control
VAT_CONTROL_MODE_MANUAL    = 0x04  // Manual control
VAT_CONTROL_MODE_THROTTLE  = 0x05  // Throttle control
```

## Usage Example

```c
#include "vat_devicenet_test.h"

// Initialize
VAT_TEST_CONTEXT_T tContext;
VAT_Test_Init(&tContext);

// Configure
VAT_TEST_CONFIG_T tConfig = {
    .node_address = 10,
    .baud_rate = 250000,
    .epr_ms = 100,
    .input_assembly = VAT_INPUT_ASSEMBLY_100,
    .output_assembly = VAT_OUTPUT_ASSEMBLY_8,
    .enable_logging = true
};
VAT_Test_Configure(&tContext, &tConfig);

// Set control mode
VAT_Test_SetControlMode(&tContext, VAT_CONTROL_MODE_PRESSURE);
VAT_Test_SetPressureSetpoint(&tContext, 500);

// Communication loop
for (int i = 0; i < 100; i++) {
    VAT_Test_WriteOutput(&tContext, hChannel);
    VAT_Test_ReadInput(&tContext, hChannel);

    if (VAT_Test_HasException(&tContext)) {
        printf("Exception: 0x%02X\n",
               VAT_Test_GetExceptionStatus(&tContext));
    }
}

// Print statistics
VAT_Test_PrintStats(&tContext);
```

## Build Options

```bash
# Standalone mode (no hardware)
make standalone

# Hardware mode (requires cifX)
make hardware

# Windows build (MinGW)
make windows

# Custom cifX path
make hardware CIFX_TOOLKIT_PATH=/path/to/netx_tk

# Clean
make clean
```

## Documentation

📖 **Full Guide**: [20251027_VAT_DeviceNet_Test_Guide.md](./20251027_VAT_DeviceNet_Test_Guide.md)

Includes:
- Complete EDS file analysis (476297.eds)
- All 24 input and 11 output assemblies
- API reference
- Integration guide
- Troubleshooting
- Advanced usage examples

## Device Information

| Field | Value |
|-------|-------|
| **Manufacturer** | VAT Vakuumventile AG |
| **Vendor Code** | 404 |
| **Product Code** | 650 |
| **Device Type** | Process Control Valve (Type 29) |
| **Protocol** | DeviceNet / CIP |
| **Firmware** | v2.1 |

## Requirements

### Standalone Mode
- GCC compiler
- Standard C library

### Hardware Mode
- cifX Toolkit (netx_tk)
- cifX device (netX-based)
- DeviceNet connection to VAT controller
- Linux: root privileges for device access

## Troubleshooting

### Compilation Error: "cifXUser.h not found"
```bash
# Use standalone mode instead
make standalone

# Or specify cifX path
make hardware CIFX_TOOLKIT_PATH=/correct/path
```

### Permission Denied
```bash
# Add execute permission
chmod +x vat_test_hardware

# Run with sudo
sudo ./vat_test_hardware
```

### Communication Timeout
- Check node address (DIP switch on VAT device)
- Verify baud rate (125k/250k/500k)
- Check cable connections
- Verify termination resistors (120Ω)

## License

Demo software provided by Hilscher Gesellschaft fuer Systemautomation mbH.
See README_DISCLAIMER.txt for full license terms.

## Support

For issues or questions:
1. Check the detailed guide: `20251027_VAT_DeviceNet_Test_Guide.md`
2. Review EDS file: `476297.eds`
3. Contact VAT support: https://www.vatvalve.com
4. Hilscher support: https://www.hilscher.com/support

---

**Generated**: 2025-10-27
**EDS File**: 476297.eds
**Version**: 1.0
