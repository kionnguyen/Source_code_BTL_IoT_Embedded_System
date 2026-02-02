/*
 * mq4.h
 *
 *  Created on: 21 thg 1, 2026
 *      Author: Admin
 */
#include <stdint.h>

#ifndef MQ4_H_
#define MQ4_H_

#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x40010800UL
#define ADC1_BASE       0x40012400UL

#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_CFGR        (*(volatile uint32_t *)(RCC_BASE + 0x04))

#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00))

#define ADC1_SR         (*(volatile uint32_t *)(ADC1_BASE + 0x00))
#define ADC1_CR1        (*(volatile uint32_t *)(ADC1_BASE + 0x04))
#define ADC1_CR2        (*(volatile uint32_t *)(ADC1_BASE + 0x08))
#define ADC1_SMPR2      (*(volatile uint32_t *)(ADC1_BASE + 0x10))
#define ADC1_SQR1        (*(volatile uint32_t *)(ADC1_BASE + 0x2C))
#define ADC1_SQR3        (*(volatile uint32_t *)(ADC1_BASE + 0x34))
#define ADC1_DR         (*(volatile uint32_t *)(ADC1_BASE + 0x4C))


void ADC1_Init(void);
uint16_t ADC1_Read(void);
uint32_t MQ4_ADC_To_PPM(uint16_t adc);

#endif /* MQ4_H_ */
