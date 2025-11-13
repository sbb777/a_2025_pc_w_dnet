################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hil_cifXToolkit/OSAbstraction/OS_Custom_STM32.c \
../Hil_cifXToolkit/OSAbstraction/OS_SPICustom_STM32.c 

OBJS += \
./Hil_cifXToolkit/OSAbstraction/OS_Custom_STM32.o \
./Hil_cifXToolkit/OSAbstraction/OS_SPICustom_STM32.o 

C_DEPS += \
./Hil_cifXToolkit/OSAbstraction/OS_Custom_STM32.d \
./Hil_cifXToolkit/OSAbstraction/OS_SPICustom_STM32.d 


# Each subdirectory must supply rules for building sources it contributes
Hil_cifXToolkit/OSAbstraction/%.o Hil_cifXToolkit/OSAbstraction/%.su Hil_cifXToolkit/OSAbstraction/%.cyclo: ../Hil_cifXToolkit/OSAbstraction/%.c Hil_cifXToolkit/OSAbstraction/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -DCIFX_TOOLKIT_HWIF -c -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/Common/cifXAPI/includes" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/OSAbstraction" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/SerialDPM" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/Source" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/User" -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/DemoAppDNS" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/DNS_API" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/GenericAP_API" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/HilscherDefinitions" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoApp/Includes" -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hil_cifXToolkit-2f-OSAbstraction

clean-Hil_cifXToolkit-2f-OSAbstraction:
	-$(RM) ./Hil_cifXToolkit/OSAbstraction/OS_Custom_STM32.cyclo ./Hil_cifXToolkit/OSAbstraction/OS_Custom_STM32.d ./Hil_cifXToolkit/OSAbstraction/OS_Custom_STM32.o ./Hil_cifXToolkit/OSAbstraction/OS_Custom_STM32.su ./Hil_cifXToolkit/OSAbstraction/OS_SPICustom_STM32.cyclo ./Hil_cifXToolkit/OSAbstraction/OS_SPICustom_STM32.d ./Hil_cifXToolkit/OSAbstraction/OS_SPICustom_STM32.o ./Hil_cifXToolkit/OSAbstraction/OS_SPICustom_STM32.su

.PHONY: clean-Hil_cifXToolkit-2f-OSAbstraction

