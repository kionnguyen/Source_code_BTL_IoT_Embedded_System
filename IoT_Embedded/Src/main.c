#include "buzzer.h"
#include "hc05.h"
#include "mq4.h"
#include "delay.h"
#include "fan.h"
#include<stdint.h>

#define PPM_THRESHOLD 600

typedef enum
{
    STATE_NORMAL = 0,
    STATE_ALARM  = 1
} system_state_t;

static system_state_t system_state = STATE_NORMAL;

static uint8_t fan_prev_state = 0;
static uint8_t buzzer_prev_state = 0;

static inline void USART1_ClearErrors(void)
{
    volatile uint32_t tmp;
    tmp = USART1_SR;
    tmp = USART1_DR;
    (void)tmp;
}

static void HC05_ApplyCommand(uint8_t code)
{
    // If state alarm, not receiving command from web
    if (system_state == STATE_ALARM)
    {
        USART1_SendString("ALARM ACTIVE\r\n");
        return;
    }

    switch (code)
    {
        case 1: Fan_On();    USART1_SendString("ACK 1\r\n"); break;
        case 2: Fan_Off();   USART1_SendString("ACK 2\r\n"); break;
        case 3: Buzzer_On(); USART1_SendString("ACK 3\r\n"); break;
        case 4: Buzzer_Off();USART1_SendString("ACK 4\r\n"); break;
        default: USART1_SendString("ERR CMD\r\n"); break;
    }
}

void HC05_PollReceiveCommand(void)
{
    while (USART1_SR & (1 << 5))  // RXNE
    {
        if (USART1_SR & ((1 << 3) | (1 << 2) | (1 << 1))) // ORE=3, NE=2, FE=1
        {
            USART1_ClearErrors();
            continue;
        }

        char c = (char)USART1_DR;  // Read received data

        if (c == '\r' || c == '\n' || c == ' ') continue;

        if (c >= '1' && c <= '4')
        {
            HC05_ApplyCommand((uint8_t)(c - '0'));
        }
        else
        {
            // Error
             USART1_SendString("ERR FORMAT\r\n");
        }
    }
}

// Polling function in delay
void delay_ms_with_poll(uint32_t ms)
{
    while (ms--)
    {
        HC05_PollReceiveCommand();
        delay_ms(1);
    }
}

int main(void)
{
    uint16_t adc, ppm;
    uint16_t fan_state, buzzer_state;

    ADC1_Init();
    USART1_Init();
    PB_Init();

    Fan_Off();
    Buzzer_Off();
    delay_ms(50);

    while (1)
    {
    	adc = ADC1_Read();
    	ppm = MQ4_ADC_To_PPM(adc);

    	// Enter Alarm state
		if (ppm > PPM_THRESHOLD && system_state == STATE_NORMAL)
		{
			fan_prev_state    = Fan_Read_Pin();
			buzzer_prev_state = Buzzer_Read_Pin();

			//Turn on fan + buzzer
			Fan_On();
			Buzzer_On();

			system_state = STATE_ALARM;
			USART1_SendString("ALARM ON\r\n");
		}
		else if (ppm < PPM_THRESHOLD && system_state == STATE_ALARM)
		{
			// Previous state
			if (fan_prev_state)    Fan_On();
			else                   Fan_Off();

			if (buzzer_prev_state) Buzzer_On();
			else                   Buzzer_Off();

			system_state = STATE_NORMAL;
			USART1_SendString("ALARM OFF\r\n");
		}

        fan_state = Fan_Read_Pin();
        buzzer_state = Buzzer_Read_Pin();

        // Send data to HC05
        USART1_SendNumber(ppm);
        USART1_SendString(" ");
        USART1_SendNumber(fan_state);
        USART1_SendString(" ");
        USART1_SendNumber(buzzer_state);
        USART1_SendString("\r\n");

        delay_ms_with_poll(200);
    }
}




