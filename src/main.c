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
#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

    uint16_t EEMEM Microcontroller_ID = 0x0002; // default
    uint16_t       microcontroller_id = 0x0000; // default
    uint8_t EEMEM  Module_Adres       = 0x02;   // default
    uint8_t        module_adres       = 0x00;   // default

    /* Configuring the Pin */
    void init_io()
    {
        /* Data Direction Register
         * 0 = ingang
         * 1 = uitgang
         **/
        DDRA = 0b11111111;
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
    void echo_id_Adres()
    {
        /* info Microcontroller can */

        //                         0x1FFFFFFF | 0xFFFF max 29-bit
        CAN_TX_msg.id           = (0x01000000 | microcontroller_id);
        CAN_TX_msg.ext_id       = CAN_EXTENDED_FRAME;
        CAN_TX_msg.rtr          = 0;
        CAN_TX_msg.length       = 8;
        CAN_TX_msg.data_byte[0] = 0;
        CAN_TX_msg.data_byte[1] = module_adres;
        CAN_TX_msg.data_byte[2] = 0;
        CAN_TX_msg.data_byte[3] = 0;
        CAN_TX_msg.data_byte[4] = 0;
        CAN_TX_msg.data_byte[5] = 0;
        CAN_TX_msg.data_byte[6] = 0;
        CAN_TX_msg.data_byte[7] = 0;

        MCP2515_message_TX();
    }

    void set_port(uint8_t uitgang, uint8_t state)
    {
        CAN_TX_msg.id           = 0x600 | module_adres;
        CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
        CAN_TX_msg.rtr          = 0;
        CAN_TX_msg.length       = 3;
        CAN_TX_msg.data_byte[0] = 0x03; /* 1 uitgang */
        CAN_TX_msg.data_byte[1] = uitgang;
        CAN_TX_msg.data_byte[2] = state;
        CAN_TX_msg.data_byte[3] = 0;
        CAN_TX_msg.data_byte[4] = 0;
        CAN_TX_msg.data_byte[5] = 0;
        CAN_TX_msg.data_byte[6] = 0;
        CAN_TX_msg.data_byte[7] = 0;

        if (uitgang < 0x08) /* PORT A */
        {
            if (state) { PORTA |= (1 << uitgang); }
            else
            {
                PORTA &= ~(1 << uitgang);
            }
        }
        else if (uitgang < 0x0F) /* PORT B */
        {
            uitgang -= 0x08;
            if (state) { PORTB |= (1 << uitgang); }
            else
            {
                PORTB &= ~(1 << uitgang);
            }
        }
        else if (uitgang < 0x18) /* PORT C */
        {
            uitgang -= 0x10;
            if (state) { PORTC |= (1 << uitgang); }
            else
            {
                PORTC &= ~(1 << uitgang);
            }
        }
        else if (uitgang < 0x1F) /* PORT D */
        {
            uitgang -= 0x18;
            if (state) { PORTD |= (1 << uitgang); }
            else
            {
                PORTD &= ~(1 << uitgang);
            }
        }
        else
        {
            /* error */
            CAN_TX_msg.length       = 4;
            CAN_TX_msg.data_byte[0] = 0x00; /* error in code µc */
            CAN_TX_msg.data_byte[1] = 0x03; /* 1 uitgang */
            CAN_TX_msg.data_byte[2] = uitgang;
            CAN_TX_msg.data_byte[3] = state;
        }
        MCP2515_message_TX();
    }
    /* */
    int main(void)
    {
        /* init */
        microcontroller_id = eeprom_read_word(&Microcontroller_ID);
        module_adres       = eeprom_read_byte(&Module_Adres);

        init_io();
        PORTC ^= (0x01 << 0);
        init_USART0();

        Test_Transmit_USART0();

        /* can */
        MCP2515_init();

        echo_id_Adres();

        CAN_TX_msg.id           = 20;
        CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
        CAN_TX_msg.rtr          = 0;
        CAN_TX_msg.length       = 0;
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
            if (RX_BufferCount > 15)
            {
                if (RingBuffer_Peek(&RX_Buffer) == 0x0A) /* new line */
                {
                    RingBuffer_Remove(&RX_Buffer); /* new line */
                    CAN_TX_msg.id = RingBuffer_Remove(&RX_Buffer); /* CAN ID */
                    CAN_TX_msg.id = (CAN_TX_msg.id << 8);
                    CAN_TX_msg.id |= RingBuffer_Remove(&RX_Buffer); /* CAN ID */
                    CAN_TX_msg.id = (CAN_TX_msg.id << 8);
                    CAN_TX_msg.id |= RingBuffer_Remove(&RX_Buffer); /* CAN ID */
                    CAN_TX_msg.id = (CAN_TX_msg.id << 8);
                    CAN_TX_msg.id |= RingBuffer_Remove(&RX_Buffer); /* CAN ID */
                    CAN_TX_msg.ext_id       = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.rtr          = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.length       = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.data_byte[0] = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.data_byte[1] = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.data_byte[2] = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.data_byte[3] = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.data_byte[4] = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.data_byte[5] = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.data_byte[6] = RingBuffer_Remove(&RX_Buffer);
                    CAN_TX_msg.data_byte[7] = RingBuffer_Remove(&RX_Buffer);

                    MCP2515_message_TX();
                }
                else
                {
                    RingBuffer_Remove(&RX_Buffer);
                }
            }
            if (MCP2515_check_for_incoming_message())
            {
                if (MCP2515_message_RX())
                {
                    /* global CAN function */
                    if (CAN_RX_msg.id == 0x1ff) { echo_id_Adres(); }

                    if (module_adres == 0x00) /* init module_adres */
                    {
                        if (CAN_RX_msg.id == (0x02000000 | microcontroller_id))
                        {
                            if (CAN_RX_msg.length == 3)
                            {
                                if (CAN_RX_msg.data_byte[0] == 0x01)
                                {
                                    eeprom_update_byte(
                                        &Module_Adres,
                                        CAN_RX_msg.data_byte[1]);
                                    module_adres =
                                        eeprom_read_byte(&Module_Adres);
                                    if (CAN_RX_msg.data_byte[1] == module_adres)
                                    {
                                        /* ok*/
                                        echo_id_Adres();
                                    }
                                    else
                                    {
                                        module_adres = 0x00; /* reset */
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        /* High Priority CAN */

                        /* start van protocol
                         * µc   ID 5       01
                         * DCA  data_byte1 04
                         * port data_byte2 00
                         * var  data_byte3 00-ff
                         */
                        //                        if (CAN_RX_msg.id == 0x501)
                        //                        {
                        //                            if (CAN_RX_msg.length ==
                        //                            3)
                        //                            {
                        //                                if
                        //                                (CAN_RX_msg.data_byte[0]
                        //                                == 0x04)
                        //                                {
                        //                                    if
                        //                                    (CAN_RX_msg.data_byte[1]
                        //                                    == 0x00)
                        //                                    {
                        //                                        PORTA =
                        //                                        CAN_RX_msg.data_byte[2];

                        //                                        char* Buffer =
                        //                                        "set DAC to";
                        //                                        Transmit_USART0(10);
                        //                                        /* new line */
                        //                                        Transmit_USART0(10);
                        //                                        /* new line */

                        //                                        while
                        //                                        (*Buffer)
                        //                                        {
                        //                                            Transmit_USART0(*Buffer++);
                        //                                        }

                        //                                        Buffer =
                        //                                        "0000"; /*
                        //                                        clean buffer
                        //                                        */ itoa(PORTA,
                        //                                        Buffer, 10);
                        //                                        while
                        //                                        (*Buffer)
                        //                                        {
                        //                                            Transmit_USART0(*Buffer++);
                        //                                        }

                        //                                        Transmit_USART0(10);
                        //                                        /* new line */
                        //                                        Transmit_USART0(10);
                        //                                        /* new line */
                        //                                    }
                        //                                }
                        //                            }
                        //                        }
                        /* start van protocol
                         * µc   ID 5       01
                         * command  data_byte0 03
                         * number   data_byte1 00-ff
                         * toestand data_byte2 00-ff
                         */
                        if (CAN_RX_msg.id == (0x500 | module_adres))
                        {
                            if (CAN_RX_msg.length == 3)
                            {
                                if (CAN_RX_msg.data_byte[0] == 0x03)
                                {
                                    set_port(
                                        CAN_RX_msg.data_byte[1],
                                        CAN_RX_msg.data_byte[2]);
                                }
                            }
                        }
                    }

                    Transmit_USART0(10); /* new line */
                    char* Buffer = "CAN ID:  ";
                    while (*Buffer) { Transmit_USART0(*Buffer++); }
                    Buffer = "000000000"; /* clean buffer */
                    ltoa(CAN_RX_msg.id, Buffer, 10);
                    while (*Buffer) { Transmit_USART0(*Buffer++); }
                    Buffer = " DATA: ";
                    while (*Buffer) { Transmit_USART0(*Buffer++); }
                    uint8_t temp = 0;
                    for (; temp < CAN_RX_msg.length; ++temp)
                    {
                        Buffer = "00000000"; /* clean buffer */
                        itoa(CAN_RX_msg.data_byte[temp], Buffer, 10);
                        while (*Buffer) { Transmit_USART0(*Buffer++); }
                        Transmit_USART0(32); /* Spatie */
                    }
                }
            }
        }
        return 0;
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif
