/*
 * gpio.h
 *
 *  Created on: 20 thg 1, 2026
 *      Author: Admin
 */
#include <stdint.h>

#ifndef GPIO_H_
#define GPIO_H_

//Base addresses
#define RCC_BASE        0x40021000UL
#define GPIOB_BASE      0x40010C00UL
#define AFIO_BASE       0x40010000UL

//RCC
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))
//GPIOB
#define GPIOB_CRL       (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_ODR       (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))
#define GPIOB_BSRR      (*(volatile uint32_t *)(GPIOB_BASE + 0x10))
#define GPIOB_IDR       (*(volatile uint32_t *)(GPIOB_BASE + 0x08))
//AFIO
#define AFIO_MAPR       (*(volatile uint32_t *)(AFIO_BASE + 0x04))


void PB_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Toggle(void);
int Buzzer_Read_Pin(void);

#endif /* GPIO_H_ */
