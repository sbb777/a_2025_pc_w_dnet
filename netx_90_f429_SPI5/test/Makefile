# Makefile for VAT Adaptive Pressure Controller DeviceNet Test
# Supports both standalone mode and cifX hardware mode

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS =

# Project name
PROJECT = vat_test

# Source files
SOURCES = vat_devicenet_test.c vat_test_main.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Header files
HEADERS = vat_devicenet_test.h

# Executable name
TARGET = $(PROJECT)

# cifX toolkit paths (modify these for your system)
CIFX_TOOLKIT_PATH = ../netx_tk
CIFX_INCLUDE = $(CIFX_TOOLKIT_PATH)/Includes
CIFX_LIB_PATH = $(CIFX_TOOLKIT_PATH)/Lib

# Default target: standalone mode (no hardware required)
all: standalone

# Standalone mode (mock cifX functions)
standalone: CFLAGS += -DENABLE_LOGGING
standalone: $(TARGET)_standalone

$(TARGET)_standalone: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@
	@echo ""
	@echo "====================================================="
	@echo " Built: $(TARGET)_standalone"
	@echo " Mode: Standalone (No hardware required)"
	@echo " Run with: ./$(TARGET)_standalone"
	@echo "====================================================="

# Hardware mode (requires cifX toolkit)
hardware: CFLAGS += -DUSE_CIFX_API -DENABLE_LOGGING -I$(CIFX_INCLUDE)
hardware: LDFLAGS += -L$(CIFX_LIB_PATH) -lcifx
hardware: $(TARGET)_hardware

$(TARGET)_hardware: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $@
	@echo ""
	@echo "====================================================="
	@echo " Built: $(TARGET)_hardware"
	@echo " Mode: Hardware (requires cifX device)"
	@echo " Run with: sudo ./$(TARGET)_hardware"
	@echo "====================================================="

# Windows build (MinGW)
windows: CC = x86_64-w64-mingw32-gcc
windows: TARGET = $(PROJECT).exe
windows: standalone

# Clean build artifacts
clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)_standalone $(TARGET)_hardware
	rm -f $(PROJECT).exe
	rm -f *.o
	@echo "Cleaned build artifacts"

# Help
help:
	@echo "VAT DeviceNet Test Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make                  - Build standalone version (default)"
	@echo "  make standalone       - Build standalone version (no hardware)"
	@echo "  make hardware         - Build hardware version (requires cifX)"
	@echo "  make windows          - Build Windows executable (MinGW)"
	@echo "  make clean            - Remove build artifacts"
	@echo "  make help             - Show this help"
	@echo ""
	@echo "Environment Variables:"
	@echo "  CIFX_TOOLKIT_PATH     - Path to cifX toolkit (default: ../netx_tk)"
	@echo ""
	@echo "Examples:"
	@echo "  make standalone       - Build and test without hardware"
	@echo "  make hardware         - Build for real cifX device"
	@echo "  CIFX_TOOLKIT_PATH=/opt/cifx make hardware"

.PHONY: all standalone hardware windows clean help
