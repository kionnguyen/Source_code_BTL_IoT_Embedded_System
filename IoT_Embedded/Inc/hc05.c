/*
 * adc.c
 *
 *  Created on: 21 thg 1, 2026
 *      Author: Admin
 *
 */

#include "hc05.h"

void USART1_Init(void)
{
    // Enable GPIOA & USART1
    RCC_APB2ENR |= (1 << 2);    // GPIOA
    RCC_APB2ENR |= (1 << 14);   // USART1

    // PA9 = TX (AF push-pull, 50 MHz)
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |=  (0xB << 4);   // CNF=10, MODE=11

    // PA10 = RX (input floating)
    GPIOA_CRH &= ~(0xF << 8);
    GPIOA_CRH |=  (0x4 << 8);   // CNF=01, MODE=00

    // Baudrate 9600 @ 8 MHz
    USART1_BRR = 0x341;

    // Enable USART, TX, RX
    USART1_CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
}

void USART1_SendChar(char c)
{
    while (!(USART1_SR & (1 << 7))); // TXE
    USART1_DR = c;
}

void USART1_SendString(char *s)
{
    while (*s)
        USART1_SendChar(*s++);
}

char USART1_ReadChar(void)
{
    while (!(USART1_SR & (1 << 5))); // RXNE
    return (char)USART1_DR;
}

int USART1_DataAvailable(void)
{
    return (USART1_SR & (1 << 5)); // RXNE
}

void USART1_SendNumber(uint32_t num)
{
    char buf[10];
    int i = 0;

    if (num == 0)
    {
        USART1_SendChar('0');
        return;
    }

    while (num)
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i--)
        USART1_SendChar(buf[i]);
}



