################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.c \
../Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c 

OBJS += \
./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.o \
./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.o 

C_DEPS += \
./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.d \
./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.d 


# Each subdirectory must supply rules for building sources it contributes
Hil_DemoAppDNS/Sources/%.o Hil_DemoAppDNS/Sources/%.su Hil_DemoAppDNS/Sources/%.cyclo: ../Hil_DemoAppDNS/Sources/%.c Hil_DemoAppDNS/Sources/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -DCIFX_TOOLKIT_HWIF -c -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/Common/cifXAPI/includes" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/OSAbstraction" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/SerialDPM" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/Source" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_cifXToolkit/User" -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/DemoAppDNS" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/DNS_API" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/GenericAP_API" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoAppDNS/includes/HilscherDefinitions" -I"D:/git/a_pcb/hilscher_dnet/F7_netX_DNSV5_SimpleConfig/F7_netX_DNSV5_SimpleConfig/Hil_DemoApp/Includes" -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hil_DemoAppDNS-2f-Sources

clean-Hil_DemoAppDNS-2f-Sources:
	-$(RM) ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.cyclo ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.d ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.o ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.su ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.cyclo ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.d ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.o ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.su

.PHONY: clean-Hil_DemoAppDNS-2f-Sources

