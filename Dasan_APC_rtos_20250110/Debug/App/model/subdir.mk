################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/model/APC_AccessMode.c \
../App/model/APC_Board.c \
../App/model/APC_CompAir.c \
../App/model/APC_ControlMode.c \
../App/model/APC_Controller.c \
../App/model/APC_Error.c \
../App/model/APC_Learn.c \
../App/model/APC_Power.c \
../App/model/APC_RemotePort.c \
../App/model/APC_Sensor.c \
../App/model/APC_Valve.c \
../App/model/APC_Warning.c 

OBJS += \
./App/model/APC_AccessMode.o \
./App/model/APC_Board.o \
./App/model/APC_CompAir.o \
./App/model/APC_ControlMode.o \
./App/model/APC_Controller.o \
./App/model/APC_Error.o \
./App/model/APC_Learn.o \
./App/model/APC_Power.o \
./App/model/APC_RemotePort.o \
./App/model/APC_Sensor.o \
./App/model/APC_Valve.o \
./App/model/APC_Warning.o 

C_DEPS += \
./App/model/APC_AccessMode.d \
./App/model/APC_Board.d \
./App/model/APC_CompAir.d \
./App/model/APC_ControlMode.d \
./App/model/APC_Controller.d \
./App/model/APC_Error.d \
./App/model/APC_Learn.d \
./App/model/APC_Power.d \
./App/model/APC_RemotePort.d \
./App/model/APC_Sensor.d \
./App/model/APC_Valve.d \
./App/model/APC_Warning.d 


# Each subdirectory must supply rules for building sources it contributes
App/model/%.o App/model/%.su App/model/%.cyclo: ../App/model/%.c App/model/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App/common -I../App/logic -I../App/model -I../App/logic/cmd -I../App/model/driver -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App/DeviceNet" -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App" -I"D:/git/a_2025_pc_w_dnet/Dasan_APC_rtos_20250110/App/DeviceNet/includes" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-model

clean-App-2f-model:
	-$(RM) ./App/model/APC_AccessMode.cyclo ./App/model/APC_AccessMode.d ./App/model/APC_AccessMode.o ./App/model/APC_AccessMode.su ./App/model/APC_Board.cyclo ./App/model/APC_Board.d ./App/model/APC_Board.o ./App/model/APC_Board.su ./App/model/APC_CompAir.cyclo ./App/model/APC_CompAir.d ./App/model/APC_CompAir.o ./App/model/APC_CompAir.su ./App/model/APC_ControlMode.cyclo ./App/model/APC_ControlMode.d ./App/model/APC_ControlMode.o ./App/model/APC_ControlMode.su ./App/model/APC_Controller.cyclo ./App/model/APC_Controller.d ./App/model/APC_Controller.o ./App/model/APC_Controller.su ./App/model/APC_Error.cyclo ./App/model/APC_Error.d ./App/model/APC_Error.o ./App/model/APC_Error.su ./App/model/APC_Learn.cyclo ./App/model/APC_Learn.d ./App/model/APC_Learn.o ./App/model/APC_Learn.su ./App/model/APC_Power.cyclo ./App/model/APC_Power.d ./App/model/APC_Power.o ./App/model/APC_Power.su ./App/model/APC_RemotePort.cyclo ./App/model/APC_RemotePort.d ./App/model/APC_RemotePort.o ./App/model/APC_RemotePort.su ./App/model/APC_Sensor.cyclo ./App/model/APC_Sensor.d ./App/model/APC_Sensor.o ./App/model/APC_Sensor.su ./App/model/APC_Valve.cyclo ./App/model/APC_Valve.d ./App/model/APC_Valve.o ./App/model/APC_Valve.su ./App/model/APC_Warning.cyclo ./App/model/APC_Warning.d ./App/model/APC_Warning.o ./App/model/APC_Warning.su

.PHONY: clean-App-2f-model

