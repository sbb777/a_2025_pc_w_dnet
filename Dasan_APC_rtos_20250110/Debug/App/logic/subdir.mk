################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/logic/APC_AsyncHandler.c \
../App/logic/APC_Main.c \
../App/logic/APC_PidHandler.c \
../App/logic/APC_Scheduler.c \
../App/logic/APC_Synch.c \
../App/logic/APC_Test.c 

OBJS += \
./App/logic/APC_AsyncHandler.o \
./App/logic/APC_Main.o \
./App/logic/APC_PidHandler.o \
./App/logic/APC_Scheduler.o \
./App/logic/APC_Synch.o \
./App/logic/APC_Test.o 

C_DEPS += \
./App/logic/APC_AsyncHandler.d \
./App/logic/APC_Main.d \
./App/logic/APC_PidHandler.d \
./App/logic/APC_Scheduler.d \
./App/logic/APC_Synch.d \
./App/logic/APC_Test.d 


# Each subdirectory must supply rules for building sources it contributes
App/logic/%.o App/logic/%.su App/logic/%.cyclo: ../App/logic/%.c App/logic/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App/common -I../App/logic -I../App/model -I../App/logic/cmd -I../App/model/driver -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App/DeviceNet" -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App" -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App/DeviceNet/includes" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-logic

clean-App-2f-logic:
	-$(RM) ./App/logic/APC_AsyncHandler.cyclo ./App/logic/APC_AsyncHandler.d ./App/logic/APC_AsyncHandler.o ./App/logic/APC_AsyncHandler.su ./App/logic/APC_Main.cyclo ./App/logic/APC_Main.d ./App/logic/APC_Main.o ./App/logic/APC_Main.su ./App/logic/APC_PidHandler.cyclo ./App/logic/APC_PidHandler.d ./App/logic/APC_PidHandler.o ./App/logic/APC_PidHandler.su ./App/logic/APC_Scheduler.cyclo ./App/logic/APC_Scheduler.d ./App/logic/APC_Scheduler.o ./App/logic/APC_Scheduler.su ./App/logic/APC_Synch.cyclo ./App/logic/APC_Synch.d ./App/logic/APC_Synch.o ./App/logic/APC_Synch.su ./App/logic/APC_Test.cyclo ./App/logic/APC_Test.d ./App/logic/APC_Test.o ./App/logic/APC_Test.su

.PHONY: clean-App-2f-logic

