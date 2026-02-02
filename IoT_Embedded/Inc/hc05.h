/*
 * adc.h
 *
 *  Created on: 21 thg 1, 2026
 *      Author: Admin
 */


#include <stdint.h>

#ifndef HC05_H_
#define HC05_H_

// Base addresses
#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x40010800UL
#define USART1_BASE     0x40013800UL

// RCC
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

// GPIOA
#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_ODR   	(*(volatile unsigned int *)(0x4001080C))

// USART1
#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))

void USART1_Init(void);
void USART1_SendChar(char c);
void USART1_SendString(char *s);
void USART1_SendNumber(uint32_t num);
char USART1_ReadChar(void);
int USART1_DataAvailable(void);

#endif /* ADC_H_ */
