################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Inc/buzzer.c \
../Inc/delay.c \
../Inc/fan.c \
../Inc/hc05.c \
../Inc/mq4.c 

OBJS += \
./Inc/buzzer.o \
./Inc/delay.o \
./Inc/fan.o \
./Inc/hc05.o \
./Inc/mq4.o 

C_DEPS += \
./Inc/buzzer.d \
./Inc/delay.d \
./Inc/fan.d \
./Inc/hc05.d \
./Inc/mq4.d 


# Each subdirectory must supply rules for building sources it contributes
Inc/%.o Inc/%.su Inc/%.cyclo: ../Inc/%.c Inc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Inc

clean-Inc:
	-$(RM) ./Inc/buzzer.cyclo ./Inc/buzzer.d ./Inc/buzzer.o ./Inc/buzzer.su ./Inc/delay.cyclo ./Inc/delay.d ./Inc/delay.o ./Inc/delay.su ./Inc/fan.cyclo ./Inc/fan.d ./Inc/fan.o ./Inc/fan.su ./Inc/hc05.cyclo ./Inc/hc05.d ./Inc/hc05.o ./Inc/hc05.su ./Inc/mq4.cyclo ./Inc/mq4.d ./Inc/mq4.o ./Inc/mq4.su

.PHONY: clean-Inc

