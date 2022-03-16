#ifndef IO_MCP23S17_H
#define IO_MCP23S17_H

#include "../include/SPI.h"

#include <avr/io.h>

#ifdef __cplusplus
extern "C"
{
#endif
    void MCP23S17_init(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // IO_MCP23S17_H
