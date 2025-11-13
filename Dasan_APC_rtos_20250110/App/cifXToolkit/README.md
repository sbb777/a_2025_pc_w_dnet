# cifXToolkit Library

This directory contains the Hilscher cifXToolkit library for interfacing with the netX 90 chip.

## Directory Structure

```
cifXToolkit/
├── Source/                          # Core cifX toolkit sources
│   └── README.md                   # Instructions for copying toolkit files
│
├── SerialDPM/                       # Serial DPM (SPI) interface
│   └── README.md                   # Instructions for SPI DPM files
│
└── OSAbstraction/                   # OS and hardware abstraction
    ├── README.md                   # Integration guidelines
    ├── OS_Custom.c                 # FreeRTOS adaptation
    └── OS_SPICustom.c              # STM32 HAL SPI5 adaptation
```

## Overview

The cifXToolkit provides a standardized API for accessing Hilscher netX chips. In this project, it enables communication with the netX 90 chip via SPI5 interface for DeviceNet protocol handling.

## Integration Steps

### 1. Copy cifXToolkit Core Files

Copy the core cifX files from the Hilscher cifXToolkit library to `Source/`:

See `Source/README.md` for the complete list of files to copy.

Key files include:
- `cifXToolkit.h` - Main API header
- `cifXFunctions.c` - Core API implementation
- `cifXHWFunctions.c` - Hardware abstraction
- `NetX_RegDefs.h` - netX register definitions
- DPM structure definitions

### 2. Copy SerialDPM Files

Copy the SPI DPM interface files to `SerialDPM/`:

See `SerialDPM/README.md` for details.

**Important**: Use the **netX 90** specific implementation:
- `NetX90_SPIDPMInterface.h`
- `NetX90_SPIDPMInterface.c`

### 3. OS Abstraction Layer

The OS abstraction layer has been implemented in `OSAbstraction/`:

#### OS_Custom.c (FreeRTOS Integration)
Provides FreeRTOS-specific implementations:
- **Mutex operations**: Using FreeRTOS semaphores
- **Memory management**: Using FreeRTOS heap
- **Time functions**: Using FreeRTOS tick counter
- **String operations**: Using standard C library

#### OS_SPICustom.c (STM32 HAL SPI5 Integration)
Provides STM32 hardware-specific implementations:
- **SPI transfer**: Using HAL_SPI_TransmitReceive()
- **GPIO control**: CS (Chip Select) signal
- **Handshaking**: SRDY (Service Ready) monitoring

### 4. STM32CubeMX SPI5 Configuration

Configure SPI5 in STM32CubeMX with the following parameters:

```
Mode: Master
Hardware NSS: Disable (software control)
Direction: Full Duplex Master
Data Size: 8 Bits
First Bit: MSB First

Clock Parameters:
  Prescaler: (adjust to achieve up to 20 MHz SPI clock)
  Clock Polarity (CPOL): Low
  Clock Phase (CPHA): 1 Edge
  CRC Calculation: Disabled
  NSS Signal Type: Software

GPIO Configuration:
  SPI5_SCK:  Clock output
  SPI5_MISO: Data input
  SPI5_MOSI: Data output
  CS:        GPIO output (manual control)
  SRDY:      GPIO input (with pull-up)
```

### 5. Hardware Connections

Verify the following connections between STM32F429 and netX 90:

| STM32F429 | netX 90 | Signal | Direction |
|-----------|---------|--------|-----------|
| SPI5_SCK  | SPI_CLK | Clock  | Out       |
| SPI5_MOSI | SPI_MOSI| Data   | Out       |
| SPI5_MISO | SPI_MISO| Data   | In        |
| GPIO (CS) | SPI_CS  | Chip Select | Out  |
| GPIO      | SRDY    | Service Ready | In  |
| GND       | GND     | Ground | -         |

**Important**: Verify voltage levels are compatible (3.3V logic).

### 6. GPIO Pin Assignment

Update the GPIO pin definitions in `OSAbstraction/OS_SPICustom.c`:

```c
/* TODO: Define GPIO pins for SPI5 handshaking */
#define NETX_CS_PIN         GPIO_PIN_6    /* Example: SPI5_CS on PF6 */
#define NETX_CS_PORT        GPIOF

#define NETX_SRDY_PIN       GPIO_PIN_7    /* Example: SRDY on PF7 */
#define NETX_SRDY_PORT      GPIOF
```

