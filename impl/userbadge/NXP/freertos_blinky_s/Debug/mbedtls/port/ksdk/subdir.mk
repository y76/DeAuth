################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mbedtls/port/ksdk/aes_alt.c \
../mbedtls/port/ksdk/des_alt.c \
../mbedtls/port/ksdk/ecp_alt.c \
../mbedtls/port/ksdk/ecp_alt_ksdk.c \
../mbedtls/port/ksdk/ecp_curves_alt.c \
../mbedtls/port/ksdk/ksdk_mbedtls.c 

C_DEPS += \
./mbedtls/port/ksdk/aes_alt.d \
./mbedtls/port/ksdk/des_alt.d \
./mbedtls/port/ksdk/ecp_alt.d \
./mbedtls/port/ksdk/ecp_alt_ksdk.d \
./mbedtls/port/ksdk/ecp_curves_alt.d \
./mbedtls/port/ksdk/ksdk_mbedtls.d 

OBJS += \
./mbedtls/port/ksdk/aes_alt.o \
./mbedtls/port/ksdk/des_alt.o \
./mbedtls/port/ksdk/ecp_alt.o \
./mbedtls/port/ksdk/ecp_alt_ksdk.o \
./mbedtls/port/ksdk/ecp_curves_alt.o \
./mbedtls/port/ksdk/ksdk_mbedtls.o 


# Each subdirectory must supply rules for building sources it contributes
mbedtls/port/ksdk/%.o: ../mbedtls/port/ksdk/%.c mbedtls/port/ksdk/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -DPERFORMANCE_EVALUATION -DDEMO_CODE_START_NS=0x40000 -DCPU_LPC55S69JBD100_cm33_core0 -DSERIAL_PORT_TYPE_UART=1 -DMCUXPRESSO_SDK -DCPU_LPC55S69JBD100 -DCPU_LPC55S69JBD100_cm33 -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -DMBEDTLS_FREESCALE_HASHCRYPT_SHA1 -DMBEDTLS_FREESCALE_HASHCRYPT_SHA256 -DPRINTF_ADVANCED_ENABLE=1 -DPRINTF_FLOAT_ENABLE=1 -DFREESCALE_KSDK_BM -DMBEDTLS_CONFIG_FILE='"ksdk_mbedtls_config.h"' -DDONT_ENABLE_FLASH_PREFETCH -DPRINTF_FLOAT_ENABLE=0 -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/source" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/freertos/freertos_kernel/portable/GCC/ARM_CM33/secure" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/drivers" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/device" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/startup" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/utilities" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/component/uart" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/component/serial_manager" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/component/lists" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/CMSIS" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/board" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/nsc_functions" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/mbedtls/include" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/mbedtls/library" -I"/home/pavel/apps/LOC-PAISA/1_IoTDev/1_NXPBoard/freertos_blinky_s/mbedtls/port/ksdk" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -mcmse -D__NEWLIB__ -fstack-usage -specs=nano.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-mbedtls-2f-port-2f-ksdk

clean-mbedtls-2f-port-2f-ksdk:
	-$(RM) ./mbedtls/port/ksdk/aes_alt.d ./mbedtls/port/ksdk/aes_alt.o ./mbedtls/port/ksdk/des_alt.d ./mbedtls/port/ksdk/des_alt.o ./mbedtls/port/ksdk/ecp_alt.d ./mbedtls/port/ksdk/ecp_alt.o ./mbedtls/port/ksdk/ecp_alt_ksdk.d ./mbedtls/port/ksdk/ecp_alt_ksdk.o ./mbedtls/port/ksdk/ecp_curves_alt.d ./mbedtls/port/ksdk/ecp_curves_alt.o ./mbedtls/port/ksdk/ksdk_mbedtls.d ./mbedtls/port/ksdk/ksdk_mbedtls.o

.PHONY: clean-mbedtls-2f-port-2f-ksdk

