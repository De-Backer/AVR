#include "../include/usart.h"

#include <stdlib.h>

static void init_USART0_baud_rate_2000000bps()
{
    /* set double speed mode */
    UCSR0A = (1 << U2X0);
    /* set the baud rate */
    UBRR0H = 0;
    UBRR0L = 0;
}

static void init_USART1_baud_rate_2000000bps()
{
    /* set double speed mode */
    UCSR1A = (1 << U2X1);
    /* set the baud rate */
    UBRR1H = 0;
    UBRR1L = 0;
}
/** \brief init_de USART0
 * Configuring USART0
 * Frame format :
 * - One start bit
 * - 8 data bits
 * - No parity bit
 * - one stop bits
 * baud rate: 2000000bps
 * 0% Error
 **/
inline void init_USART0()
{
    init_USART0_baud_rate_2000000bps();

    /* Enable  transmitter */
    /* Enable receiver */
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    /* Set frame format: 8data, 1 stop bit */
    UCSR0C = (3 << UCSZ00);
    RingBuffer_InitBuffer(&RX_Buffer, RX_BufferData, sizeof(RX_BufferData));
}

/** \brief init_de USART1
 * Configuring USART0
 * Frame format :
 * - One start bit
 * - 8 data bits
 * - No parity bit
 * - one stop bits
 * baud rate: 2000000bps
 * 0% Error
 **/
inline void init_USART1()
{
    init_USART1_baud_rate_2000000bps();

    /* Enable receiver */
    UCSR1B = (1 << RXEN1);
    /* Set frame format: 8data, 1 stop bit */
    UCSR1C = (3 << UCSZ10);
    RingBuffer_InitBuffer(&RX1_Buffer, RX1_BufferData, sizeof(RX1_BufferData));
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
        static uint8_t punt=0;
        unsigned char data = UDR1;
        if (data==0x7e && punt==1) {
            //stop puls
//            for (int var = 0; var < 250; ++var) {
//                __asm__ __volatile__ ("nop");
//                __asm__ __volatile__ ("nop");
//                __asm__ __volatile__ ("nop");
//                __asm__ __volatile__ ("nop");
//                __asm__ __volatile__ ("nop");
//            }
//            PORTD &= ~(1 << 5);//start puls
//            PORTD |= (1 << 5);
            punt=0;

        }else if (data==0x7e){
            //start puls
            punt=0;
            PORTD |= (1 << 5);
        } else if (data==0x80 && punt==0) {
            punt=1;
            PORTD |= (1 << 5);
            for (int var = 0; var < 100; ++var) {
                __asm__ __volatile__ ("nop");
                __asm__ __volatile__ ("nop");
                __asm__ __volatile__ ("nop");
                __asm__ __volatile__ ("nop");
                __asm__ __volatile__ ("nop");
                __asm__ __volatile__ ("nop");
            }
            PORTD &= ~(1 << 5);

//            while ((TIFR1&(1<<OCF1A))==0);
//            TIFR1=(1<<OCF1A);//reset timer ctc
//            OCR1A=200;
//            TCCR1B=(1<<WGM12);
//            TCCR1A=(2<<COM1A0);/* on comp. laag */
//            TCCR1B|=(1<<CS10); //clk/1
//            while ((TIFR1&(1<<OCF1A))==0);
//            TIFR1=(1<<OCF1A);//reset timer ctc
//            OCR1A=800;
//            TCCR1B=(1<<WGM12);
//            TCCR1A=(3<<COM1A0);/* on comp. hoog */
//            TCCR1B|=(1<<CS10); //clk/1
        }
    }
}
