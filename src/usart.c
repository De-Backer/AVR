#include "../include/usart.h"

#include <stdlib.h>

static void init_USART0_baud_rate_2500000bps()
{
    /* set double speed mode */
    UCSR0A = (1 << U2X0);
    /* set the baud rate */
    UBRR0H = 0;
    UBRR0L = 0;
}
static void init_USART0_baud_rate_1250000bps()
{
    /* set the baud rate */
    UBRR0H = 0;
    UBRR0L = 0;
}
static void init_USART0_baud_rate_250000bps()
{
    /* set the baud rate */
    UBRR0H = 0;
    UBRR0L = 4;
}
static void init_USART0_baud_rate_9600bps()
{
    /* set the baud rate */
    UBRR0H = 0;
    UBRR0L = 129;
}
static void init_USART0_baud_rate_2400bps()
{
    /* set the baud rate */
    UBRR0H = 2;
    UBRR0L = 8;
}
/** \brief init_de USART0
 * Configuring USART0
 * Frame format :
 * - One start bit
 * - 8 data bits
 * - No parity bit
 * - one stop bits
 * baud rate: 2500000bps
 * 0% Error
 **/
inline void init_USART0()
{
    init_USART0_baud_rate_2500000bps();

    /* Enable  transmitter */
    /* Enable receiver */
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    /* Set frame format: 8data, 1 stop bit */
    UCSR0C = (3 << UCSZ00);
    RingBuffer_InitBuffer(&RX_Buffer, RX_BufferData, sizeof(RX_BufferData));
}

/** \brief init_de USART0
 * Configuring USART0
 * Frame format :
 * - One start bit
 * - 8 data bits
 * - No parity bit
 * - one stop bits
 * baud rate: enum Baud_rate Baud
 **/
inline void init_USART1(uint8_t Baud)
{
#if F_CPU==20000000
uint16_t set_ubrr[15]={520,520,129,86,64,86,64,42,32,10,10,4,4,0,0};
uint8_t  set_U2Xn[15]={  0,  1,  0, 0, 0, 1, 1, 1, 1, 0, 1,0,1,0,1};
#else
#endif
    if(set_U2Xn[Baud]==1){
        /* set double speed mode */
        UCSR1A = (1 << U2X0);
    } else {
        /* set double speed mode Not */
        UCSR1A &= ~(1 << U2X0);
    }
    /* set the baud rate */
    UBRR1H = (uint8_t)(set_ubrr[Baud]>>8);
    UBRR1L = (uint8_t)set_ubrr[Baud];

    /* Enable  transmitter */
    /* Enable receiver */
    UCSR1B = (1 << TXEN1) | (1 << RXEN1);
    /* Set frame format: 8data, 1 stop bit */
    UCSR1C = (3 << UCSZ10);
    RingBuffer_InitBuffer(&RX_Buffer_1, RX_BufferData_1, sizeof(RX_BufferData_1));
}

/** \brief zend_USART0
 * */
inline void Transmit_USART0(unsigned char data)
{
    /* Wait for empty transmit buffer */
    while (! (UCSR0A & (1 << UDRE0)))
        ;
    /* Put data into buffer, sends the data */
    UDR0 = data;
}

/** \brief zend_USART1
 * */
inline void Transmit_USART1(unsigned char data)
{
    /* Wait for empty transmit buffer */
    while (! (UCSR1A & (1 << UDRE1)))
        ;
    /* Put data into buffer, sends the data */
    UDR1 = data;
}

/**
 * @brief Test_Transmit_USART0
 * zent a test data
 */
void Test_Transmit_USART0()
{
    char* Buffer = "Test USART0";
    Transmit_USART0(10); /* new line */
    Transmit_USART0(10); /* new line */

    while (*Buffer) { Transmit_USART0(*Buffer++); }

    Transmit_USART0(10); /* new line */
    Transmit_USART0(10); /* new line */
}

void Receive_USART0()
{
    if (UCSR0A & (1 << RXC0))
    {
        unsigned char data = UDR0;
        if (RingBuffer_IsFull(&RX_Buffer))
        {
            /* Buffer Is Full */

            char* Buffer = "ERROR RX Buffer ";

            Transmit_USART0(10); /* new line */
            Transmit_USART0(10); /* new line */

            while (*Buffer) { Transmit_USART0(*Buffer++); }

            Transmit_USART0(data);

            Transmit_USART0(10); /* new line */
            Transmit_USART0(10); /* new line */

            return;
        }

        RingBuffer_Insert(&RX_Buffer, data);
    }
}

void Receive_USART1()
{
    if (UCSR1A & (1 << RXC1))
    {
        unsigned char data = UDR1;
        if (RingBuffer_IsFull(&RX_Buffer_1))
        {
            /* Buffer Is Full */

            char* Buffer = "ERROR RX Buffer ";

            Transmit_USART0(10); /* new line */
            Transmit_USART0(10); /* new line */

            while (*Buffer) { Transmit_USART0(*Buffer++); }

            Transmit_USART0(data);

            Transmit_USART0(10); /* new line */
            Transmit_USART0(10); /* new line */

            return;
        }

        RingBuffer_Insert(&RX_Buffer_1, data);
    }
}
