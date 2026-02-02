/*
 * mq4.c
 *
 *  Created on: 21 thg 1, 2026
 *      Author: Admin
 */

#include "mq4.h"
#include "buzzer.h"
#include "hc05.h"
#include "delay.h"

void ADC1_Init(void)
{
    // Enable clocks
    RCC_APB2ENR |= (1 << 0);   // AFIO
    RCC_APB2ENR |= (1 << 2);   // GPIOA
    RCC_APB2ENR |= (1 << 9);   // ADC1

    // PA0 analog mode
    GPIOA_CRL &= ~(0xF << 0);

    // ADC clock = PCLK2 / 8
    RCC_CFGR &= ~(0x3 << 14);
    RCC_CFGR |=  (0x3 << 14);

    // Sample time CH0 = 239.5 cycles
    ADC1_SMPR2 &= ~(0x7 << 0);
    ADC1_SMPR2 |=  (0x7 << 0);

    // 1 conversion
    ADC1_SQR1 &= ~(0xF << 20);
    ADC1_SQR3 = 0;

    // Continuous mode
    ADC1_CR2 |= (1 << 1);

    // Enable ADC (double ADON)
    ADC1_CR2 |= (1 << 0);
    delay_ms(5);
    ADC1_CR2 |= (1 << 0);

    // Reset calibration
    ADC1_CR2 |= (1 << 3);
    while (ADC1_CR2 & (1 << 3));

    // Start calibration
    ADC1_CR2 |= (1 << 2);
    while (ADC1_CR2 & (1 << 2));

    // Enable ADC again
    ADC1_CR2 |= (1 << 0);

    // Start conversion
    ADC1_CR2 |= (1 << 22);
}


uint16_t ADC1_Read(void)
{
    uint32_t timeout = 1000000;
    while (!(ADC1_SR & (1 << 1)) && timeout--);

    if (timeout == 0)
        return 0xFFFF;

    return ADC1_DR;
}


uint32_t MQ4_ADC_To_PPM(uint16_t adc)
{
    return (adc * 1000UL) / 4095;
}
