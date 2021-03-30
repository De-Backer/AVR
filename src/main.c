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

/* EEPROM List of parameters */
#define EE_MICROCONTROLLER_ID 0 /* 16 bit */
#define EE_MODULE_ADRES       2

    uint16_t microcontroller_id = 0x0000; // default
    uint8_t  module_adres       = 0x00;   // default

#if defined(__AVR_ATmega1284P__)

#    define MICROCONTROLLER_TYPE 0x01
#    define PROTOCOL_VERSIE      0x00

#    define EE_MICROCONTROLLER_DDRA 3
#    define EE_MICROCONTROLLER_DDRB 4
#    define EE_MICROCONTROLLER_DDRC 5
#    define EE_MICROCONTROLLER_DDRD 6

#    define EE_MICROCONTROLLER_PORTA 7
#    define EE_MICROCONTROLLER_PORTB 8
#    define EE_MICROCONTROLLER_PORTC 9
#    define EE_MICROCONTROLLER_PORTD 10
/* CAN to out:
 * aantal inputs kopelen aan output
 *  | Adres
 *  | data 0 command
 *  | data 1 number
 *  | data 2 toestand
 *  |== 4 byte
 *  +output doet wat?
 *  |function (aan/uit/pwm/...)
 *  |data
 *  == 6 byte tot
 *
 *  [Priority and Adres] [data 0]
 **/
#    define EE_EXTENDED_DDR0A 11
#    define EE_EXTENDED_DDR0B 12
#    define EE_EXTENDED_DDR1A 13
#    define EE_EXTENDED_DDR1B 14
#    define EE_EXTENDED_DDR2A 15
#    define EE_EXTENDED_DDR2B 16
#    define EE_EXTENDED_DDR3A 17
#    define EE_EXTENDED_DDR3B 18
#    define EE_EXTENDED_DDR4A 19
#    define EE_EXTENDED_DDR4B 20
#    define EE_EXTENDED_DDR5A 21
#    define EE_EXTENDED_DDR5B 22
#    define EE_EXTENDED_DDR6A 23
#    define EE_EXTENDED_DDR6B 24
#    define EE_EXTENDED_DDR7A 25
#    define EE_EXTENDED_DDR7B 26
#    define EE_EXTENDED_DDR8A 27
#    define EE_EXTENDED_DDR8B 28

