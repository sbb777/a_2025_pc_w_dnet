################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.c \
../Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c \
../Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.c 

OBJS += \
./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.o \
./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.o \
./Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.o 

C_DEPS += \
./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.d \
./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.d \
./Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.d 


# Each subdirectory must supply rules for building sources it contributes
Hil_DemoAppDNS/Sources/%.o Hil_DemoAppDNS/Sources/%.su Hil_DemoAppDNS/Sources/%.cyclo: ../Hil_DemoAppDNS/Sources/%.c Hil_DemoAppDNS/Sources/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -DCIFX_TOOLKIT_HWIF -c -I../Core/Inc -I../USB_HOST/App -I../USB_HOST/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/git/netx_90_f429_SPI5/netx_tk" -I"D:/git/netx_90_f429_SPI5/netx_tk/BSL" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common" -I"D:/git/netx_90_f429_SPI5/netx_tk/Source" -I"D:/git/netx_90_f429_SPI5/netx_tk/User" -I"D:/git/netx_90_f429_SPI5/netx_tk/OSAbstraction" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common/cifXAPI" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common/HilscherDefinitions" -I"D:/git/netx_90_f429_SPI5/netx_tk/SerialDPM" -I"D:/git/netx_90_f429_SPI5/Hil_DemoApp/Includes" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DemoAppDNS" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DNS_API" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/GenericAP_API" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/HilscherDefinitions" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes" -I"D:/git/netx_90_f429_SPI5/Hil_DemoApp" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hil_DemoAppDNS-2f-Sources

clean-Hil_DemoAppDNS-2f-Sources:
	-$(RM) ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.cyclo ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.d ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.o ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.su ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.cyclo ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.d ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.o ./Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.su ./Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.cyclo ./Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.d ./Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.o ./Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.su

.PHONY: clean-Hil_DemoAppDNS-2f-Sources

