# SerialDPM Directory

This directory contains the Serial DPM (Dual Port Memory) interface implementation for SPI communication.

## Files to Copy from cifXToolkit

Copy the following files from the cifXToolkit's SerialDPM directory:

### Core SerialDPM Files
- `cifXSPIDPMInterface.h` / `cifXSPIDPMInterface.c` - Main SPI DPM interface
- `NetX51_52_SPIDPMInterface.h` / `NetX51_52_SPIDPMInterface.c` - netX 51/52 specific SPI implementation
- `NetX90_SPIDPMInterface.h` / `NetX90_SPIDPMInterface.c` - netX 90 specific SPI implementation (USE THIS)

### Configuration
- Select the netX 90 implementation as it matches the netX 90 RevA chip on your hardware

## Source Location

These files should be copied from:
`cifXToolkit/SerialDPM/` directory in the Hilscher cifXToolkit library.

## Integration Notes

The SerialDPM layer provides:
- SPI protocol handling for DPM access
- Hardware handshaking (SRDY/BUSY signals)
- DPM read/write operations over SPI
- netX 90 specific timing and protocol

This layer must be adapted to use STM32 HAL SPI5 functions.
