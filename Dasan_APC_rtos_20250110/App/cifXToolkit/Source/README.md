# cifXToolkit Source Directory

This directory should contain the core cifX toolkit source files.

## Files to Copy from cifXToolkit

Copy the following files from the cifXToolkit library:

### Core cifX Files
- `cifXEndianess.h` / `cifXEndianess.c` - Endianness handling
- `cifXErrors.h` - Error code definitions
- `cifXFunctions.h` / `cifXFunctions.c` - Main cifX API functions
- `cifXHWFunctions.h` / `cifXHWFunctions.c` - Hardware abstraction functions
- `cifXInit.c` - Initialization functions
- `cifXInterrupt.h` / `cifXInterrupt.c` - Interrupt handling
- `cifXToolkit.h` - Main toolkit header
- `OS_Dependent.h` - OS-dependent definitions

### netX DPM Files
- `NetX_RegDefs.h` - netX register definitions
- `Hil_ApplicationCmd.h` - Application command interface
- `Hil_Compiler.h` - Compiler-specific definitions
- `Hil_DualPortMemory.h` - DPM structure definitions
- `Hil_Packet.h` - Packet handling definitions
- `Hil_Results.h` - Result code definitions
- `Hil_SystemCmd.h` - System command definitions

## Source Location

These files should be copied from the Hilscher cifXToolkit library.

## Integration Notes

These files provide the low-level cifX protocol and hardware interface.
They interact with the netX 90 chip through the DPM (Dual Port Memory).
