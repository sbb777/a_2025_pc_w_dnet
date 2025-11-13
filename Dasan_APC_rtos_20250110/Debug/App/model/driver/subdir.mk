################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/model/driver/APC_ADS1252.c \
../App/model/driver/APC_ChipSelect.c \
../App/model/driver/APC_DAC7612.c \
../App/model/driver/APC_Display.c \
../App/model/driver/APC_E2prom.c \
../App/model/driver/APC_Flash.c \
../App/model/driver/APC_LocalPort.c \
../App/model/driver/APC_Max1116.c \
../App/model/driver/APC_Max3100.c \
../App/model/driver/APC_SRAM.c \
../App/model/driver/APC_Spi.c \
../App/model/driver/APC_TMC.c \
../App/model/driver/APC_Timer.c 

OBJS += \
./App/model/driver/APC_ADS1252.o \
./App/model/driver/APC_ChipSelect.o \
./App/model/driver/APC_DAC7612.o \
./App/model/driver/APC_Display.o \
./App/model/driver/APC_E2prom.o \
./App/model/driver/APC_Flash.o \
./App/model/driver/APC_LocalPort.o \
./App/model/driver/APC_Max1116.o \
./App/model/driver/APC_Max3100.o \
./App/model/driver/APC_SRAM.o \
./App/model/driver/APC_Spi.o \
./App/model/driver/APC_TMC.o \
./App/model/driver/APC_Timer.o 

C_DEPS += \
./App/model/driver/APC_ADS1252.d \
./App/model/driver/APC_ChipSelect.d \
./App/model/driver/APC_DAC7612.d \
./App/model/driver/APC_Display.d \
./App/model/driver/APC_E2prom.d \
./App/model/driver/APC_Flash.d \
./App/model/driver/APC_LocalPort.d \
./App/model/driver/APC_Max1116.d \
./App/model/driver/APC_Max3100.d \
./App/model/driver/APC_SRAM.d \
./App/model/driver/APC_Spi.d \
./App/model/driver/APC_TMC.d \
./App/model/driver/APC_Timer.d 


# Each subdirectory must supply rules for building sources it contributes
App/model/driver/%.o App/model/driver/%.su App/model/driver/%.cyclo: ../App/model/driver/%.c App/model/driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App/common -I../App/logic -I../App/model -I../App/logic/cmd -I../App/model/driver -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-model-2f-driver

clean-App-2f-model-2f-driver:
	-$(RM) ./App/model/driver/APC_ADS1252.cyclo ./App/model/driver/APC_ADS1252.d ./App/model/driver/APC_ADS1252.o ./App/model/driver/APC_ADS1252.su ./App/model/driver/APC_ChipSelect.cyclo ./App/model/driver/APC_ChipSelect.d ./App/model/driver/APC_ChipSelect.o ./App/model/driver/APC_ChipSelect.su ./App/model/driver/APC_DAC7612.cyclo ./App/model/driver/APC_DAC7612.d ./App/model/driver/APC_DAC7612.o ./App/model/driver/APC_DAC7612.su ./App/model/driver/APC_Display.cyclo ./App/model/driver/APC_Display.d ./App/model/driver/APC_Display.o ./App/model/driver/APC_Display.su ./App/model/driver/APC_E2prom.cyclo ./App/model/driver/APC_E2prom.d ./App/model/driver/APC_E2prom.o ./App/model/driver/APC_E2prom.su ./App/model/driver/APC_Flash.cyclo ./App/model/driver/APC_Flash.d ./App/model/driver/APC_Flash.o ./App/model/driver/APC_Flash.su ./App/model/driver/APC_LocalPort.cyclo ./App/model/driver/APC_LocalPort.d ./App/model/driver/APC_LocalPort.o ./App/model/driver/APC_LocalPort.su ./App/model/driver/APC_Max1116.cyclo ./App/model/driver/APC_Max1116.d ./App/model/driver/APC_Max1116.o ./App/model/driver/APC_Max1116.su ./App/model/driver/APC_Max3100.cyclo ./App/model/driver/APC_Max3100.d ./App/model/driver/APC_Max3100.o ./App/model/driver/APC_Max3100.su ./App/model/driver/APC_SRAM.cyclo ./App/model/driver/APC_SRAM.d ./App/model/driver/APC_SRAM.o ./App/model/driver/APC_SRAM.su ./App/model/driver/APC_Spi.cyclo ./App/model/driver/APC_Spi.d ./App/model/driver/APC_Spi.o ./App/model/driver/APC_Spi.su ./App/model/driver/APC_TMC.cyclo ./App/model/driver/APC_TMC.d ./App/model/driver/APC_TMC.o ./App/model/driver/APC_TMC.su ./App/model/driver/APC_Timer.cyclo ./App/model/driver/APC_Timer.d ./App/model/driver/APC_Timer.o ./App/model/driver/APC_Timer.su

.PHONY: clean-App-2f-model-2f-driver