Replace with actual pin assignments from your hardware design.

## API Usage

### Initialize cifX Device

```c
#include "cifXToolkit.h"

int32_t lRet;
CIFXHANDLE hDriver = NULL;
CIFXHANDLE hDevice = NULL;

/* Initialize driver */
lRet = xDriverOpen(&hDriver);
if (lRet != CIFX_NO_ERROR)
{
    /* Handle error */
}

/* Open device */
lRet = xChannelOpen(hDriver, "cifX0", 0, &hDevice);
if (lRet != CIFX_NO_ERROR)
{
    /* Handle error */
}
```

### Send/Receive Packets

```c
CIFX_PACKET tSendPacket;
CIFX_PACKET tRecvPacket;
uint32_t ulRecvLen;

/* Prepare send packet */
/* ... fill tSendPacket ... */

/* Send packet and wait for response */
lRet = xChannelPutPacket(hDevice, &tSendPacket, 1000);
if (lRet == CIFX_NO_ERROR)
{
    lRet = xChannelGetPacket(hDevice, sizeof(tRecvPacket),
                             &tRecvPacket, &ulRecvLen, 1000);
}
```

### Access I/O Data

```c
uint8_t abInputData[32];
uint8_t abOutputData[32];
uint32_t ulBytesRead;

/* Read input data from DPM */
lRet = xChannelIORead(hDevice, 0, 0, sizeof(abInputData),
                      abInputData, &ulBytesRead, 100);

/* Write output data to DPM */
lRet = xChannelIOWrite(hDevice, 0, 0, sizeof(abOutputData),
                       abOutputData, 100);
```

## Timing Requirements

### SPI Communication
- **SPI Clock**: Up to 20 MHz (adjust based on cable length and noise)
- **CS Setup Time**: Minimum 10ns before first clock edge
- **CS Hold Time**: Minimum 10ns after last clock edge
- **SRDY Polling**: Check before each DPM access
- **Timeout**: Implement reasonable timeouts (typically 100-1000ms)

### DPM Access
- **Handshake Protocol**:
  1. Assert CS low
  2. Wait for SRDY high
  3. Perform SPI transfer
  4. Deassert CS high
  5. Wait for SRDY low

## Error Handling

Common errors and solutions:

| Error | Cause | Solution |
|-------|-------|----------|
| `CIFX_DEV_NOT_READY` | netX not initialized | Check netX firmware, verify SRDY signal |
| `CIFX_DEV_NO_COM_FLAG` | DPM communication flag missing | Verify DPM structure, check SPI timing |
| `CIFX_DEV_EXCHANGE_FAILED` | Data exchange timeout | Check SRDY handshaking, verify SPI transfer |
| `CIFX_INVALID_PARAMETER` | Invalid API parameter | Verify buffer sizes and pointers |

## Memory Usage

Typical memory requirements:
- **RAM**: ~16KB for DPM buffers and internal structures
- **Flash**: ~64KB for cifXToolkit library code
- **Stack**: 2-4KB per thread using cifX API

Adjust FreeRTOS heap size accordingly in `FreeRTOSConfig.h`:
```c
#define configTOTAL_HEAP_SIZE    ((size_t)(64 * 1024))  /* 64KB heap */
```

## Thread Safety

The cifXToolkit uses mutexes to ensure thread safety:
- Multiple tasks can safely access the same device
- Mutex timeout can be configured per API call
- Avoid calling cifX functions from ISRs

## Debugging

Enable debug output by defining:
```c
#define CIFX_TOOLKIT_DEBUG  1
```

This will output diagnostic messages through the configured debug interface.

## References

- cifXToolkit API Reference Manual
- netX 90 Dual-Port Memory Interface Manual
- netX 90 SPI DPM Application Note
- Hilscher Knowledge Base: https://kb.hilscher.com

## Notes

- The SerialDPM layer handles the low-level SPI protocol for DPM access
- SRDY signal is critical for proper handshaking - ensure it's correctly connected
- SPI clock rate should be adjusted based on PCB layout and cable length
- Use external pull-up resistor on SRDY if not using internal pull-up
- Consider adding EMI filtering on SPI lines in noisy industrial environments
