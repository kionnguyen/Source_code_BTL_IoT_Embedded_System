/*
 * gpio.c
 *
 *  Created on: 20 thg 1, 2026
 *      Author: Admin
 */
#include "buzzer.h"

void PB_Init(void)
{
    RCC_APB2ENR |= (1<<0) | (1<<3);   // AFIO + GPIOB clock

    // Disable JTAG, keep SWD
    AFIO_MAPR &= ~(0x7 << 24);
    AFIO_MAPR |=  (0x2 << 24);

    // PB3 output 2MHz push-pull
    GPIOB_CRL &= ~(0xF << 12);
    GPIOB_CRL |=  (0x2 << 12);

    // PB4 output 2MHz push-pull
    GPIOB_CRL &= ~(0xF << 16);
    GPIOB_CRL |=  (0x2 << 16);
}


void Buzzer_On(void)
{
    GPIOB_BSRR = (1 << (3 + 16));   // RESET PB3 = 0 → buzzer ON
}

void Buzzer_Off(void)
{
    GPIOB_BSRR = (1 << 3);          // SET PB3 = 1 → buzzer OFF
}

void Buzzer_Toggle(void)
{
    GPIOB_ODR ^= (1 << 3);          // đảo trạng thái PB3
}

int Buzzer_Read_Pin(void)
{
    if (GPIOB_IDR & (1 << 3))
        return 0;   // OFF
    else
        return 1;   // ON
}


