################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/logic/cmd/APC_CmdFilter.c \
../App/logic/cmd/APC_CmdFunctions.c \
../App/logic/cmd/APC_CmdHandler.c \
../App/logic/cmd/APC_CmdListener.c 

OBJS += \
./App/logic/cmd/APC_CmdFilter.o \
./App/logic/cmd/APC_CmdFunctions.o \
./App/logic/cmd/APC_CmdHandler.o \
./App/logic/cmd/APC_CmdListener.o 

C_DEPS += \
./App/logic/cmd/APC_CmdFilter.d \
./App/logic/cmd/APC_CmdFunctions.d \
./App/logic/cmd/APC_CmdHandler.d \
./App/logic/cmd/APC_CmdListener.d 


# Each subdirectory must supply rules for building sources it contributes
App/logic/cmd/%.o App/logic/cmd/%.su App/logic/cmd/%.cyclo: ../App/logic/cmd/%.c App/logic/cmd/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App/common -I../App/logic -I../App/model -I../App/logic/cmd -I../App/model/driver -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-logic-2f-cmd

clean-App-2f-logic-2f-cmd:
	-$(RM) ./App/logic/cmd/APC_CmdFilter.cyclo ./App/logic/cmd/APC_CmdFilter.d ./App/logic/cmd/APC_CmdFilter.o ./App/logic/cmd/APC_CmdFilter.su ./App/logic/cmd/APC_CmdFunctions.cyclo ./App/logic/cmd/APC_CmdFunctions.d ./App/logic/cmd/APC_CmdFunctions.o ./App/logic/cmd/APC_CmdFunctions.su ./App/logic/cmd/APC_CmdHandler.cyclo ./App/logic/cmd/APC_CmdHandler.d ./App/logic/cmd/APC_CmdHandler.o ./App/logic/cmd/APC_CmdHandler.su ./App/logic/cmd/APC_CmdListener.cyclo ./App/logic/cmd/APC_CmdListener.d ./App/logic/cmd/APC_CmdListener.o ./App/logic/cmd/APC_CmdListener.su

.PHONY: clean-App-2f-logic-2f-cmd

