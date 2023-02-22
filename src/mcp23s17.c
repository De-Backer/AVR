#include "../include/mcp23s17.h"
#include <util/delay.h>

void MCP23S17_init(void)
{
    RESET_DDR_MCP23S17  |= (0x01 << RESET_PIN_MCP23S17);  /* output */
    RESET_PORT_MCP23S17 &= ~(0x01 << RESET_PIN_MCP23S17); /* laag */

    init_SPI();
    SPI_DDR_MCP23S17  |=  (0x01 << SPI_CS_MCP23S17);      /* output */
    MCP23S17_UNSELECT();

    MCP23S17_reset();
    /*
     * 00101100 = 0x2C
     * Bank   0
     * MIRROR 0
     * SEQOP  1 address pointer does not increment
     * DISSLW 1 Slew rate disabled
     * HAEN   1 Enables the MCP23S17 address pins.
     * ODR    1 Open-drain output (overrides the INTPOL bit)
     * INTPOL 0 |||||||
     * ------ 0 Unimplemented
*/
    MCP23S17_write_register(0,IOCONA,0x2C);
    MCP23S17_write_register(1,IOCONA,0x2C);
    MCP23S17_write_register(2,IOCONA,0x2C);
    MCP23S17_write_register(3,IOCONA,0x2C);
    MCP23S17_write_register(4,IOCONA,0x2C);
    MCP23S17_write_register(5,IOCONA,0x2C);
    MCP23S17_write_register(6,IOCONA,0x2C);
    MCP23S17_write_register(7,IOCONA,0x2C);
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
    slave_address &= 0x07;
    unsigned char OpCode = (unsigned char)(slave_address << 0x01);
    OpCode |= 0x40;/* write */
    MCP23S17_SELECT();
    spi_write(OpCode);
    spi_write(address);
    spi_write(value);
    MCP23S17_UNSELECT();

}

unsigned char MCP23S17_read_register(unsigned char slave_address, unsigned char address)
{
    slave_address &= 0x07;
    unsigned char OpCode = (unsigned char)(slave_address << 0x01);
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
