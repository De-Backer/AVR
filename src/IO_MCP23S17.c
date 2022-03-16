#include "../include/IO_MCP23S17.h"


void MCP23S17_init()
{
    init_SPI();
    SPI_DDR_IO |= (0x01 << SPI_CS_IO);   /* output */
    SPI_PORT_IO &= ~(0x01 << SPI_CS_IO); /* laag */

}
