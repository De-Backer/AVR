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

/* Intel_HEX info
 * : BB AAAA RR DDDD DDDD DDDD DDDD DDDD DDDD DDDD DDDD CC
 *
 * :    start code
 * BB   Byte count (meestal is dit 10 => 16 Bytes)
 * AAAA Address
 * RR   Record type
 *      - 00 Data
 *      - 01 End of File
 * DD   Data (*16)
 * CC   Checksum
 *      als je alles optelt van "Byte count" tot en met "Checksum"
 *      moeten de LSB 00 zijn
 *      eg: 00 + 01 + FF = 100 => x 00 is ok
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

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

/* Debug */
#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)
#define DEBUG        __FILE__ ":" TOSTRING(__LINE__)

//uint8_t mcusr __attribute__ ((section (".noinit")));//<= the MCU Status Register
//void getMCUSR(void) __attribute__((naked)) __attribute__((section(".init0")));
//void getMCUSR(void)
//{
//    __asm__ __volatile__ ( "mov %0, r2 \n" : "=r" (mcusr) : );
//}

/* CAN to EEPROM
 * config        CAN_Priority_config
 * module adres  0x00 - 0xff
 * Frame and rtr 0x00
 * length        0x05 - 0x08
 * data EEPROM   0x01
 * data EEPROM   0x01 (read) 0x02 (update & read)
 * data adres_H  0x00 - 0x0f <---(max 4Kbit)
 * data adres_L  0x00 - 0xff
 * data 0        0x00 - 0xff
 * data 1        0x00 - 0xff | see length 6
 * data 2        0x00 - 0xff |            7
 * data 3        0x00 - 0xff |            8
 *
 *  voorstel:
 *  reset module om de "echo_id_Adres();" te starten
 *  dan weet men de:
 *  -microcontroller_id
 *  -module_adres         => module_adres
 *  -MICROCONTROLLER_TYPE
 *  -PROTOCOL_VERSIE
 *  -EE_IO_block          => de ofset in de eeprom
 *  -I_max_block
 *  -O_max_block
 */
/* CAN_Priority_config [module_adres] 0x00 0x00 0x06
 *                                    0x01 0x02 0x00 0x00 <-- EE_MICROCONTROLLER_ID
 *                microcontroller_id_H
 *                microcontroller_id_L
 *
 * CAN_Priority_config [module_adres] 0x00 0x00 0x05
 *                               0x01 0x02 0x00 0x02 <-- EE_MODULE_ADRES
 *                module_adres
 */

/* EEPROM List of parameters */
#define EE_MICROCONTROLLER_ID 0 /* 16 bit */
#define EE_MODULE_ADRES       2

    static uint16_t microcontroller_id = 0x0000; // default
    static uint8_t  module_adres       = 0x00;   // default
    /* ?? => microcontroller_id == 0x0000
     *  yes can stuurt naar id 0 van id 0
     *  de µc die stuurt luistert niet naar zijn eigen bericht!
     * maar niet met 3 µc met id 0 beginnen.
     */

#if defined(__AVR_ATmega1284P__)

#    define MICROCONTROLLER_TYPE 0x01
#    define PROTOCOL_VERSIE      0x01

/* 0x04 [module_adres] 0x00 0x00 0x08
 *                0x01 0x02 0x00 0x03 <-- EE_MICROCONTROLLER_DDRA
 *                DDRA DDRB DDRC DDRD
 *
 *          0x01  0xff  0x00  0x00 0x00
 *          0x02  0x00  0x00  0x00 0x00
 */
#    define EE_MICROCONTROLLER_DDRA 3
#    define EE_MICROCONTROLLER_DDRB 4
#    define EE_MICROCONTROLLER_DDRC 5
#    define EE_MICROCONTROLLER_DDRD 6

/* 0x04 [module_adres]  0x00  0x00  0x08
 *                0x01  0x02  0x00  0x07 <-- EE_MICROCONTROLLER_PORTA
 *                PORTA PORTB PORTC PORTD
 *
 *          0x01  0xff  0x00  0x00 0x00
 *          0x02  0x00  0x00  0x00 0x00
 */
#    define EE_MICROCONTROLLER_PORTA 7
#    define EE_MICROCONTROLLER_PORTB 8
#    define EE_MICROCONTROLLER_PORTC 9
#    define EE_MICROCONTROLLER_PORTD 10
/* Pinout ATmega1284P
 *        | \__/ |
 * 08 PB0-| 1  40|-PA0 00
 * 09 PB1-| 2  39|-PA1 01
 * 0a PB2-| 3  38|-PA2 02
 * 0b PB3-| 4  37|-PA3 03
 * 0c PB4-| 5  36|-PA4 04
 * 0d PB5-| 6  35|-PA5 05
 * 0e PB6-| 7  34|-PA6 06
 * 0f PB7-| 8  33|-PA7 07
 * Reset -| 9  32|-Aref
 * VCC   -|10  31|-GND
 * GND   -|11  30|-AVCC
 * XTAL2 -|12  29|-PC7 17
 * XTAL1 -|13  28|-PC6 16
 * 18 PD0-|14  27|-PC5 15
 * 19 PD1-|15  26|-PC4 14
 * 1a PD2-|16  25|-PC3 13
 * 1b PD3-|17  24|-PC2 12
 * 1c PD4-|18  23|-PC1 11
 * 1d PD5-|19  22|-PC0 10
 * 1e PD6-|20  21|-PD7 1f
*/
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
    /* de all_of functie
     * - masker die de uitgangen "all off" bepaald
     * - toestand waardat de uitgang moet zijn (0 of 1)
     * bv 1:
     * PortA |=(EE_all_of_E_MICROCONTROLLER_PORTA && EE_all_of_MICROCONTROLLER_PORTA);
     * bv 0:
     * PortA &=(EE_all_of_E_MICROCONTROLLER_PORTA && ~EE_all_of_MICROCONTROLLER_PORTA);
     *
     */
#    define EE_all_of_E_MICROCONTROLLER_PORTA 48
#    define EE_all_of_E_MICROCONTROLLER_PORTB 49
#    define EE_all_of_E_MICROCONTROLLER_PORTC 50
#    define EE_all_of_E_MICROCONTROLLER_PORTD 51
#    define EE_all_of_E_EXTENDED_PORT_0A 52
#    define EE_all_of_E_EXTENDED_PORT_0B 53
#    define EE_all_of_E_EXTENDED_PORT_1A 54
#    define EE_all_of_E_EXTENDED_PORT_1B 55
#    define EE_all_of_E_EXTENDED_PORT_2A 56
#    define EE_all_of_E_EXTENDED_PORT_2B 57
#    define EE_all_of_E_EXTENDED_PORT_3A 58
#    define EE_all_of_E_EXTENDED_PORT_3B 59
#    define EE_all_of_E_EXTENDED_PORT_4A 60
#    define EE_all_of_E_EXTENDED_PORT_4B 61
#    define EE_all_of_E_EXTENDED_PORT_5A 62
#    define EE_all_of_E_EXTENDED_PORT_5B 63
#    define EE_all_of_E_EXTENDED_PORT_6A 64
#    define EE_all_of_E_EXTENDED_PORT_6B 65
#    define EE_all_of_E_EXTENDED_PORT_7A 66
#    define EE_all_of_E_EXTENDED_PORT_7B 67
#    define EE_all_of_E_EXTENDED_PORT_8A 68
#    define EE_all_of_E_EXTENDED_PORT_8B 69

