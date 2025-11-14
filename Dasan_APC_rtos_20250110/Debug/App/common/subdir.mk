################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/common/APC_Queue.c \
../App/common/APC_Utils.c 

OBJS += \
./App/common/APC_Queue.o \
./App/common/APC_Utils.o 

C_DEPS += \
./App/common/APC_Queue.d \
./App/common/APC_Utils.d 


# Each subdirectory must supply rules for building sources it contributes
App/common/%.o App/common/%.su App/common/%.cyclo: ../App/common/%.c App/common/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App/common -I../App/logic -I../App/model -I../App/logic/cmd -I../App/model/driver -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App/DeviceNet" -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App" -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App/DeviceNet/includes" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-common

clean-App-2f-common:
	-$(RM) ./App/common/APC_Queue.cyclo ./App/common/APC_Queue.d ./App/common/APC_Queue.o ./App/common/APC_Queue.su ./App/common/APC_Utils.cyclo ./App/common/APC_Utils.d ./App/common/APC_Utils.o ./App/common/APC_Utils.su

.PHONY: clean-App-2f-common

