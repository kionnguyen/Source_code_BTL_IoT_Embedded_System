/*
 * fan.c
 *
 *  Created on: 22 thg 1, 2026
 *      Author: Admin
 */
#include "fan.h"
#include "buzzer.h"

void Fan_On(void)
{
    GPIOB_BSRR = (1 << 4);        // PB4 = 1
}

void Fan_Off(void)
{
    GPIOB_BSRR = (1 << (4 + 16)); // PB4 = 0
}

int Fan_Read_Pin(void)
{
    if (GPIOB_IDR & (1 << 4))
        return 1;   // ON
    else
        return 0;   // OFF
}