#    define EE_all_of_MICROCONTROLLER_PORTA 70
#    define EE_all_of_MICROCONTROLLER_PORTB 71
#    define EE_all_of_MICROCONTROLLER_PORTC 72
#    define EE_all_of_MICROCONTROLLER_PORTD 73
#    define EE_all_of_EXTENDED_PORT_0A 74
#    define EE_all_of_EXTENDED_PORT_0B 75
#    define EE_all_of_EXTENDED_PORT_1A 76
#    define EE_all_of_EXTENDED_PORT_1B 77
#    define EE_all_of_EXTENDED_PORT_2A 78
#    define EE_all_of_EXTENDED_PORT_2B 79
#    define EE_all_of_EXTENDED_PORT_3A 80
#    define EE_all_of_EXTENDED_PORT_3B 81
#    define EE_all_of_EXTENDED_PORT_4A 82
#    define EE_all_of_EXTENDED_PORT_4B 83
#    define EE_all_of_EXTENDED_PORT_5A 84
#    define EE_all_of_EXTENDED_PORT_5B 85
#    define EE_all_of_EXTENDED_PORT_6A 86
#    define EE_all_of_EXTENDED_PORT_6B 87
#    define EE_all_of_EXTENDED_PORT_7A 88
#    define EE_all_of_EXTENDED_PORT_7B 89
#    define EE_all_of_EXTENDED_PORT_8A 90
#    define EE_all_of_EXTENDED_PORT_8B 91

    /* Start reset => new IO laden
     * CAN_Priority_config [module_adres]  0x00  0x00  0x02 0x04 0x04
     */

    /* CAN to out:
     * aantal inputs kopelen aan output
     *  | Adres
     *  | data 0 command
     *  | data 1 number
     *  | data 2 toestand
     *  | naam_output
     *  |== 5 byte
     *  => [Adres] [command] [number] [toestand] [naam_output]
     *      0x01     0x01      0x05      0x01       0x10
     *
     *  +output doet wat?
     *  |naam_output
     *  |function (aan/uit/pwm/...)
     *  |data
     *  == 4 byte
     *  => [naam_output] [function] [PIN-number] [data]
     *         0x10        0x02       0x00-0xff  0x00-0xff
     *
     *         function
     *         - 00 uitgang uit
     *         - 01 uitgang aan
     *         - 02 uitgang togel
     *         - 03 PWM-uitgang
     *         - 04 DAC-uitgang
     *
     **/

#    define EE_IO_block 100 // 0x64
#    define I_max_block 0xff // 0x64 + I_max_block * 5 = ofset for O_from_EEPROM
#    define O_max_block 0xff
    static uint8_t I_from_EEPROM[I_max_block][5]; // [0--255] [Adres / command / number
                                           // / toestand / naam_output]
    static uint8_t O_from_EEPROM[O_max_block]
                         [3]; // [naam_output] [function / PIN-number / data]
    static uint8_t current_O[O_max_block][2];// [PIN-number] [function / data]
#else
#    warning "device type not defined"
#endif

#if PROTOCOL_VERSIE==0x00
#   define CAN_Priority_High_reserve 0x000
#   define CAN_Priority_global       0x100
#   define CAN_Priority_High         0x200
#   define CAN_Priority_USART1       0x300
#   define CAN_Priority_config       0x400
#   define CAN_Priority_set          0x500
#   define CAN_Priority_normale      0x600
#   define CAN_Priority_reserve      0x700
#elif PROTOCOL_VERSIE==0x01
#   define CAN_Priority_High_reserve 0x000
#   define CAN_Priority_global       0x100
#   define CAN_Priority_High         0x200
#   define CAN_Priority_High_USART1  0x300
#   define CAN_Priority_set          0x400
#   define CAN_Priority_normale      0x500
#   define CAN_Priority_USART1       0x600
#   define CAN_Priority_config       0x700
#else
#    warning "PROTOCOL_VERSIE not defined"
#endif

