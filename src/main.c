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
#include <avr/wdt.h>
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
        //        MCP2515_message_TX();
        //        _delay_ms(50); // 2Hz
        //        if (MCP2515_check_for_incoming_message())
        //        {
        //            MCP2515_message_RX();
        //            Transmit_USART0(CAN_RX_msg.id);
        //        }
        wdt_enable(WDTO_250MS); /* Watchdog Reset after 250mSec */
        for (;;)
        {
            /* loop */;
            wdt_reset(); /* Reset Watchdog timer*/

            /* verwerk de RX USART0 buffer */
            int8_t RX_BufferCount = RingBuffer_GetCount(&RX_Buffer);
            CAN_TX_msg.id         = 50;
            CAN_TX_msg.length     = 0;
            while (RX_BufferCount--)
            {
                char* Buffer = "RX_BufferCount";
                Transmit_USART0(10); /* new line */
                while (*Buffer) { Transmit_USART0(*Buffer++); }
                Transmit_USART0(10); /* new line */

                CAN_TX_msg.data_byte[CAN_TX_msg.length++] =
                    RingBuffer_Remove(&RX_Buffer);
                if (CAN_TX_msg.length > 7 || RX_BufferCount == 0)
                {
                    MCP2515_message_TX();

                    char* Buffer = "send_message";
                    Transmit_USART0(10); /* new line */
                    while (*Buffer) { Transmit_USART0(*Buffer++); }
                    Transmit_USART0(10); /* new line */

                    uint8_t temp = 0;
                    for (; temp < CAN_TX_msg.length; ++temp)
                    { Transmit_USART0(CAN_TX_msg.data_byte[temp]); }

                    ++CAN_TX_msg.id;
                    CAN_TX_msg.length = 0;
                }
            }
            //            Transmit_USART0(10); /* new line */
            //            _delay_ms(500);      // 2Hz
            /* TEST loop */
            //            PORTC ^= (0x01 << 0);
            if (MCP2515_check_for_incoming_message())
            {
                char* Buffer = "incoming_message";
                Transmit_USART0(10); /* new line */
                while (*Buffer) { Transmit_USART0(*Buffer++); }
                Transmit_USART0(10); /* new line */

                if (MCP2515_message_RX())
                {
                    char* Buffer = "MCP2515_message_RX";
                    Transmit_USART0(10); /* new line */
                    while (*Buffer) { Transmit_USART0(*Buffer++); }
                    Transmit_USART0(10); /* new line */

                    uint8_t temp = 0;
                    for (; temp < CAN_RX_msg.length; ++temp)
                    {
                        //                        char* buffer = "00000000";
                        /* Transmit_USART0 */
                        //                        itoa(CAN_RX_msg.data_byte[temp],
                        //                        buffer, 10);
                        Transmit_USART0(CAN_RX_msg.data_byte[temp]);
                        //                        while (*buffer) {
                        //                        Transmit_USART0(*buffer++); }
                    }
                }
            }
            //            uint8_t source = 0;
            //            for (; source < 8; ++source)
            //            {
            //                //                plot(source);
            //                if (MCP2515_check_for_incoming_message())
            //                {
            //                    MCP2515_message_RX();
            //                    uint8_t temp = 0;
            //                    for (; temp < CAN_RX_msg.length; ++temp)
            //                    {
            //                        char* buffer = "00000000";
            //                        /* Transmit_USART0 */
            //                        itoa(CAN_RX_msg.data_byte[temp], buffer,
            //                        10); while (*buffer) {
            //                        Transmit_USART0(*buffer++); }
            //                    }
            //                }
            //                union data
            //                {
            //                    uint16_t in;
            //                    char     out[2];
            //                } adc;
            //                adc.in = ADC_value(source);
            //                if ((PINA & 0x01) == 0x01)
            //                { CAN_TX_msg.id = source; /* µc 1 */ }
            //                else
            //                {
            //                    CAN_TX_msg.id = source | 0x10; /*  µc2 */
            //                }
            //                CAN_TX_msg.data_byte[0] = adc.out[0];
            //                CAN_TX_msg.data_byte[1] = adc.out[1];

            //                /* test errors */
            //                if (MCP2515_read_register(EFLG) == 0)
            //                {
            //                    if (MCP2515_message_TX() == 0)
            //                    {
            //                        char* Buffer = "Error no Transmit
            //                        buffer empty"; Transmit_USART0(10); /*
            //                        new line */ while (*Buffer) {
            //                        Transmit_USART0(*Buffer++); }
            //                        MCP2515_init();
            //                    }
            //                }
            //                else
            //                {
            //                    char* Buffer = "TEC";
            //                    Transmit_USART0(10); /* new line */

            //                    while (*Buffer) {
            //                    Transmit_USART0(*Buffer++); } Buffer =
            //                    "000"; itoa(MCP2515_read_register(TEC),
            //                    Buffer, 10); while (*Buffer) {
            //                    Transmit_USART0(*Buffer++); } Buffer = "
            //                    REC "; while (*Buffer) {
            //                    Transmit_USART0(*Buffer++); } Buffer =
            //                    "000"; itoa(MCP2515_read_register(REC),
            //                    Buffer, 10); while (*Buffer) {
            //                    Transmit_USART0(*Buffer++); }
            //                    MCP2515_init();
            //                }
            //                //                if
            //                (MCP2515_read_register(TEC) > 127)
            //                //                {

            //                //                    char* Buffer =
            //                "Error-Passive TEC";
            //                //                    Transmit_USART0(10); /*
            //                new line */

            //                //                    while (*Buffer) {
            //                // Transmit_USART0(*Buffer++); } Buffer =
            //                //                    "000";
            //                itoa(MCP2515_read_register(TEC),
            //                //                    Buffer, 10); while
            //                (*Buffer) {
            //                // Transmit_USART0(*Buffer++); } Buffer = "
            //                //                    REC "; while (*Buffer) {
            //                // Transmit_USART0(*Buffer++); } Buffer =
            //                //                    "000";
            //                itoa(MCP2515_read_register(REC),
            //                //                    Buffer, 10); while
            //                (*Buffer) {
            //                // Transmit_USART0(*Buffer++); }

            //                //                    Transmit_USART0(10); /*
            //                new line */
            //                //                }
            //                //                if
            //                (MCP2515_read_register(REC) > 127)
            //                //                {

            //                //                    char* Buffer =
            //                "Error-Passive REC";
            //                //                    Transmit_USART0(10); /*
            //                new line */

            //                //                    while (*Buffer) {
            //                // Transmit_USART0(*Buffer++); } Buffer =
            //                //                    "000";
            //                itoa(MCP2515_read_register(REC),
            //                //                    Buffer, 10); while
            //                (*Buffer) {
            //                // Transmit_USART0(*Buffer++); }

            //                //                    Transmit_USART0(10); /*
            //                new line */
            //                //                }

            //                if (MCP2515_check_for_incoming_message())
            //                {
            //                    MCP2515_message_RX();
            //                    // Transmit_USART0(CAN_RX_msg.id);
            //                }

            //                MCP2515_check_for_interrupts();
            //            }
        }
        return 0;
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif
