################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ThirdParty/FreeRTOS_tp/croutine.c \
../ThirdParty/FreeRTOS_tp/event_groups.c \
../ThirdParty/FreeRTOS_tp/list.c \
../ThirdParty/FreeRTOS_tp/queue.c \
../ThirdParty/FreeRTOS_tp/stream_buffer.c \
../ThirdParty/FreeRTOS_tp/tasks.c \
../ThirdParty/FreeRTOS_tp/timers.c 

OBJS += \
./ThirdParty/FreeRTOS_tp/croutine.o \
./ThirdParty/FreeRTOS_tp/event_groups.o \
./ThirdParty/FreeRTOS_tp/list.o \
./ThirdParty/FreeRTOS_tp/queue.o \
./ThirdParty/FreeRTOS_tp/stream_buffer.o \
./ThirdParty/FreeRTOS_tp/tasks.o \
./ThirdParty/FreeRTOS_tp/timers.o 

C_DEPS += \
./ThirdParty/FreeRTOS_tp/croutine.d \
./ThirdParty/FreeRTOS_tp/event_groups.d \
./ThirdParty/FreeRTOS_tp/list.d \
./ThirdParty/FreeRTOS_tp/queue.d \
./ThirdParty/FreeRTOS_tp/stream_buffer.d \
./ThirdParty/FreeRTOS_tp/tasks.d \
./ThirdParty/FreeRTOS_tp/timers.d 


# Each subdirectory must supply rules for building sources it contributes
ThirdParty/FreeRTOS_tp/%.o ThirdParty/FreeRTOS_tp/%.su ThirdParty/FreeRTOS_tp/%.cyclo: ../ThirdParty/FreeRTOS_tp/%.c ThirdParty/FreeRTOS_tp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"C:/Users/ThinkPad/Desktop/FreeRTOS/freeRTOS_workspace/freeRTOS_project/ThirdParty/FreeRTOS_tp" -I"C:/Users/ThinkPad/Desktop/FreeRTOS/freeRTOS_workspace/freeRTOS_project/ThirdParty/FreeRTOS_tp/include" -I"C:/Users/ThinkPad/Desktop/FreeRTOS/freeRTOS_workspace/freeRTOS_project/ThirdParty/FreeRTOS_tp/portable/GCC/ARM_CM4F" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-ThirdParty-2f-FreeRTOS_tp

clean-ThirdParty-2f-FreeRTOS_tp:
	-$(RM) ./ThirdParty/FreeRTOS_tp/croutine.cyclo ./ThirdParty/FreeRTOS_tp/croutine.d ./ThirdParty/FreeRTOS_tp/croutine.o ./ThirdParty/FreeRTOS_tp/croutine.su ./ThirdParty/FreeRTOS_tp/event_groups.cyclo ./ThirdParty/FreeRTOS_tp/event_groups.d ./ThirdParty/FreeRTOS_tp/event_groups.o ./ThirdParty/FreeRTOS_tp/event_groups.su ./ThirdParty/FreeRTOS_tp/list.cyclo ./ThirdParty/FreeRTOS_tp/list.d ./ThirdParty/FreeRTOS_tp/list.o ./ThirdParty/FreeRTOS_tp/list.su ./ThirdParty/FreeRTOS_tp/queue.cyclo ./ThirdParty/FreeRTOS_tp/queue.d ./ThirdParty/FreeRTOS_tp/queue.o ./ThirdParty/FreeRTOS_tp/queue.su ./ThirdParty/FreeRTOS_tp/stream_buffer.cyclo ./ThirdParty/FreeRTOS_tp/stream_buffer.d ./ThirdParty/FreeRTOS_tp/stream_buffer.o ./ThirdParty/FreeRTOS_tp/stream_buffer.su ./ThirdParty/FreeRTOS_tp/tasks.cyclo ./ThirdParty/FreeRTOS_tp/tasks.d ./ThirdParty/FreeRTOS_tp/tasks.o ./ThirdParty/FreeRTOS_tp/tasks.su ./ThirdParty/FreeRTOS_tp/timers.cyclo ./ThirdParty/FreeRTOS_tp/timers.d ./ThirdParty/FreeRTOS_tp/timers.o ./ThirdParty/FreeRTOS_tp/timers.su

.PHONY: clean-ThirdParty-2f-FreeRTOS_tp

