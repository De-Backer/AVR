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

#include "../include/CAN_MCP2515.h"
#include "../include/SPI.h"
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
		char* buffer = "0000";
		/* Transmit_USART0 */
		//        itoa((0x1F & ADMUX), buffer, 10);
		itoa(source, buffer, 10);
		while (*buffer) { Transmit_USART0(*buffer++); }
		buffer = "0000";     /* clean buffer */
		Transmit_USART0(45); /* - */
							 //        uint16_t data3 = ADC_value(source);
		utoa(ADC_value(source), buffer, 10);
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

        /* can */
        MCP2515_init();

        /* test can */
        CAN_TX_msg.id           = 20;
        CAN_TX_msg.ext_id       = 0;
        CAN_TX_msg.rtr          = 0;
        CAN_TX_msg.length       = 2;
        CAN_TX_msg.data_byte[0] = 0;
        CAN_TX_msg.data_byte[1] = 0;
        CAN_TX_msg.data_byte[2] = 0;
        CAN_TX_msg.data_byte[3] = 0;
        CAN_TX_msg.data_byte[4] = 0;
        CAN_TX_msg.data_byte[5] = 0;
        CAN_TX_msg.data_byte[6] = 0;
        CAN_TX_msg.data_byte[7] = 0;

        /* can */
        MCP2515_message_TX();
        //        _delay_ms(50); // 2Hz
        //        if (MCP2515_check_for_incoming_message())
        //        {
        //            MCP2515_message_RX();
        //            Transmit_USART0(CAN_RX_msg.id);
        //        }
        for (;;)
        {
            /* loop */;

            /* verwerk de RX USART0 buffer */
            int8_t RX_BufferCount = RingBuffer_GetCount(&RX_Buffer);
            while (RX_BufferCount--)
            { Transmit_USART0(RingBuffer_Remove(&RX_Buffer)); }

            Transmit_USART0(10); /* new line */
                                 //            _delay_ms(500);      // 2Hz
                                 /* TEST loop */
            //            PORTC ^= (0x01 << 0);
            uint8_t source = 0;
            for (; source < 8; ++source)
            {
                plot(source);
                if (MCP2515_check_for_incoming_message())
                {
                    MCP2515_message_RX();
                    Transmit_USART0(CAN_RX_msg.id);
                }
                union data
                {
                    uint16_t in;
                    char     out[2];
                } adc;
                adc.in        = ADC_value(source);
                CAN_TX_msg.id = source; /* µc 1 */
                //                CAN_TX_msg.id           = source | 0x10; /*
                //                µc2 */
                CAN_TX_msg.data_byte[0] = adc.out[0];
                CAN_TX_msg.data_byte[1] = adc.out[1];
                MCP2515_message_TX();

                if (MCP2515_check_for_incoming_message())
                {
                    MCP2515_message_RX();
                    Transmit_USART0(CAN_RX_msg.id);
                }

                MCP2515_check_for_interrupts();
            }
        }
        return 0;
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif
