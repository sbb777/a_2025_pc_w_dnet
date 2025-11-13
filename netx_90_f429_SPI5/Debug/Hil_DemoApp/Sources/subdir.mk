################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hil_DemoApp/Sources/App_DemoApplication.c \
../Hil_DemoApp/Sources/App_PacketCommunication.c \
../Hil_DemoApp/Sources/App_SystemPackets.c \
../Hil_DemoApp/Sources/App_VAT_AssemblySelector.c \
../Hil_DemoApp/Sources/App_VAT_Conversion.c \
../Hil_DemoApp/Sources/App_VAT_Diagnostic.c \
../Hil_DemoApp/Sources/App_VAT_ExplicitHandler.c \
../Hil_DemoApp/Sources/App_VAT_Flash.c \
../Hil_DemoApp/Sources/App_VAT_IoHandler.c \
../Hil_DemoApp/Sources/App_VAT_Parameters.c \
../Hil_DemoApp/Sources/App_VAT_UserObject.c \
../Hil_DemoApp/Sources/devicenet_master_info.c \
../Hil_DemoApp/Sources/mmio_control.c \
../Hil_DemoApp/Sources/vat_devicenet_test.c 

OBJS += \
./Hil_DemoApp/Sources/App_DemoApplication.o \
./Hil_DemoApp/Sources/App_PacketCommunication.o \
./Hil_DemoApp/Sources/App_SystemPackets.o \
./Hil_DemoApp/Sources/App_VAT_AssemblySelector.o \
./Hil_DemoApp/Sources/App_VAT_Conversion.o \
./Hil_DemoApp/Sources/App_VAT_Diagnostic.o \
./Hil_DemoApp/Sources/App_VAT_ExplicitHandler.o \
./Hil_DemoApp/Sources/App_VAT_Flash.o \
./Hil_DemoApp/Sources/App_VAT_IoHandler.o \
./Hil_DemoApp/Sources/App_VAT_Parameters.o \
./Hil_DemoApp/Sources/App_VAT_UserObject.o \
./Hil_DemoApp/Sources/devicenet_master_info.o \
./Hil_DemoApp/Sources/mmio_control.o \
./Hil_DemoApp/Sources/vat_devicenet_test.o 

C_DEPS += \
./Hil_DemoApp/Sources/App_DemoApplication.d \
./Hil_DemoApp/Sources/App_PacketCommunication.d \
./Hil_DemoApp/Sources/App_SystemPackets.d \
./Hil_DemoApp/Sources/App_VAT_AssemblySelector.d \
./Hil_DemoApp/Sources/App_VAT_Conversion.d \
./Hil_DemoApp/Sources/App_VAT_Diagnostic.d \
./Hil_DemoApp/Sources/App_VAT_ExplicitHandler.d \
./Hil_DemoApp/Sources/App_VAT_Flash.d \
./Hil_DemoApp/Sources/App_VAT_IoHandler.d \
./Hil_DemoApp/Sources/App_VAT_Parameters.d \
./Hil_DemoApp/Sources/App_VAT_UserObject.d \
./Hil_DemoApp/Sources/devicenet_master_info.d \
./Hil_DemoApp/Sources/mmio_control.d \
./Hil_DemoApp/Sources/vat_devicenet_test.d 


# Each subdirectory must supply rules for building sources it contributes
Hil_DemoApp/Sources/%.o Hil_DemoApp/Sources/%.su Hil_DemoApp/Sources/%.cyclo: ../Hil_DemoApp/Sources/%.c Hil_DemoApp/Sources/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -DCIFX_TOOLKIT_HWIF -c -I../Core/Inc -I../USB_HOST/App -I../USB_HOST/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/git/netx_90_f429_SPI5/netx_tk" -I"D:/git/netx_90_f429_SPI5/netx_tk/BSL" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common" -I"D:/git/netx_90_f429_SPI5/netx_tk/Source" -I"D:/git/netx_90_f429_SPI5/netx_tk/User" -I"D:/git/netx_90_f429_SPI5/netx_tk/OSAbstraction" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common/cifXAPI" -I"D:/git/netx_90_f429_SPI5/netx_tk/Common/HilscherDefinitions" -I"D:/git/netx_90_f429_SPI5/netx_tk/SerialDPM" -I"D:/git/netx_90_f429_SPI5/Hil_DemoApp/Includes" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DemoAppDNS" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DNS_API" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/GenericAP_API" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes/HilscherDefinitions" -I"D:/git/netx_90_f429_SPI5/Hil_DemoAppDNS/includes" -I"D:/git/netx_90_f429_SPI5/Hil_DemoApp" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hil_DemoApp-2f-Sources

clean-Hil_DemoApp-2f-Sources:
	-$(RM) ./Hil_DemoApp/Sources/App_DemoApplication.cyclo ./Hil_DemoApp/Sources/App_DemoApplication.d ./Hil_DemoApp/Sources/App_DemoApplication.o ./Hil_DemoApp/Sources/App_DemoApplication.su ./Hil_DemoApp/Sources/App_PacketCommunication.cyclo ./Hil_DemoApp/Sources/App_PacketCommunication.d ./Hil_DemoApp/Sources/App_PacketCommunication.o ./Hil_DemoApp/Sources/App_PacketCommunication.su ./Hil_DemoApp/Sources/App_SystemPackets.cyclo ./Hil_DemoApp/Sources/App_SystemPackets.d ./Hil_DemoApp/Sources/App_SystemPackets.o ./Hil_DemoApp/Sources/App_SystemPackets.su ./Hil_DemoApp/Sources/App_VAT_AssemblySelector.cyclo ./Hil_DemoApp/Sources/App_VAT_AssemblySelector.d ./Hil_DemoApp/Sources/App_VAT_AssemblySelector.o ./Hil_DemoApp/Sources/App_VAT_AssemblySelector.su ./Hil_DemoApp/Sources/App_VAT_Conversion.cyclo ./Hil_DemoApp/Sources/App_VAT_Conversion.d ./Hil_DemoApp/Sources/App_VAT_Conversion.o ./Hil_DemoApp/Sources/App_VAT_Conversion.su ./Hil_DemoApp/Sources/App_VAT_Diagnostic.cyclo ./Hil_DemoApp/Sources/App_VAT_Diagnostic.d ./Hil_DemoApp/Sources/App_VAT_Diagnostic.o ./Hil_DemoApp/Sources/App_VAT_Diagnostic.su ./Hil_DemoApp/Sources/App_VAT_ExplicitHandler.cyclo ./Hil_DemoApp/Sources/App_VAT_ExplicitHandler.d ./Hil_DemoApp/Sources/App_VAT_ExplicitHandler.o ./Hil_DemoApp/Sources/App_VAT_ExplicitHandler.su ./Hil_DemoApp/Sources/App_VAT_Flash.cyclo ./Hil_DemoApp/Sources/App_VAT_Flash.d ./Hil_DemoApp/Sources/App_VAT_Flash.o ./Hil_DemoApp/Sources/App_VAT_Flash.su ./Hil_DemoApp/Sources/App_VAT_IoHandler.cyclo ./Hil_DemoApp/Sources/App_VAT_IoHandler.d ./Hil_DemoApp/Sources/App_VAT_IoHandler.o ./Hil_DemoApp/Sources/App_VAT_IoHandler.su ./Hil_DemoApp/Sources/App_VAT_Parameters.cyclo ./Hil_DemoApp/Sources/App_VAT_Parameters.d ./Hil_DemoApp/Sources/App_VAT_Parameters.o ./Hil_DemoApp/Sources/App_VAT_Parameters.su ./Hil_DemoApp/Sources/App_VAT_UserObject.cyclo ./Hil_DemoApp/Sources/App_VAT_UserObject.d ./Hil_DemoApp/Sources/App_VAT_UserObject.o ./Hil_DemoApp/Sources/App_VAT_UserObject.su ./Hil_DemoApp/Sources/devicenet_master_info.cyclo ./Hil_DemoApp/Sources/devicenet_master_info.d ./Hil_DemoApp/Sources/devicenet_master_info.o ./Hil_DemoApp/Sources/devicenet_master_info.su ./Hil_DemoApp/Sources/mmio_control.cyclo ./Hil_DemoApp/Sources/mmio_control.d ./Hil_DemoApp/Sources/mmio_control.o ./Hil_DemoApp/Sources/mmio_control.su ./Hil_DemoApp/Sources/vat_devicenet_test.cyclo ./Hil_DemoApp/Sources/vat_devicenet_test.d ./Hil_DemoApp/Sources/vat_devicenet_test.o ./Hil_DemoApp/Sources/vat_devicenet_test.su

.PHONY: clean-Hil_DemoApp-2f-Sources

