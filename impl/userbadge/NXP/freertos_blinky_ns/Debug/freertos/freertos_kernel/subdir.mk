################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../freertos/freertos_kernel/croutine.c \
../freertos/freertos_kernel/event_groups.c \
../freertos/freertos_kernel/list.c \
../freertos/freertos_kernel/queue.c \
../freertos/freertos_kernel/stream_buffer.c \
../freertos/freertos_kernel/tasks.c \
../freertos/freertos_kernel/timers.c 

C_DEPS += \
./freertos/freertos_kernel/croutine.d \
./freertos/freertos_kernel/event_groups.d \
./freertos/freertos_kernel/list.d \
./freertos/freertos_kernel/queue.d \
./freertos/freertos_kernel/stream_buffer.d \
./freertos/freertos_kernel/tasks.d \
./freertos/freertos_kernel/timers.d 

OBJS += \
./freertos/freertos_kernel/croutine.o \
./freertos/freertos_kernel/event_groups.o \
./freertos/freertos_kernel/list.o \
./freertos/freertos_kernel/queue.o \
./freertos/freertos_kernel/stream_buffer.o \
./freertos/freertos_kernel/tasks.o \
./freertos/freertos_kernel/timers.o 


# Each subdirectory must supply rules for building sources it contributes
freertos/freertos_kernel/%.o: ../freertos/freertos_kernel/%.c freertos/freertos_kernel/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -DCPU_LPC55S69JBD100_cm33_core0 -DSERIAL_PORT_TYPE_UART=1 -DSDK_OS_FREE_RTOS -DMCUXPRESSO_SDK -DCPU_LPC55S69JBD100 -DCPU_LPC55S69JBD100_cm33 -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/source" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/freertos/freertos_kernel/portable/GCC/ARM_CM33/non_secure" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/drivers" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/device" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/startup" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/utilities" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/component/uart" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/component/serial_manager" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/component/lists" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/freertos/freertos_kernel/include" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/CMSIS" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/nsc_functions" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/freertos/secure_context" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/board" -O0 -fno-common -g3 -gdwarf-4 -D __SEMIHOST_HARDFAULT_DISABLE -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__NEWLIB__ -fstack-usage -specs=nano.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-freertos-2f-freertos_kernel

clean-freertos-2f-freertos_kernel:
	-$(RM) ./freertos/freertos_kernel/croutine.d ./freertos/freertos_kernel/croutine.o ./freertos/freertos_kernel/event_groups.d ./freertos/freertos_kernel/event_groups.o ./freertos/freertos_kernel/list.d ./freertos/freertos_kernel/list.o ./freertos/freertos_kernel/queue.d ./freertos/freertos_kernel/queue.o ./freertos/freertos_kernel/stream_buffer.d ./freertos/freertos_kernel/stream_buffer.o ./freertos/freertos_kernel/tasks.d ./freertos/freertos_kernel/tasks.o ./freertos/freertos_kernel/timers.d ./freertos/freertos_kernel/timers.o

.PHONY: clean-freertos-2f-freertos_kernel