#    define EE_EXTENDED_PORT_0A 29
#    define EE_EXTENDED_PORT_0B 30
#    define EE_EXTENDED_PORT_1A 31
#    define EE_EXTENDED_PORT_1B 32
#    define EE_EXTENDED_PORT_2A 34
#    define EE_EXTENDED_PORT_2B 35
#    define EE_EXTENDED_PORT_3A 36
#    define EE_EXTENDED_PORT_3B 37
#    define EE_EXTENDED_PORT_4A 38
#    define EE_EXTENDED_PORT_4B 39
#    define EE_EXTENDED_PORT_5A 40
#    define EE_EXTENDED_PORT_5B 41
#    define EE_EXTENDED_PORT_6A 42
#    define EE_EXTENDED_PORT_6B 43
#    define EE_EXTENDED_PORT_7A 44
#    define EE_EXTENDED_PORT_7B 45
#    define EE_EXTENDED_PORT_8A 46
#    define EE_EXTENDED_PORT_8B 47
#else
#    warning "device type not defined"
#endif

    /* Configuring the Pin */
    void init_io()
    {
        if (module_adres == 0xff)
        {
            /* no adres => all tri-state */

            /* Data Direction Register
             * 0 = ingang
             * 1 = uitgang
             **/
            DDRA = 0x00;
            DDRB = 0x00;
            DDRC = 0x00;
            DDRD = 0x00;
            /* Data Register
             * 0 = laag (uitgang) / tri-state (ingang)
             * 1 = hoog (uitgang) / pull up (ingang)
             **/
            PORTA = 0x00;
            PORTB = 0x00;
            PORTC = 0x00;
            PORTD = 0x00;
            return;
        }
        DDRA = eeprom_read_byte((uint8_t*) EE_MICROCONTROLLER_DDRA);
        DDRB = eeprom_read_byte((uint8_t*) EE_MICROCONTROLLER_DDRB);
        DDRC = eeprom_read_byte((uint8_t*) EE_MICROCONTROLLER_DDRC);
        DDRD = eeprom_read_byte((uint8_t*) EE_MICROCONTROLLER_DDRD);

        PORTA = eeprom_read_byte((uint8_t*) EE_MICROCONTROLLER_PORTA);
        PORTB = eeprom_read_byte((uint8_t*) EE_MICROCONTROLLER_PORTB);
        PORTC = eeprom_read_byte((uint8_t*) EE_MICROCONTROLLER_PORTC);
        PORTD = eeprom_read_byte((uint8_t*) EE_MICROCONTROLLER_PORTD);
    }

    void echo_id_Adres()
    {
        /* info Microcontroller can */

        //                         0x1FFFFFFF | 0xFFFF max 29-bit
        CAN_TX_msg.id           = (0x01000000 | microcontroller_id);
        CAN_TX_msg.ext_id       = CAN_EXTENDED_FRAME;
        CAN_TX_msg.rtr          = 0;
        CAN_TX_msg.length       = 8;
        CAN_TX_msg.data_byte[0] = MICROCONTROLLER_TYPE;
        CAN_TX_msg.data_byte[1] = PROTOCOL_VERSIE;
        CAN_TX_msg.data_byte[2] = module_adres;
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

    void build_can_block()
    {
        if (RingBuffer_Peek(&RX_Buffer) == 0x0A) /* new line */
        {
            RingBuffer_Remove(&RX_Buffer);                 /* new line */
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

    /** main */
    int main(void)
    {
        /* init */
        microcontroller_id =
            eeprom_read_word((uint16_t*) EE_MICROCONTROLLER_ID);
        module_adres = eeprom_read_byte((uint8_t*) EE_MODULE_ADRES);

        init_io();
        //        PORTC ^= (0x01 << 0);
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
            if (RX_BufferCount > 15) { build_can_block(); }
            if (MCP2515_check_for_incoming_message())
            {
                if (MCP2515_message_RX())
                {
                    /* 1 global */
                    if (CAN_RX_msg.id == 0x1ff) { echo_id_Adres(); }

                    /* 3 reserve */

                    /* 4 config */
                    else if (CAN_RX_msg.id == (0x400 | module_adres))
                    {
                        if (CAN_RX_msg.data_byte[0] == 0x01) /* EEPROM */
                        {

                            Transmit_USART0(10); /* new line */
                            char* Buffer = "- EEPROM -";
                            while (*Buffer) { Transmit_USART0(*Buffer++); }
                        }
                        else if (CAN_RX_msg.data_byte[0] == 0x02) /* RAM */
                        {
                            Transmit_USART0(10); /* new line */
                            char* Buffer = "- RAM -";
                            while (*Buffer) { Transmit_USART0(*Buffer++); }
                        }
                        else if (CAN_RX_msg.data_byte[0] == 0x03) /* ROM */
                        {
                            Transmit_USART0(10); /* new line */
                            char* Buffer = "- ROM -";
                            while (*Buffer) { Transmit_USART0(*Buffer++); }
                        }
                        else
                        {                        /* error in code µc */
                            Transmit_USART0(10); /* new line */
                            char* Buffer = "- error in code µc -";
                            while (*Buffer) { Transmit_USART0(*Buffer++); }
                        }
                    }

                    /* 5 set */
                    else if (CAN_RX_msg.id == (0x500 | module_adres))
                    {
                        /* start van protocol
                         * µc   ID 5       01
                         * command  data_byte0 03
                         * number   data_byte1 00-ff
                         * toestand data_byte2 00-ff
                         */
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
                    /* 7 reserve */

                    /* extended */
                    else if (CAN_RX_msg.id == (0x02000000 | microcontroller_id))
                    {
                        if (CAN_RX_msg.length == 3)
                        {
                            if (CAN_RX_msg.data_byte[0] == 0x01)
                            {
                                eeprom_update_byte(
                                    (uint8_t*) EE_MODULE_ADRES,
                                    CAN_RX_msg.data_byte[1]);
                                module_adres = eeprom_read_byte(
                                    (uint8_t*) EE_MODULE_ADRES);
                                if (CAN_RX_msg.data_byte[1] != module_adres)
                                {
                                    module_adres = 0x00; /* reset */
                                }
                            }
                            else if (CAN_RX_msg.data_byte[0] == 0x02)
                            {
                                uint16_t temp =
                                    (uint16_t)(CAN_RX_msg.data_byte[1] << 8);
                                temp |= CAN_RX_msg.data_byte[2];
                                eeprom_update_word(
                                    (uint16_t*) EE_MICROCONTROLLER_ID,
                                    temp);
                                microcontroller_id = eeprom_read_word(
                                    (uint16_t*) EE_MICROCONTROLLER_ID);
                                if (temp != microcontroller_id)
                                {
                                    microcontroller_id = 0x0000; /* reset */
                                }
                            }
                            echo_id_Adres();
                        }
                    }

                    /* 2 High */
                    /* 6 normale */
                    else if ((CAN_RX_msg.id & 0x200) || (CAN_RX_msg.id & 0x600))
                    {
                        /* test CAN id on list */

                        Transmit_USART0(10); /* new line */
                        char* Buffer = "- test CAN id on list -";
                        while (*Buffer) { Transmit_USART0(*Buffer++); }
                    }

                    Transmit_USART0(10); /* new line */
                    char* Buffer = "CAN: ";
                    while (*Buffer) { Transmit_USART0(*Buffer++); }
                    Buffer = "000000000"; /* clean buffer */
                    ltoa(CAN_RX_msg.id, Buffer, 10);
                    while (*Buffer) { Transmit_USART0(*Buffer++); }
                    Transmit_USART0(32); /* Spatie */
                    uint8_t temp = 0;
                    for (; temp < CAN_RX_msg.length; ++temp)
                    {
                        Buffer = "000000000"; /* clean buffer */
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
