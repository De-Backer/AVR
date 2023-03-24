
#ifndef _usart_h__
#define _usart_h__

#ifdef __cplusplus
extern "C"
{
#endif

#include "../include/RingBuffer.h"

#include <avr/io.h>
#include <util/delay.h>

enum Baud_rate {B2400bps,
                B4800bps,
                B9600bps,
                B14400bps,
                B19200bps,
                B28800bps,
                B38400bps,
                B57600bps,
                B76800bps,
                B115200bps,
                B230400bps,
                B250000bps,
                B500000bps,
                B1250000bps,
                B2500000bps
               };

    void init_USART0();
    void init_USART1(uint8_t Baud);

    void Transmit_USART0(unsigned char data);
    void Transmit_USART1(unsigned char data);
    void         Test_Transmit_USART0();
    void         Receive_USART0();
    void         Receive_USART1();
    extern RingBuffer_t RX_Buffer;
    extern uint8_t      RX_BufferData[20];

    extern RingBuffer_t RX_Buffer_1;
    extern uint8_t      RX_BufferData_1[10];

#    ifdef __cplusplus
} // extern "C"
#endif

#endif
