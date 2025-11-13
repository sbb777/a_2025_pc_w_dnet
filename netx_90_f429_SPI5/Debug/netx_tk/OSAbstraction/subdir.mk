################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../netx_tk/OSAbstraction/OS_Custom.c \
../netx_tk/OSAbstraction/OS_SPICustom.c 

OBJS += \
./netx_tk/OSAbstraction/OS_Custom.o \
./netx_tk/OSAbstraction/OS_SPICustom.o 

C_DEPS += \
./netx_tk/OSAbstraction/OS_Custom.d \
./netx_tk/OSAbstraction/OS_SPICustom.d 


# Each subdirectory must supply rules for building sources it contributes
netx_tk/OSAbstraction/%.o netx_tk/OSAbstraction/%.su netx_tk/OSAbstraction/%.cyclo: ../netx_tk/OSAbstraction/%.c netx_tk/OSAbstraction/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -DCIFX_TOOLKIT_HWIF -c -I../Core/Inc -I../USB_HOST/App -I../USB_HOST/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/git/netx_90_f429_SPI5/netx_tk" -I"D:/git/netx_90_f429_SPI5/netx_tk/BSL" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common" -I"D:/git/netx_90_f429_SPI5/netx_tk/Source" -I"D:/git/netx_90_f429_SPI5/netx_tk/User" -I"D:/git/netx_90_f429_SPI5/netx_tk/OSAbstraction" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common/cifXAPI" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common/HilscherDefinitions" -I"D:/git/netx_90_f429_SPI5/netx_tk/SerialDPM" -I"D:/git/netx_90_f429_SPI5/Hil_DemoApp/Includes" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DemoAppDNS" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DNS_API" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/GenericAP_API" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/HilscherDefinitions" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes" -I"D:/git/netx_90_f429_SPI5/Hil_DemoApp" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-netx_tk-2f-OSAbstraction

clean-netx_tk-2f-OSAbstraction:
	-$(RM) ./netx_tk/OSAbstraction/OS_Custom.cyclo ./netx_tk/OSAbstraction/OS_Custom.d ./netx_tk/OSAbstraction/OS_Custom.o ./netx_tk/OSAbstraction/OS_Custom.su ./netx_tk/OSAbstraction/OS_SPICustom.cyclo ./netx_tk/OSAbstraction/OS_SPICustom.d ./netx_tk/OSAbstraction/OS_SPICustom.o ./netx_tk/OSAbstraction/OS_SPICustom.su

.PHONY: clean-netx_tk-2f-OSAbstraction

