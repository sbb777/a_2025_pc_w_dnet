################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../netx_tk/Source/Hilcrc32.c \
../netx_tk/Source/Hilmd5.c \
../netx_tk/Source/cifXDownload.c \
../netx_tk/Source/cifXEndianess.c \
../netx_tk/Source/cifXFunctions.c \
../netx_tk/Source/cifXHWFunctions.c \
../netx_tk/Source/cifXInit.c \
../netx_tk/Source/cifXInterrupt.c \
../netx_tk/Source/netX5x_hboot.c \
../netx_tk/Source/netX5xx_hboot.c \
../netx_tk/Source/netX90_netX4x00.c 

OBJS += \
./netx_tk/Source/Hilcrc32.o \
./netx_tk/Source/Hilmd5.o \
./netx_tk/Source/cifXDownload.o \
./netx_tk/Source/cifXEndianess.o \
./netx_tk/Source/cifXFunctions.o \
./netx_tk/Source/cifXHWFunctions.o \
./netx_tk/Source/cifXInit.o \
./netx_tk/Source/cifXInterrupt.o \
./netx_tk/Source/netX5x_hboot.o \
./netx_tk/Source/netX5xx_hboot.o \
./netx_tk/Source/netX90_netX4x00.o 

C_DEPS += \
./netx_tk/Source/Hilcrc32.d \
./netx_tk/Source/Hilmd5.d \
./netx_tk/Source/cifXDownload.d \
./netx_tk/Source/cifXEndianess.d \
./netx_tk/Source/cifXFunctions.d \
./netx_tk/Source/cifXHWFunctions.d \
./netx_tk/Source/cifXInit.d \
./netx_tk/Source/cifXInterrupt.d \
./netx_tk/Source/netX5x_hboot.d \
./netx_tk/Source/netX5xx_hboot.d \
./netx_tk/Source/netX90_netX4x00.d 


# Each subdirectory must supply rules for building sources it contributes
netx_tk/Source/%.o netx_tk/Source/%.su netx_tk/Source/%.cyclo: ../netx_tk/Source/%.c netx_tk/Source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -DCIFX_TOOLKIT_HWIF -c -I../Core/Inc -I../USB_HOST/App -I../USB_HOST/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/git/netx_90_f429_SPI5/netx_tk" -I"D:/git/netx_90_f429_SPI5/netx_tk/BSL" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common" -I"D:/git/netx_90_f429_SPI5/netx_tk/Source" -I"D:/git/netx_90_f429_SPI5/netx_tk/User" -I"D:/git/netx_90_f429_SPI5/netx_tk/OSAbstraction" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common/cifXAPI" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common/HilscherDefinitions" -I"D:/git/netx_90_f429_SPI5/netx_tk/SerialDPM" -I"D:/git/netx_90_f429_SPI5/Hil_DemoApp/Includes" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DemoAppDNS" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DNS_API" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/GenericAP_API" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/HilscherDefinitions" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes" -I"D:/git/netx_90_f429_SPI5/Hil_DemoApp" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-netx_tk-2f-Source

clean-netx_tk-2f-Source:
	-$(RM) ./netx_tk/Source/Hilcrc32.cyclo ./netx_tk/Source/Hilcrc32.d ./netx_tk/Source/Hilcrc32.o ./netx_tk/Source/Hilcrc32.su ./netx_tk/Source/Hilmd5.cyclo ./netx_tk/Source/Hilmd5.d ./netx_tk/Source/Hilmd5.o ./netx_tk/Source/Hilmd5.su ./netx_tk/Source/cifXDownload.cyclo ./netx_tk/Source/cifXDownload.d ./netx_tk/Source/cifXDownload.o ./netx_tk/Source/cifXDownload.su ./netx_tk/Source/cifXEndianess.cyclo ./netx_tk/Source/cifXEndianess.d ./netx_tk/Source/cifXEndianess.o ./netx_tk/Source/cifXEndianess.su ./netx_tk/Source/cifXFunctions.cyclo ./netx_tk/Source/cifXFunctions.d ./netx_tk/Source/cifXFunctions.o ./netx_tk/Source/cifXFunctions.su ./netx_tk/Source/cifXHWFunctions.cyclo ./netx_tk/Source/cifXHWFunctions.d ./netx_tk/Source/cifXHWFunctions.o ./netx_tk/Source/cifXHWFunctions.su ./netx_tk/Source/cifXInit.cyclo ./netx_tk/Source/cifXInit.d ./netx_tk/Source/cifXInit.o ./netx_tk/Source/cifXInit.su ./netx_tk/Source/cifXInterrupt.cyclo ./netx_tk/Source/cifXInterrupt.d ./netx_tk/Source/cifXInterrupt.o ./netx_tk/Source/cifXInterrupt.su ./netx_tk/Source/netX5x_hboot.cyclo ./netx_tk/Source/netX5x_hboot.d ./netx_tk/Source/netX5x_hboot.o ./netx_tk/Source/netX5x_hboot.su ./netx_tk/Source/netX5xx_hboot.cyclo ./netx_tk/Source/netX5xx_hboot.d ./netx_tk/Source/netX5xx_hboot.o ./netx_tk/Source/netX5xx_hboot.su ./netx_tk/Source/netX90_netX4x00.cyclo ./netx_tk/Source/netX90_netX4x00.d ./netx_tk/Source/netX90_netX4x00.o ./netx_tk/Source/netX90_netX4x00.su

.PHONY: clean-netx_tk-2f-Source