#define S1_Opening_flag 0x7e
#define S1_Frame_type 0x80
#define S1_Frame_length 0x2b
#define S1_Address_field 0xff
#define S1_Control_field 0x03
#define S1_Closing_flag 0x7e

    /* Configuring the Pin */
    static void init_io()
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

    static void CAN_echo_id_Adres(uint8_t data1, uint8_t data2)
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
        CAN_TX_msg.data_byte[3] = EE_IO_block;
        CAN_TX_msg.data_byte[4] = I_max_block;
        CAN_TX_msg.data_byte[5] = O_max_block;
        CAN_TX_msg.data_byte[6] = data1;
        CAN_TX_msg.data_byte[7] = data2;

        MCP2515_message_TX();
    }

    static void USART_echo_id_Adres(uint8_t data1, uint8_t data2)
    {
        /* info Microcontroller can */

        typedef union
        {
            uint32_t long_id;
            uint8_t  int_id[8];
        } ID;
        ID id;
        id.long_id = (0x01000000 | microcontroller_id);
        Transmit_USART0(10); /* new line */
        Transmit_USART0(id.int_id[3]);
        Transmit_USART0(id.int_id[2]);
        Transmit_USART0(id.int_id[1]);
        Transmit_USART0(id.int_id[0]);
        Transmit_USART0(CAN_EXTENDED_FRAME);
        Transmit_USART0(0x00);
        Transmit_USART0(0x08);
        Transmit_USART0(MICROCONTROLLER_TYPE);
        Transmit_USART0(PROTOCOL_VERSIE);
        Transmit_USART0(module_adres);
        Transmit_USART0(EE_IO_block);
        Transmit_USART0(I_max_block);
        Transmit_USART0(O_max_block);
        Transmit_USART0(data1);
        Transmit_USART0(data2);
    }

    static void set_port(uint8_t uitgang, uint8_t state, uint8_t duur)
    {
        CAN_TX_msg.id           = CAN_Priority_normale | module_adres;
        CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
        CAN_TX_msg.rtr          = 0;
        CAN_TX_msg.length       = 4;
        CAN_TX_msg.data_byte[0] = 0x03; /* 1 uitgang */
        CAN_TX_msg.data_byte[1] = uitgang;
        CAN_TX_msg.data_byte[2] = state;
        CAN_TX_msg.data_byte[3] = duur;
        CAN_TX_msg.data_byte[4] = 0;
        CAN_TX_msg.data_byte[5] = 0;
        CAN_TX_msg.data_byte[6] = 0;
        CAN_TX_msg.data_byte[7] = 0;

        //set current pin state and duur
        current_O[uitgang][0]=state;
        current_O[uitgang][1]=duur;

        if (uitgang < 0x08) /* PORT A */
        {
            if (state == 0x01) { PORTA |= (1 << uitgang); }
            else if (state == 0x02)
            {
                PORTA ^= (1 << uitgang);
                CAN_TX_msg.data_byte[2] = (PORTA & (1 << uitgang));
            }
            else
            {
                PORTA &= ~(1 << uitgang);
            }
        }
        else if (uitgang < 0x10) /* PORT B */
        {
            uitgang -= 0x08;
            if (state == 0x01) { PORTB |= (1 << uitgang); }
            else if (state == 0x02)
            {
                PORTB ^= (1 << uitgang);
                CAN_TX_msg.data_byte[2] = (PORTB & (1 << uitgang));
            }
            else
            {
                PORTB &= ~(1 << uitgang);
            }
        }
        else if (uitgang < 0x18) /* PORT C */
        {
            uitgang -= 0x10;
            if (state == 0x01) { PORTC |= (1 << uitgang); }
            else if (state == 0x02)
            {
                PORTC ^= (1 << uitgang);
                CAN_TX_msg.data_byte[2] = (PORTC & (1 << uitgang));
            }
            else
            {
                PORTC &= ~(1 << uitgang);
            }
        }
        else if (uitgang < 0x20) /* PORT D */
        {
            uitgang -= 0x18;
            if (state == 0x01) { PORTD |= (1 << uitgang); }
            else if (state == 0x02)
            {
                PORTD ^= (1 << uitgang);
                CAN_TX_msg.data_byte[2] = (PORTD & (1 << uitgang));
            }
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
            CAN_TX_msg.data_byte[4] = duur;
        }
        MCP2515_message_TX();
    }

    static void build_can_block()
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

    uint8_t meterdata_int()
    {
/* 7e -- S1_Opening_flag
 * 80  0 S1_Frame_type
 * 2b  1 S1_Frame_length
 * ff  2 S1_Address_field
 * 03  3 S1_Control_field
 * 31  4 Meter ID  1
 * 53  5 Meter ID  2
 * 41  6 Meter ID  3
 * 47  7 Meter ID  4
 * 31  8 Meter ID  5
 * 31  9 Meter ID  6
 * 30 10 Meter ID  7
 * 30 11 Meter ID  8
 * 32 12 Meter ID  9
 * 37 13 Meter ID 10
 * 33 14 Meter ID 11
 * 36 15 Meter ID 12
 * 34 16 Meter ID 13
 * 39 17 Meter ID 14
 * 0a 18 Additional information
 * 34 19 Sampling frequency
 * c3 20 Network frequency
 * 37 21
 * 9b 22 Frame sequence number
 * ca 23 Voltage Phase 1
 * 3f 24
 * 00 25 Current Phase 1
 * 00 26
 * 19 27
 * 00 28 Voltage Phase 2
 * 00 29
 * 00 30 Current Phase 2
 * 00 31
 * 00 32
 * 00 33 Voltage Phase 3
 * 00 34
 * 00 35 Current Phase 3
 * 00 36
 * 00 37
 * 00 38 Current neutral
 * 00 39
 * 00 40
 * a3 41 CRC
 * 8a 42
 * 7e -- S1_Opening_flag */
        static uint8_t frame_sequence_nr=0;
        uint8_t return_var=0;
        /* data to be received? */
        if (!(UCSR1A & (1 << RXC1)))
        {
            return 0; /* no */
        }

        /* Get received data from buffer */
        unsigned char data = UDR1;

        if (data!=S1_Opening_flag)
        {
            return 2;
        }
        /* Wait for data to be received */
        while ( !(UCSR1A & (1 << RXC1)))
            ;
        /* dit werkt telegram_ponter==5 */
//        PORTD |= (1 << 5); /* hoog */

        /* Get received data from buffer */
        data = UDR1;


        if (data==S1_Opening_flag) /* dit is de start */
        {
            /* Wait for data to be received */
            while ( !(UCSR1A & (1 << RXC1)))
                ;
            /* Get received data from buffer */
            data = UDR1;
        }

        if (data!=S1_Frame_type)
        {
            return 3;
        }

        while ( !(UCSR1A & (1 << RXC1)))/*  1 */;
        data = UDR1;
        if (data!=S1_Frame_length)
        {
            return 4;
        }

        while ( !(UCSR1A & (1 << RXC1)))/*  2 */;
        data = UDR1;
        if (data!=S1_Address_field)
        {
            return 5;
        }

        while ( !(UCSR1A & (1 << RXC1)))/* 3 */;
        data = UDR1;
        if (data!=S1_Control_field)
        {
            return 6;
        }

        /* 4-17 telegram_ponter = Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/*  4 */;
        data = UDR1; /* Meter ID */

        TIFR1=(1<<OCF1A);//reset timer ctc
        TCNT1=0;
        OCR1A=5;
        TCCR1A=(2<<COM1A0);/* on comp. laag */
//                PORTD &= ~(1 << 5); /* Laag */

        while ( !(UCSR1A & (1 << RXC1)))/*  5 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/*  6 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/*  7 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/*  8 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/*  9 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 10 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 11 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 12 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 13 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 14 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 15 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 16 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 17 */;
        data = UDR1; /* Meter ID */
        while ( !(UCSR1A & (1 << RXC1)))/* 18 */;
        data = UDR1;/* Additional information 3.3.1 */
        if(data!=0x0a){
            /* Single Phase,
             * Per period fixed sampling,
             * 3W,
             * all sample values correct,
             * not measured and/or provided
             * versie 1.0 */
            return_var=data;
        }
//        if(data&0x01){
//            /* 1 = Poly phase */
//        } else {
//            /* 0 = Single Phase*/
//        }
//        if(data&0x02){
//            /* 1 = Per period fixed sampling */
//        } else {
//            /* 0 = Per second fixed sampling */
//        }
//        if(data&0x04){
//            /* 1 = 4W */
//        } else {
//            /* 0 = 3W (and Single phase) */
//        }
//        if(data&0x08){
//            /* 1 = all sample values correct */
//        } else {
//            /* 0 = One or more samples corrupt */
//        }
//        if(data&0x10){
//            /* 1 = measured and provided */
//        } else {
//            /* 0 = not measured and/or provided */
//        }
        while ( !(UCSR1A & (1 << RXC1)))/* 19 */;
        data = UDR1;/* Sampling frequency */
        if(data!=0x34){
            return_var=data;
        }
        union B2I16
        {
           int16_t i;
           uint8_t b[2];
        };
        union B2I16 Network_frequency;
        /* 20-21 telegram_ponter = Network frequency */
        while ( !(UCSR1A & (1 << RXC1)))/* 20 */;
        data = UDR1;/* Network frequency */
        Network_frequency.b[1]=data;
        while ( !(UCSR1A & (1 << RXC1)))/* 21 */;
        data = UDR1;/* Network frequency */
        Network_frequency.b[0]=data;
        while ( !(UCSR1A & (1 << RXC1)))/* 22 */;
        data = UDR1;/* Frame sequence number */
        if(frame_sequence_nr!=data){
            return_var=data-frame_sequence_nr;
            frame_sequence_nr=data;
        }
        union B2I16 Voltage_Phase_1;
        /* 23-26 telegram_ponter = Voltage Phase 1 */
        while ( !(UCSR1A & (1 << RXC1)))/* 23 */;
        data = UDR1;/* Voltage Phase 1 */
        Voltage_Phase_1.b[1]=data;
        while ( !(UCSR1A & (1 << RXC1)))/* 24 */;
        data = UDR1;/* Voltage Phase 1 */
        Voltage_Phase_1.b[0]=data;

        union B2I32
        {
           int32_t i;
           uint8_t b[4];
        };
        union B2I32 Current_Phase_1;
        /* 25-27 telegram_ponter = Current Phase 1 */
        while ( !(UCSR1A & (1 << RXC1)))/* 25 */;
        data = UDR1;/* Current Phase 1 */
        if(data&0x80){
            Current_Phase_1.b[3]=0xff;
        } else {
            Current_Phase_1.b[3]=0x00;
        }
        Current_Phase_1.b[2]=data;
        while ( !(UCSR1A & (1 << RXC1)))/* 26 */;
        data = UDR1;/* Current Phase 1 */
        Current_Phase_1.b[1]=data;
        while ( !(UCSR1A & (1 << RXC1)))/* 27 */;
        data = UDR1;/* Current Phase 1 */
        Current_Phase_1.b[0]=data;

        /* 28-29 telegram_ponter = Voltage Phase 2 */
        while ( !(UCSR1A & (1 << RXC1)))/* 28 */;
        data = UDR1;/* Voltage Phase 2 */
        while ( !(UCSR1A & (1 << RXC1)))/* 29 */;
        data = UDR1;/* Voltage Phase 2 */
        /* 30-32 telegram_ponter = Current Phase 2 */
        while ( !(UCSR1A & (1 << RXC1)))/* 30 */;
        data = UDR1;/* Current Phase 2 */
        while ( !(UCSR1A & (1 << RXC1)))/* 31 */;
        data = UDR1;/* Current Phase 2 */
        while ( !(UCSR1A & (1 << RXC1)))/* 32 */;
        data = UDR1;/* Current Phase 2 */

        /* 33-34 telegram_ponter = Voltage Phase 3 */
        while ( !(UCSR1A & (1 << RXC1)))/* 33 */;
        data = UDR1;/* Voltage Phase 3 */
        while ( !(UCSR1A & (1 << RXC1)))/* 34 */;
        data = UDR1;/* Voltage Phase 3 */
        /* 35-37 telegram_ponter = Current Phase 3 */
        while ( !(UCSR1A & (1 << RXC1)))/* 35 */;
        data = UDR1;/* Current Phase 3 */
        while ( !(UCSR1A & (1 << RXC1)))/* 36 */;
        data = UDR1;/* Current Phase 3 */
        while ( !(UCSR1A & (1 << RXC1)))/* 37 */;
        data = UDR1;/* Current Phase 3 */

        /* 38-40 telegram_ponter = Current neutral */
        while ( !(UCSR1A & (1 << RXC1)))/* 38 */;
        data = UDR1;/* Current neutral */
        while ( !(UCSR1A & (1 << RXC1)))/* 39 */;
        data = UDR1;/* Current neutral */
        while ( !(UCSR1A & (1 << RXC1)))/* 40 */;
        data = UDR1;/* Current neutral */
        while ( !(UCSR1A & (1 << RXC1)))/* 41 */;
        data = UDR1;/* CRC */
        while ( !(UCSR1A & (1 << RXC1)))/* 42 */;
        data = UDR1;/* CRC */
        while ( !(UCSR1A & (1 << RXC1)))/* 43 */;
        data = UDR1;
        if (data!=S1_Closing_flag)
        {
            return 9;
        }
        ++frame_sequence_nr;

        TIFR1=(1<<OCF1A);//reset timer ctc
        TCNT1=0;
        OCR1A=1000;
        TCCR1A=(3<<COM1A0);/* on comp. hoog */

        return return_var;
    }

    static void build_can_meterdata()
    {
        static uint8_t Frame_number=0;
        if (RingBuffer_Peek(&RX1_Buffer) == 0x7e) /* opening/closing flag */
        {
            RingBuffer_Remove(&RX1_Buffer);
            if (RingBuffer_Peek(&RX1_Buffer) == 0x7e) /* opening/closing flag */
            {
                RingBuffer_Remove(&RX1_Buffer);
            }
            if(RingBuffer_Peek(&RX1_Buffer) != 0x80) /* 0x08???? Frame Type */
            {
                CAN_TX_msg.id = CAN_Priority_config | module_adres ; /* CAN ID */
                CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                CAN_TX_msg.rtr          = 0;
                CAN_TX_msg.length       = 5;
                CAN_TX_msg.data_byte[0] = 0xff;
                CAN_TX_msg.data_byte[1] = 0x01;
                CAN_TX_msg.data_byte[2] = 0x80;
                CAN_TX_msg.data_byte[3] = RingBuffer_Remove(&RX1_Buffer);
                CAN_TX_msg.data_byte[4] = RingBuffer_Peek(&RX1_Buffer);
                CAN_TX_msg.data_byte[5] = 0;
                CAN_TX_msg.data_byte[6] = 0;
                CAN_TX_msg.data_byte[7] = 0;

                MCP2515_message_TX();
                return;
            }
            RingBuffer_Remove(&RX1_Buffer);
            if(RingBuffer_Peek(&RX1_Buffer) != 0x2b) /* Frame length */
            {
                CAN_TX_msg.id = CAN_Priority_config | module_adres ; /* CAN ID */
                CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                CAN_TX_msg.rtr          = 0;
                CAN_TX_msg.length       = 5;
                CAN_TX_msg.data_byte[0] = 0xff;
                CAN_TX_msg.data_byte[1] = 0x02;
                CAN_TX_msg.data_byte[2] = 0x2b;
                CAN_TX_msg.data_byte[3] = RingBuffer_Remove(&RX1_Buffer);
                CAN_TX_msg.data_byte[4] = RingBuffer_Peek(&RX1_Buffer);
                CAN_TX_msg.data_byte[5] = 0;
                CAN_TX_msg.data_byte[6] = 0;
                CAN_TX_msg.data_byte[7] = 0;

                MCP2515_message_TX();
                return;
            }
            RingBuffer_Remove(&RX1_Buffer);
            if(RingBuffer_Peek(&RX1_Buffer) != 0xff) /* address field */
            {
                CAN_TX_msg.id = CAN_Priority_config | module_adres ; /* CAN ID */
                CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                CAN_TX_msg.rtr          = 0;
                CAN_TX_msg.length       = 5;
                CAN_TX_msg.data_byte[0] = 0xff;
                CAN_TX_msg.data_byte[1] = 0x03;
                CAN_TX_msg.data_byte[2] = 0xff;
                CAN_TX_msg.data_byte[3] = RingBuffer_Remove(&RX1_Buffer);
                CAN_TX_msg.data_byte[4] = RingBuffer_Peek(&RX1_Buffer);
                CAN_TX_msg.data_byte[5] = 0;
                CAN_TX_msg.data_byte[6] = 0;
                CAN_TX_msg.data_byte[7] = 0;

                MCP2515_message_TX();
                return;
            }
            RingBuffer_Remove(&RX1_Buffer);
            if(RingBuffer_Peek(&RX1_Buffer) != 0x03) /* control field */
            {
                CAN_TX_msg.id = CAN_Priority_config | module_adres ; /* CAN ID */
                CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                CAN_TX_msg.rtr          = 0;
                CAN_TX_msg.length       = 5;
                CAN_TX_msg.data_byte[0] = 0xff;
                CAN_TX_msg.data_byte[1] = 0x04;
                CAN_TX_msg.data_byte[2] = 0x03;
                CAN_TX_msg.data_byte[3] = RingBuffer_Remove(&RX1_Buffer);
                CAN_TX_msg.data_byte[4] = RingBuffer_Peek(&RX1_Buffer);
                CAN_TX_msg.data_byte[5] = 0;
                CAN_TX_msg.data_byte[6] = 0;
                CAN_TX_msg.data_byte[7] = 0;

                MCP2515_message_TX();
                return;
            }
            RingBuffer_Remove(&RX1_Buffer);/* control field */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c1 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c2 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c3 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c4 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c5 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c6 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c7 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c8 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c9 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c10 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c11 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c12 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c13 */
            RingBuffer_Remove(&RX1_Buffer);/* Meter ID c14 */

            uint8_t Additional_info=RingBuffer_Remove(&RX1_Buffer);/* Additional information */
            uint8_t Sampling_frequency=RingBuffer_Remove(&RX1_Buffer);/* Sampling frequency */
            RingBuffer_Remove(&RX1_Buffer);/* Network frequency (MSB) */
            RingBuffer_Remove(&RX1_Buffer);/* Network frequency (LSB) 1 mHz resolution */

            ++Frame_number;
            if (RingBuffer_Peek(&RX1_Buffer) != Frame_number) /* Frame sequence number */
            {
                CAN_TX_msg.id = CAN_Priority_USART1 | module_adres ; /* CAN ID */
                CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                CAN_TX_msg.rtr          = 0;
                CAN_TX_msg.length       = 4;
                CAN_TX_msg.data_byte[0] = Frame_number;
                CAN_TX_msg.data_byte[1] = RingBuffer_Peek(&RX1_Buffer);
                CAN_TX_msg.data_byte[2] = Additional_info;
                CAN_TX_msg.data_byte[3] = Sampling_frequency;
                CAN_TX_msg.data_byte[4] = 0;
                CAN_TX_msg.data_byte[5] = 0;
                CAN_TX_msg.data_byte[6] = 0;
                CAN_TX_msg.data_byte[7] = 0;
                MCP2515_message_TX();

                Frame_number=RingBuffer_Remove(&RX1_Buffer);/* Frame sequence number */
            }
            if (Frame_number==20){
                CAN_TX_msg.id = CAN_Priority_USART1 | module_adres ; /* CAN ID */
                CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                CAN_TX_msg.rtr          = 0;
                CAN_TX_msg.length       = 5;
                CAN_TX_msg.data_byte[0] = RingBuffer_Remove(&RX1_Buffer);/* Voltage Phase 1 (MSB) */
                CAN_TX_msg.data_byte[1] = RingBuffer_Remove(&RX1_Buffer);/* Voltage Phase 1 (LSB) */
                CAN_TX_msg.data_byte[2] = RingBuffer_Remove(&RX1_Buffer);/* Current Phase 1 byte 1 (MSB) */
                CAN_TX_msg.data_byte[3] = RingBuffer_Remove(&RX1_Buffer);/* Current Phase 1 byte 2 */
                CAN_TX_msg.data_byte[4] = RingBuffer_Remove(&RX1_Buffer);/* Current Phase 1 byte 3 (LSB) */
                CAN_TX_msg.data_byte[5] = 0;
                CAN_TX_msg.data_byte[6] = 0;
                CAN_TX_msg.data_byte[7] = 0;

                MCP2515_message_TX();

            } else {
                RingBuffer_Remove(&RX1_Buffer);/* Voltage Phase 1 (MSB) */
                RingBuffer_Remove(&RX1_Buffer);/* Voltage Phase 1 (LSB) */

                RingBuffer_Remove(&RX1_Buffer);/* Current Phase 1 byte 1 (MSB) */
                RingBuffer_Remove(&RX1_Buffer);/* Current Phase 1 byte 2 */
                RingBuffer_Remove(&RX1_Buffer);/* Current Phase 1 byte 3 (LSB) */
            }

            RingBuffer_Remove(&RX1_Buffer);/* Voltage Phase 2 (MSB) */
            RingBuffer_Remove(&RX1_Buffer);/* Voltage Phase 2 (LSB) */

            RingBuffer_Remove(&RX1_Buffer);/* Current Phase 2 byte 1 (MSB) */
            RingBuffer_Remove(&RX1_Buffer);/* Current Phase 2 byte 2 */
            RingBuffer_Remove(&RX1_Buffer);/* Current Phase 2 byte 3 (LSB) */

            RingBuffer_Remove(&RX1_Buffer);/* Voltage Phase 3 (MSB) */
            RingBuffer_Remove(&RX1_Buffer);/* Voltage Phase 3 (LSB) */

            RingBuffer_Remove(&RX1_Buffer);/* Current Phase 3 byte 1 (MSB) */
            RingBuffer_Remove(&RX1_Buffer);/* Current Phase 3 byte 2 */
            RingBuffer_Remove(&RX1_Buffer);/* Current Phase 3 byte 3 (LSB) */

            RingBuffer_Remove(&RX1_Buffer);/* Current neutral byte 1 (MSB) */
            RingBuffer_Remove(&RX1_Buffer);/* Current neutral byte 2 */
            RingBuffer_Remove(&RX1_Buffer);/* Current neutral byte 3 (LSB) */
            RingBuffer_Remove(&RX1_Buffer);/* CRC */
            RingBuffer_Remove(&RX1_Buffer);/* CRC */
        }
        else
        {
            RingBuffer_Remove(&RX1_Buffer);
        }
    }

    static void build_RAM_IO_from_EEPROM()
    {
        wdt_disable(); /* Stop Watchdog Reset */

        uint16_t ee_adres = EE_IO_block;
        uint8_t  temp[5];
        uint8_t  var = 0;
        for (; var < I_max_block; ++var)
        {
            eeprom_read_block((void*) temp, (const void*) ee_adres, 5);
            I_from_EEPROM[var][0] = temp[0]; // Adres
            I_from_EEPROM[var][1] = temp[1]; // command
            I_from_EEPROM[var][2] = temp[2]; // number
            I_from_EEPROM[var][3] = temp[3]; // toestand
            I_from_EEPROM[var][4] = temp[4]; // naam_output
            ee_adres += 5;
        }
        var = 0;
        for (; var < O_max_block; ++var)
        {
            eeprom_read_block((void*) temp, (const void*) ee_adres, 3);
            O_from_EEPROM[var][0] = temp[0]; // function
            O_from_EEPROM[var][1] = temp[1]; // number
            O_from_EEPROM[var][2] = temp[2]; // data
            ee_adres += 3;
        }

        wdt_enable(WDTO_250MS); /* Watchdog Reset after 250mSec */
    }

    static void CAN_EEPROM()
    {
        //        Transmit_USART0(10); /* new line */
        //        char* Buffer = "- EEPROM -";
        //        while (*Buffer) { Transmit_USART0(*Buffer++); }
        if (CAN_RX_msg.data_byte[1] == 0x01) /* read */
        {
            //            Transmit_USART0(10); /* new line */
            //            char* Buffer = "- read -";
            //            while (*Buffer) { Transmit_USART0(*Buffer++); }

            uint8_t length = CAN_RX_msg.length;
            length -= 4;
            uint16_t ee_adres;
            ee_adres = (CAN_RX_msg.data_byte[2] << 8);
            ee_adres |= CAN_RX_msg.data_byte[3];
            uint8_t temp[4];
            temp[0] = 0;
            temp[1] = 0;
            temp[2] = 0;
            temp[3] = 0;
            eeprom_read_block((void*) temp, (const void*) ee_adres, length);
            CAN_TX_msg.id           = (CAN_Priority_config | module_adres);
            CAN_TX_msg.ext_id       = 0;
            CAN_TX_msg.rtr          = 0;
            CAN_TX_msg.length       = length + 4;
            CAN_TX_msg.data_byte[0] = 0x01;
            CAN_TX_msg.data_byte[1] = 0x03;
            CAN_TX_msg.data_byte[2] = CAN_RX_msg.data_byte[2];
            CAN_TX_msg.data_byte[3] = CAN_RX_msg.data_byte[3];
            CAN_TX_msg.data_byte[4] = temp[0];
            CAN_TX_msg.data_byte[5] = temp[1];
            CAN_TX_msg.data_byte[6] = temp[2];
            CAN_TX_msg.data_byte[7] = temp[3];
            MCP2515_message_TX();
        }
        else if (CAN_RX_msg.data_byte[1] == 0x02) /* update & read */
        {
            //            Transmit_USART0(10); /* new line */
            //            char* Buffer = "- update -";
            //            while (*Buffer) { Transmit_USART0(*Buffer++); }

            uint8_t length = CAN_RX_msg.length;
            length -= 4;
            uint16_t ee_adres;
            ee_adres = (uint16_t)(CAN_RX_msg.data_byte[2] << 8);
            ee_adres |= CAN_RX_msg.data_byte[3];
            uint8_t temp[4];
            temp[0] = CAN_RX_msg.data_byte[4];
            temp[1] = CAN_RX_msg.data_byte[5];
            temp[2] = CAN_RX_msg.data_byte[6];
            temp[3] = CAN_RX_msg.data_byte[7];
            eeprom_update_block((const void*) temp, (void*) ee_adres, length);

            /* read */
            length = CAN_RX_msg.length;
            length -= 4;
            ee_adres =  (uint16_t)(CAN_RX_msg.data_byte[2] << 8);
            ee_adres |= CAN_RX_msg.data_byte[3];
            temp[0] = 0;
            temp[1] = 0;
            temp[2] = 0;
            temp[3] = 0;
            eeprom_read_block((void*) temp, (const void*) ee_adres, length);
            CAN_TX_msg.id           = (CAN_Priority_config | module_adres);
            CAN_TX_msg.ext_id       = 0;
            CAN_TX_msg.rtr          = 0;
            CAN_TX_msg.length       = length + 4;
            CAN_TX_msg.data_byte[0] = 0x01;
            CAN_TX_msg.data_byte[1] = 0x03;
            CAN_TX_msg.data_byte[2] = CAN_RX_msg.data_byte[2];
            CAN_TX_msg.data_byte[3] = CAN_RX_msg.data_byte[3];
            CAN_TX_msg.data_byte[4] = temp[0];
            CAN_TX_msg.data_byte[5] = temp[1];
            CAN_TX_msg.data_byte[6] = temp[2];
            CAN_TX_msg.data_byte[7] = temp[3];
            MCP2515_message_TX();

            /*load data in ram */
            if (ee_adres == EE_MICROCONTROLLER_ID)
            {
                microcontroller_id =
                    eeprom_read_word((uint16_t*) EE_MICROCONTROLLER_ID);
            }
            else if (ee_adres == EE_MODULE_ADRES)
            {
                module_adres = eeprom_read_byte((uint8_t*) EE_MODULE_ADRES);
            }
            else if (ee_adres < EE_EXTENDED_DDR0A)
            {
                char* Buffer = "- DO reset -";
                while (*Buffer) { Transmit_USART0(*Buffer++); }
                //                init_io();
            }
            else if (ee_adres > (EE_IO_block - 1))
            {
                build_RAM_IO_from_EEPROM();
            }
            else
            {
                char* Buffer = "- TODO -" DEBUG;
                while (*Buffer) { Transmit_USART0(*Buffer++); }
            }
        }
        else
        {                        /* error in code µc */
            Transmit_USART0(10); /* new line */
            char* Buffer = "- error in code µc -" DEBUG;
            while (*Buffer) { Transmit_USART0(*Buffer++); }
        }
    }

    static void CAN_messag(
        uint8_t module, uint8_t command, uint8_t number, uint8_t toestand)
    {
        uint8_t var = 0;
        for (; var < I_max_block; ++var)
        {
            if (I_from_EEPROM[var][4] < O_max_block)
            {
                if (I_from_EEPROM[var][0] == module)
                {
                    if (I_from_EEPROM[var][1] == command)
                    {
                        if (I_from_EEPROM[var][2] == number)
                        {
                            if (I_from_EEPROM[var][3] == toestand)
                            {
                                if (O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x00)
                                {
                                    /* uitgang uit */
                                    set_port(
                                        O_from_EEPROM[I_from_EEPROM[var][4]][1],
                                        0x00,O_from_EEPROM[I_from_EEPROM[var][4]][2]);
                                }
                                else if (
                                    O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x01)
                                {
                                    /* uitgang aan */
                                    set_port(
                                        O_from_EEPROM[I_from_EEPROM[var][4]][1],
                                        0x01,O_from_EEPROM[I_from_EEPROM[var][4]][2]);
                                }
                                else if (
                                    O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x02)
                                {
                                    /* uitgang togel */
                                    set_port(
                                        O_from_EEPROM[I_from_EEPROM[var][4]][1],
                                        0x02,0x00);
                                }
                                else if (
                                    O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x03)
                                {
                                    /* PWM-uitgang */
                                }
                                else if (
                                    O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x04)
                                {
                                    /* DAC-uitgang */
                                }
                                else if (
                                    O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x05)
                                {
                                    /* Timer uitgang uit in sec */
                                    //pin-number = 0x??
                                    //uit        = 0x00
                                    //duur       = 0x01-0xff
                                }
                                else if (
                                    O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x06)
                                {
                                    /* Timer uitgang aan in sec */
                                }
                                else if (
                                    O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x07)
                                {
                                    /* Timer uitgang uit in minu */
                                }
                                else if (
                                    O_from_EEPROM[I_from_EEPROM[var][4]][0]
                                    == 0x08)
                                {
                                    /* Timer uitgang aan in minu */
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    static uint8_t input_pol(uint8_t input,uint8_t vpin,uint8_t offset)
    {
        uint8_t set_V_input=vpin;
        uint8_t pin_nr=0x00;

        for(;pin_nr<0x08;++pin_nr)
        {
            if(vpin&(0x01<<pin_nr))
            {
                //was 1
                if(!(input&(0x01<<pin_nr))){
                    //is 0
                    set_V_input&=~(0x01<<pin_nr);

                    //send pin
                    CAN_TX_msg.id           = CAN_Priority_normale | module_adres;
                    CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                    CAN_TX_msg.rtr          = 0;
                    CAN_TX_msg.length       = 3;
                    CAN_TX_msg.data_byte[0] = 0x01; /* 1 ingang */
                    CAN_TX_msg.data_byte[1] = pin_nr+offset;/* ingang nr */
                    CAN_TX_msg.data_byte[2] = 0;/* state */
                    CAN_TX_msg.data_byte[3] = 0;
                    CAN_TX_msg.data_byte[4] = 0;
                    CAN_TX_msg.data_byte[5] = 0;
                    CAN_TX_msg.data_byte[6] = 0;
                    CAN_TX_msg.data_byte[7] = 0;
                    MCP2515_message_TX();
                    CAN_messag((CAN_Priority_normale | module_adres), 0x01, pin_nr+offset, 0);//echo to self
                }
            } else {
                //was 0
                if(input&(0x01<<pin_nr)){
                    //is 1
                    set_V_input|=(0x01<<pin_nr);

                    //send pin
                    CAN_TX_msg.id           = CAN_Priority_normale | module_adres;
                    CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                    CAN_TX_msg.rtr          = 0;
                    CAN_TX_msg.length       = 3;
                    CAN_TX_msg.data_byte[0] = 0x01; /* 1 ingang */
                    CAN_TX_msg.data_byte[1] = pin_nr+offset;/* ingang nr */
                    CAN_TX_msg.data_byte[2] = 1;/* state */
                    CAN_TX_msg.data_byte[3] = 0;
                    CAN_TX_msg.data_byte[4] = 0;
                    CAN_TX_msg.data_byte[5] = 0;
                    CAN_TX_msg.data_byte[6] = 0;
                    CAN_TX_msg.data_byte[7] = 0;
                    MCP2515_message_TX();
                    CAN_messag((CAN_Priority_normale | module_adres), 0x01, pin_nr+offset, 1);//echo to self
                }
            }
        }
        return set_V_input;
    }

    /** main */
    int main(void)
    {
        {
            /* The MCU Status Register provides information on which reset
             * source caused an MCU reset.
             * 0x01 Power-on
             * 0x02 External
             * 0x04 Brown-out
             * 0x05 Brown-out + Power-on
             * 0x06 Brown-out + External
             * 0x07 Brown-out + External + Power-on
             * 0x08 Watchdog
             * 0x10 JTAG
             */
            uint8_t mcusr;
            __asm__ __volatile__ ( "mov %0, r2 \n" : "=r" (mcusr) : );/* MCUSR from bootloader */
            // int8_t Reset_caused_by = MCUSR;
            // MCUSR                  = 0x00; /* Reset the MCUSR */

            /* init */
            microcontroller_id =
                eeprom_read_word((uint16_t*) EE_MICROCONTROLLER_ID);
            module_adres = eeprom_read_byte((uint8_t*) EE_MODULE_ADRES);

            init_io();
            init_USART0();
            init_USART1();

            /* 0x20 => deze µc is gereset */
            USART_echo_id_Adres(0x20 | mcusr, 0x00);

            /* can */
            MCP2515_init();

            CAN_echo_id_Adres(mcusr, 0x00);
        }

        CAN_TX_msg.id           = 0;
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

        build_RAM_IO_from_EEPROM();

        /* adc */
        /* DIDR0 – Digital Input Disable Register 0 */
        DIDR0 = 0xff;

        /* ADC Multiplexer Selection Register */
            /* ADLAR ADC Left Adjust Result */
            ADMUX = (0x01 << ADLAR);
            /*AVCC with external capacitor at AREF pin */
            ADMUX |=(0x01 << REFS0);

        /* ADC Control and Status Register A */
            /* ADPS2:0: ADC Prescaler Select Bits 156.25KHz sample rate @ 20MHz */
            ADCSRA = (0x01 << ADPS2) | (0x01 << ADPS1) | (0x01 << ADPS0);
            /* ADEN: ADC Enable */
            ADCSRA |= (0x01 << ADEN);
            /* Start Conversion */
            ADCSRA |= (0x01 << ADSC);
        uint8_t v_adc_pin[8];

        wdt_enable(WDTO_250MS); /* Watchdog Reset after 250mSec */

        /* Can Watchdog Timer3 */
        TCCR3A=0x00;
        TCCR3B=0x05;/*CSn2 en CSn0*/
        TCCR3C=0x00;
        uint8_t Can_watchdog=0;
        uint8_t poling=0;

        /* input */
        uint8_t v_pinA = (PINA & ~DDRA);
        uint8_t v_pinB = (PINB & ~DDRB);
        uint8_t v_pinC = (PINC & ~DDRC);
        uint8_t v_pinD = (PIND & ~DDRD);
        v_pinA=input_pol(v_pinA,~v_pinA, 0x00);
        v_pinB=input_pol(v_pinB,~v_pinB, 0x08);
        v_pinC=input_pol(v_pinC,~v_pinC, 0x10);
        v_pinD=input_pol(v_pinD,~v_pinD, 0x18);

        /* output zet op de can bus bij opstart */
        uint8_t output=0;
        for (uint8_t pin_nr=0;pin_nr<0x08;++pin_nr,++output) {
            if(0x01&&(DDRA<<pin_nr))/* is een output */
                set_port(output,0x01&&(PORTA<<pin_nr),0);
        }
        wdt_reset(); /* Reset Watchdog timer*/
        for (uint8_t pin_nr=0;pin_nr<0x08;++pin_nr,++output) {
            if(0x01&&(DDRB<<pin_nr))
                set_port(output,0x01&&(PORTB<<pin_nr),0);
        }
        wdt_reset(); /* Reset Watchdog timer*/
        for (uint8_t pin_nr=0;pin_nr<0x08;++pin_nr,++output) {
            if(0x01&&(DDRC<<pin_nr))
                set_port(output,0x01&&(PORTC<<pin_nr),0);
        }
        wdt_reset(); /* Reset Watchdog timer*/
        for (uint8_t pin_nr=0;pin_nr<0x08;++pin_nr,++output) {
            if(0x01&&(DDRD<<pin_nr))
                set_port(output,0x01&&(PORTD<<pin_nr),0);
        }
        wdt_reset(); /* Reset Watchdog timer*/

//        TCCR1A=(1<<COM1A0);
//        PORTD |= (1 << 5);//stop puls
//        TCCR1B=(1<<WGM12);
        TCCR1B=(1<<WGM12);
        TCCR1B|=(1<<CS10); //clk/1
        for (;;)
        {
            /* loop */

//            meterdata();//test
            meterdata_int();

//            Receive_USART1();
            if(TCNT3>65000){

                /* test */
//                if(temp>0){
//                    CAN_TX_msg.id           = CAN_Priority_normale | module_adres;
//                    CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
//                    CAN_TX_msg.rtr          = 0;
//                    CAN_TX_msg.length       = 8;
//                    CAN_TX_msg.data_byte[0] = 0xFF;
//                    CAN_TX_msg.data_byte[1] = temp;
//                    MCP2515_message_TX();
//                }
                /* test */

                ++Can_watchdog;
                if(Can_watchdog>4){
                    CAN_TX_msg.id           = 0x111;
                    CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                    CAN_TX_msg.rtr          = 1;
                    CAN_TX_msg.length       = 0;
                    CAN_TX_msg.data_byte[0] = 0;
                    CAN_TX_msg.data_byte[1] = 0;
                    CAN_TX_msg.data_byte[2] = 0;
                    CAN_TX_msg.data_byte[3] = 0;
                    CAN_TX_msg.data_byte[4] = 0;
                    CAN_TX_msg.data_byte[5] = 0;
                    CAN_TX_msg.data_byte[6] = 0;
                    CAN_TX_msg.data_byte[7] = 0;
                    MCP2515_message_TX();
                }
                if(Can_watchdog>10){/* 1 ≃~ 3sec*/

                    CAN_TX_msg.id           = (CAN_Priority_config | module_adres);
                    CAN_TX_msg.ext_id       = CAN_STANDARD_FRAME;
                    CAN_TX_msg.rtr          = 0;
                    CAN_TX_msg.length       = 8;
                    CAN_TX_msg.data_byte[0] = 0x00;
                    CAN_TX_msg.data_byte[1] = 0x00;
                    CAN_TX_msg.data_byte[2] = Can_watchdog;
                    CAN_TX_msg.data_byte[3] = MCP2515_check_for_incoming_message();
                    CAN_TX_msg.data_byte[4] = MCP2515_message_RX();
                    CAN_TX_msg.data_byte[5] = 0;
                    CAN_TX_msg.data_byte[6] = 0;
                    CAN_TX_msg.data_byte[7] = 0;
                    MCP2515_message_TX();

                    /* 0x40 => deze µc word gereset */
                    CAN_echo_id_Adres(0x40, 0x40);
                    USART_echo_id_Adres(0x40, 0x40);

                    wdt_enable(WDTO_15MS);/* reset MCU */

                    for (;;){}
                }

                TCNT3=0;
            }
            wdt_reset(); /* Reset Watchdog timer*/

//            Receive_USART1();

            if (MCP2515_check_for_incoming_message())
            {
                if (MCP2515_message_RX())
                {
                     /* Reset Watchdog timer can */
                    Can_watchdog=0;

                    /*  global */
                    if (CAN_RX_msg.id == 0x1ff)
                    {
                        CAN_echo_id_Adres(0x00, 0x00);
                    }

                    /*  config */
                    else if (CAN_RX_msg.id == (CAN_Priority_config | module_adres))
                    {
                        if (CAN_RX_msg.data_byte[0] == 0x01) /* EEPROM */
                        {
                            CAN_EEPROM();
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
                        else if (CAN_RX_msg.data_byte[0] == 0x04) /* reset */
                        {
                            Transmit_USART0(10); /* new line */
                            char* Buffer = "- reset -";
                            while (*Buffer) { Transmit_USART0(*Buffer++); }
                            if (CAN_RX_msg.data_byte[1] == 0x04)
                            {
                                wdt_enable(WDTO_15MS); /* Watchdog Reset after
                                                          15mSec */

                                /* 0x40 => deze µc word gereset */
                                CAN_echo_id_Adres(0x40, 0x40);
                                USART_echo_id_Adres(0x40, 0x40);

                                for (;;)
                                { /* Watchdog Reset */
                                }
                            }
                        }
                        else
                        {                        /* error in code µc */

                        }
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
