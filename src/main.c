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
#include "../include/usart.h"

#include <avr/io.h>
//#include <avr/wdt.h>
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

    /* */
    int main(void)
    {
        /* init */
        init_io();
        PORTC ^= (0x01 << 0);
        init_USART0();

        Test_Transmit_USART0();

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

        //        wdt_enable(WDTO_250MS); /* Watchdog Reset after 250mSec */
        for (;;)
        {
            /* loop */;
            //            wdt_reset(); /* Reset Watchdog timer*/
            Receive_USART0();

            /* verwerk de RX USART0 buffer */
            int8_t RX_BufferCount = RingBuffer_GetCount(&RX_Buffer);
            if (RX_BufferCount > 0)
            {
                CAN_TX_msg.id     = 50;
                CAN_TX_msg.length = 0;
                while (RX_BufferCount--)
                {

                    CAN_TX_msg.data_byte[CAN_TX_msg.length++] =
                        RingBuffer_Remove(&RX_Buffer);
                    if (CAN_TX_msg.length > 7
                        || (RX_BufferCount == 0 && CAN_TX_msg.length > 0))
                    {
                        MCP2515_message_TX();
                        uint8_t temp = 0;
                        for (; temp < CAN_TX_msg.length; ++temp)
                        { Transmit_USART0(CAN_TX_msg.data_byte[temp]); }

                        ++CAN_TX_msg.id;
                        CAN_TX_msg.length = 0;
                    }
                }
            }
            if (MCP2515_check_for_incoming_message())
            {
                if (MCP2515_message_RX())
                {
                    uint8_t temp = 0;
                    for (; temp < CAN_RX_msg.length; ++temp)
                    { Transmit_USART0(CAN_RX_msg.data_byte[temp]); }
                }
            }
        }
        return 0;
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif
