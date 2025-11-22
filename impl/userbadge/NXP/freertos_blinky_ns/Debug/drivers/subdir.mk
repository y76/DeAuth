################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_common_arm.c \
../drivers/fsl_flexcomm.c \
../drivers/fsl_gpio.c \
../drivers/fsl_power.c \
../drivers/fsl_reset.c \
../drivers/fsl_usart.c 

C_DEPS += \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_common_arm.d \
./drivers/fsl_flexcomm.d \
./drivers/fsl_gpio.d \
./drivers/fsl_power.d \
./drivers/fsl_reset.d \
./drivers/fsl_usart.d 

OBJS += \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_common_arm.o \
./drivers/fsl_flexcomm.o \
./drivers/fsl_gpio.o \
./drivers/fsl_power.o \
./drivers/fsl_reset.o \
./drivers/fsl_usart.o 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c drivers/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -DCPU_LPC55S69JBD100_cm33_core0 -DSERIAL_PORT_TYPE_UART=1 -DSDK_OS_FREE_RTOS -DMCUXPRESSO_SDK -DCPU_LPC55S69JBD100 -DCPU_LPC55S69JBD100_cm33 -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/source" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/freertos/freertos_kernel/portable/GCC/ARM_CM33/non_secure" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/drivers" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/device" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/startup" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/utilities" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/component/uart" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/component/serial_manager" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/component/lists" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/freertos/freertos_kernel/include" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/CMSIS" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/nsc_functions" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/freertos/secure_context" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_ns/board" -O0 -fno-common -g3 -gdwarf-4 -D __SEMIHOST_HARDFAULT_DISABLE -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__NEWLIB__ -fstack-usage -specs=nano.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-drivers

clean-drivers:
	-$(RM) ./drivers/fsl_clock.d ./drivers/fsl_clock.o ./drivers/fsl_common.d ./drivers/fsl_common.o ./drivers/fsl_common_arm.d ./drivers/fsl_common_arm.o ./drivers/fsl_flexcomm.d ./drivers/fsl_flexcomm.o ./drivers/fsl_gpio.d ./drivers/fsl_gpio.o ./drivers/fsl_power.d ./drivers/fsl_power.o ./drivers/fsl_reset.d ./drivers/fsl_reset.o ./drivers/fsl_usart.d ./drivers/fsl_usart.o

.PHONY: clean-drivers

