

========================================
 STM32F429 + netX90 System Starting
========================================
UART5: 115200 baud, 8N1
Date: 2025-10-28
========================================


========================================
 VAT DeviceNet Test Initialization
========================================
VAT Test Configuration:
  Mode: 4 (1=Basic, 2=Calibration, 3=Monitoring)
  Node Address: 10
  Baud Rate: 250000 bps
  Input Assembly: 0x64 (8 bytes)
  Output Assembly: 0x08 (6 bytes)
========================================


========================================
 DeviceNet Stack Initialization
========================================
Step 1: Calling AppDNS_ConfigureStack()...

*** NEW CODE EXECUTING - VERSION 2025-11-06 14:30 ***
*** If you see this, new firmware is loaded! ***

  --> Calling SetConfiguration()...
  [OK] SetConfiguration() completed
  --> Calling ChannelInit()...
  [OK] ChannelInit() completed
  --> Calling StartCommunication()...
  [OK] StartCommunication() completed

*** Waiting for channel to be ready before registering classes...
*** Channel ready! (waited 0 ms)
    COS Flags: 0x00000007
    VAT Parameter Manager initialized with 99 parameters

=== Registering VAT CIP Classes ===
    [FAIL] Failed to register Class 0x01, error: 0xC0000009
    [FAIL] Failed to register VAT CIP classes!
Step 1: ? AppDNS_ConfigureStack() FAILED
  Error Code: 0xC0000009

DeviceNet Stack initialization failed!
Master scan will NOT detect this slave.
========================================

Waiting for channel ready...
  Expected: HIL_COMM_COS_READY (0x00000001)
  [0 s] COS Flags: 0x00000007

*** Skipping VAT test due to channel not ready ***

*** VAT Test completed - keeping channel open for continuous operation ***

========================================
 VAT Test Statistics
========================================

========== VAT Test Statistics ==========
Total Read Operations:   0
Total Write Operations:  0
Read Errors:             0
Write Errors:            0
Exception Count:         0
Timeout Count:           0
Last Error Code:         0x00000000
=========================================

Communication channel remains active.
========================================
