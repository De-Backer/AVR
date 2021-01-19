/* main.c
 * 20200330
 * Author: Simon De Backer
 * info  :
 * atmega1284p ADC naar USART 8 ingangen
 *
 * 5V
 * - 10K NTC
 * ------ Pin ADC
 * - 2K4 weerstand
 * GND
 */

#ifndef _main_c__
#define _main_c__

#ifdef __cplusplus
extern "C"
{
#endif

#include "../include/adc.h"
#include "../include/usart.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

    /* Configuring the Pin */
    void init_io()
    {
        /* Data Direction Register
         * 0 = ingang
         * 1 = uitgang
         **/
        DDRA = 0b00000000;
        DDRB = 0b00000000;
        DDRC = 0b00000001;
        DDRD = 0b00000000;

        /* Data Register
         * 0 = laag (uitgang) / tri-state (ingang)
         * 1 = hoog (uitgang) / pull up (ingang)
         **/
        PORTA = 0x00;
        PORTB = 0xff;
        PORTC = 0xff;
        PORTD = 0xff;
    }

    void plot(uint8_t source)
    {
        _delay_us(600);
        source &= 0b00000111;
        ADMUX &= 0b11100000; // 0
        ADMUX |= source;
        _delay_us(300);

        uint16_t data   = 0;
        uint16_t data2  = 0;
        uint16_t data3  = 0;
        char*    buffer = "0000";

        /* Sleep Enable */
        SMCR |= (0x01 << SE);
        __asm__("SLEEP");

        data = ADCL; /* ADCL must be read first, then ADCH ref 23.9.3 */
        data |= (ADCH << 2);

        uint8_t var = 0;
        for (; var < 2; ++var)
        {
            uint8_t var1 = 0;
            for (; var1 < 10; ++var1)
            {
                /* Sleep Enable */
                SMCR |= (0x01 << SE);
                __asm__("SLEEP");

                data = ADCL; /* ADCL must be read first, then ADCH ref 23.9.3 */
                data |= (ADCH << 2);
                data2 += data;
            }

            data3 += (data2 >> 0);
        }

        /* Transmit_USART0 */
        itoa((0x1F & ADMUX), buffer, 10);
        while (*buffer) { Transmit_USART0(*buffer++); }
        buffer = "0000";     /* clean buffer */
        Transmit_USART0(45); /* - */
        utoa(data3, buffer, 10);
        while (*buffer) { Transmit_USART0(*buffer++); }
        Transmit_USART0(59); /* ; */
    }
    /* */
    int main(void)
    {
        /* init */
        init_io();
        PORTC ^= (0x01 << 0);
        init_ADC();
        PORTC ^= (0x01 << 0);
        init_USART0();

        Test_Transmit_USART0();

        /*  Sleep Mode Control Register - ADC Noise Reduction */
        SMCR |= (0x01 << SM0);

        sei(); // Enable Global Interrupts

        for (;;)
        {
            /* loop */;

            /* verwerk de RX USART0 buffer */
            int8_t RX_BufferCount = RingBuffer_GetCount(&RX_Buffer);
            while (RX_BufferCount--)
            { Transmit_USART0(RingBuffer_Remove(&RX_Buffer)); }

            Transmit_USART0(10); /* new line */
            _delay_ms(500);      // 2Hz
            /* TEST loop */
            //            PORTC ^= (0x01 << 0);
            uint8_t source = 0;
            for (; source < 8; ++source) { plot(source); }
        }
        return 0;
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif
