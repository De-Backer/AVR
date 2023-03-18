#include "../include/mcp23s17.h"
#include <util/delay.h>

/* IO prind
 *
 *   ||||||||||
 *   ===========
 *  prind 0  C1
 *     J1  J2
 * 20 -A0 GND-
 * 21 -A1 VCC-
 * 22 -A2  B7- 2F
 * 23 -A3  B6- 2E
 * 24 -A4  B5- 2D
 * 25 -A5  B4- 2C
 * 26 -A6  B3- 2B
 * 27 -A8  B2- 2A
 *    -VCC B1- 29
 *    -GND B0- 28
 *
 * 2x=> prind 0
 * 3x=> prind 1
 * 4x=> prind 2
 * 5x=> prind 3
 * 6x=> prind 4
 * 7x=> prind 5
 * 8x=> prind 6
 * 9x=> prind 7
 * */

void MCP23S17_init(void)
{
    RESET_DDR_MCP23S17  |= (0x01 << RESET_PIN_MCP23S17);  /* output */
    RESET_PORT_MCP23S17 &= ~(0x01 << RESET_PIN_MCP23S17); /* laag */

    init_SPI();
    SPI_DDR_MCP23S17  |=  (0x01 << SPI_CS_MCP23S17);      /* output */
    MCP23S17_UNSELECT();

    MCP23S17_reset();
    /*
     * 00101100 = 0x0C
     * Bank   0
     * MIRROR 0
     * SEQOP  0 address pointer increments
     * DISSLW 0 Slew rate enabled
     * HAEN   1 Enables the MCP23S17 address pins.
     * ODR    1 Open-drain output (overrides the INTPOL bit)
     * INTPOL 0 |||||||
     * ------ 0 Unimplemented
*/
    MCP23S17_write_register(0,IOCONA,0x0C);
    MCP23S17_write_register(1,IOCONA,0x0C);
    MCP23S17_write_register(2,IOCONA,0x0C);
    MCP23S17_write_register(3,IOCONA,0x0C);
    MCP23S17_write_register(4,IOCONA,0x0C);
    MCP23S17_write_register(5,IOCONA,0x0C);
    MCP23S17_write_register(6,IOCONA,0x0C);
    MCP23S17_write_register(7,IOCONA,0x0C);
}

void MCP23S17_reset(void)
{
    RESET_PORT_MCP23S17 &= ~(0x01 << RESET_PIN_MCP23S17); /* laag */
    _delay_us(5);
    RESET_PORT_MCP23S17 |=  (0x01 << RESET_PIN_MCP23S17); /* up */
    _delay_us(2);

}

void MCP23S17_write_register(unsigned char slave_address,unsigned char address, unsigned char value)
{
#ifdef PCB_dev
    uint8_t slave_addres[8]={0x00,0x04,0x02,0x06,0x01,0x05,0x03,0x07};/* Fix PCB fout */
    uint8_t OpCode = (uint8_t)(slave_addres[slave_address] << 0x01);
#else
    slave_address &= 0x07;
    unsigned char OpCode = (unsigned char)(slave_address << 0x01);
#endif
    OpCode |= 0x40;/* write */
    MCP23S17_SELECT();
    spi_write(OpCode);
    spi_write(address);
    spi_write(value);
    MCP23S17_UNSELECT();

}

unsigned char MCP23S17_read_register(unsigned char slave_address, unsigned char address)
{
#ifdef PCB_dev
    uint8_t slave_addres[8]={0x00,0x04,0x02,0x06,0x01,0x05,0x03,0x07};/* Fix PCB fout */
    uint8_t OpCode = (uint8_t)(slave_addres[slave_address] << 0x01);
#else
    slave_address &= 0x07;
    unsigned char OpCode = (unsigned char)(slave_address << 0x01);
#endif
    OpCode |= 0x41; /* read */
    MCP23S17_SELECT();
    spi_write(OpCode);
    spi_write(address);
    unsigned char return_var= spi_read();
    MCP23S17_UNSELECT();

    return return_var;
}

void          MCP23S17_bit_modify(
        unsigned char slave_address,
        unsigned char address,
        unsigned char bitmask,
        unsigned char value)
{
    unsigned char var=MCP23S17_read_register(slave_address,address);
    var&=~bitmask;
    value&=bitmask;
    var|=value;
    MCP23S17_write_register(slave_address,address,var);
}
