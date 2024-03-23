################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/Src/io_task.c \
../Application/Src/ltc2991.c \
../Application/Src/mcp23017.c \
../Application/Src/serial_buffer_task.c \
../Application/Src/serial_cmd_task.c 

OBJS += \
./Application/Src/io_task.o \
./Application/Src/ltc2991.o \
./Application/Src/mcp23017.o \
./Application/Src/serial_buffer_task.o \
./Application/Src/serial_cmd_task.o 

C_DEPS += \
./Application/Src/io_task.d \
./Application/Src/ltc2991.d \
./Application/Src/mcp23017.d \
./Application/Src/serial_buffer_task.d \
./Application/Src/serial_cmd_task.d 


# Each subdirectory must supply rules for building sources it contributes
Application/Src/%.o Application/Src/%.su Application/Src/%.cyclo: ../Application/Src/%.c Application/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Application/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-Src

clean-Application-2f-Src:
	-$(RM) ./Application/Src/io_task.cyclo ./Application/Src/io_task.d ./Application/Src/io_task.o ./Application/Src/io_task.su ./Application/Src/ltc2991.cyclo ./Application/Src/ltc2991.d ./Application/Src/ltc2991.o ./Application/Src/ltc2991.su ./Application/Src/mcp23017.cyclo ./Application/Src/mcp23017.d ./Application/Src/mcp23017.o ./Application/Src/mcp23017.su ./Application/Src/serial_buffer_task.cyclo ./Application/Src/serial_buffer_task.d ./Application/Src/serial_buffer_task.o ./Application/Src/serial_buffer_task.su ./Application/Src/serial_cmd_task.cyclo ./Application/Src/serial_cmd_task.d ./Application/Src/serial_cmd_task.o ./Application/Src/serial_cmd_task.su

.PHONY: clean-Application-2f-Src

