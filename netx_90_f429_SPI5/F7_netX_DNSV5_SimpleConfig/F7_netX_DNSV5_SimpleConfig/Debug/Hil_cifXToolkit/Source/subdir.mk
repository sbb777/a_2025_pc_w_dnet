################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hil_cifXToolkit/Source/Hilcrc32.c \
../Hil_cifXToolkit/Source/Hilmd5.c \
../Hil_cifXToolkit/Source/cifXDownload.c \
../Hil_cifXToolkit/Source/cifXEndianess.c \
../Hil_cifXToolkit/Source/cifXFunctions.c \
../Hil_cifXToolkit/Source/cifXHWFunctions.c \
../Hil_cifXToolkit/Source/cifXInit.c \
../Hil_cifXToolkit/Source/cifXInterrupt.c \
../Hil_cifXToolkit/Source/netX5x_hboot.c \
../Hil_cifXToolkit/Source/netX5xx_hboot.c \
../Hil_cifXToolkit/Source/netX90_netX4x00.c 

OBJS += \
./Hil_cifXToolkit/Source/Hilcrc32.o \
./Hil_cifXToolkit/Source/Hilmd5.o \
./Hil_cifXToolkit/Source/cifXDownload.o \
./Hil_cifXToolkit/Source/cifXEndianess.o \
./Hil_cifXToolkit/Source/cifXFunctions.o \
./Hil_cifXToolkit/Source/cifXHWFunctions.o \
./Hil_cifXToolkit/Source/cifXInit.o \
./Hil_cifXToolkit/Source/cifXInterrupt.o \
./Hil_cifXToolkit/Source/netX5x_hboot.o \
./Hil_cifXToolkit/Source/netX5xx_hboot.o \
./Hil_cifXToolkit/Source/netX90_netX4x00.o 

C_DEPS += \
./Hil_cifXToolkit/Source/Hilcrc32.d \
./Hil_cifXToolkit/Source/Hilmd5.d \
./Hil_cifXToolkit/Source/cifXDownload.d \
./Hil_cifXToolkit/Source/cifXEndianess.d \
./Hil_cifXToolkit/Source/cifXFunctions.d \
./Hil_cifXToolkit/Source/cifXHWFunctions.d \
./Hil_cifXToolkit/Source/cifXInit.d \
./Hil_cifXToolkit/Source/cifXInterrupt.d \
./Hil_cifXToolkit/Source/netX5x_hboot.d \
./Hil_cifXToolkit/Source/netX5xx_hboot.d \
./Hil_cifXToolkit/Source/netX90_netX4x00.d 


# Each subdirectory must supply rules for building sources it contributes
Hil_cifXToolkit/Source/%.o Hil_cifXToolkit/Source/%.su Hil_cifXToolkit/Source/%.cyclo: ../Hil_cifXToolkit/Source/%.c Hil_cifXToolkit/Source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -DCIFX_TOOLKIT_HWIF -c -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/Common/cifXAPI/includes" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/OSAbstraction" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/SerialDPM" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/Source" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/User" -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/DemoAppDNS" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/DNS_API" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/GenericAP_API" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/HilscherDefinitions" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoApp/Includes" -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hil_cifXToolkit-2f-Source

clean-Hil_cifXToolkit-2f-Source:
	-$(RM) ./Hil_cifXToolkit/Source/Hilcrc32.cyclo ./Hil_cifXToolkit/Source/Hilcrc32.d ./Hil_cifXToolkit/Source/Hilcrc32.o ./Hil_cifXToolkit/Source/Hilcrc32.su ./Hil_cifXToolkit/Source/Hilmd5.cyclo ./Hil_cifXToolkit/Source/Hilmd5.d ./Hil_cifXToolkit/Source/Hilmd5.o ./Hil_cifXToolkit/Source/Hilmd5.su ./Hil_cifXToolkit/Source/cifXDownload.cyclo ./Hil_cifXToolkit/Source/cifXDownload.d ./Hil_cifXToolkit/Source/cifXDownload.o ./Hil_cifXToolkit/Source/cifXDownload.su ./Hil_cifXToolkit/Source/cifXEndianess.cyclo ./Hil_cifXToolkit/Source/cifXEndianess.d ./Hil_cifXToolkit/Source/cifXEndianess.o ./Hil_cifXToolkit/Source/cifXEndianess.su ./Hil_cifXToolkit/Source/cifXFunctions.cyclo ./Hil_cifXToolkit/Source/cifXFunctions.d ./Hil_cifXToolkit/Source/cifXFunctions.o ./Hil_cifXToolkit/Source/cifXFunctions.su ./Hil_cifXToolkit/Source/cifXHWFunctions.cyclo ./Hil_cifXToolkit/Source/cifXHWFunctions.d ./Hil_cifXToolkit/Source/cifXHWFunctions.o ./Hil_cifXToolkit/Source/cifXHWFunctions.su ./Hil_cifXToolkit/Source/cifXInit.cyclo ./Hil_cifXToolkit/Source/cifXInit.d ./Hil_cifXToolkit/Source/cifXInit.o ./Hil_cifXToolkit/Source/cifXInit.su ./Hil_cifXToolkit/Source/cifXInterrupt.cyclo ./Hil_cifXToolkit/Source/cifXInterrupt.d ./Hil_cifXToolkit/Source/cifXInterrupt.o ./Hil_cifXToolkit/Source/cifXInterrupt.su ./Hil_cifXToolkit/Source/netX5x_hboot.cyclo ./Hil_cifXToolkit/Source/netX5x_hboot.d ./Hil_cifXToolkit/Source/netX5x_hboot.o ./Hil_cifXToolkit/Source/netX5x_hboot.su ./Hil_cifXToolkit/Source/netX5xx_hboot.cyclo ./Hil_cifXToolkit/Source/netX5xx_hboot.d ./Hil_cifXToolkit/Source/netX5xx_hboot.o ./Hil_cifXToolkit/Source/netX5xx_hboot.su ./Hil_cifXToolkit/Source/netX90_netX4x00.cyclo ./Hil_cifXToolkit/Source/netX90_netX4x00.d ./Hil_cifXToolkit/Source/netX90_netX4x00.o ./Hil_cifXToolkit/Source/netX90_netX4x00.su

.PHONY: clean-Hil_cifXToolkit-2f-Source

