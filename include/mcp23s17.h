#ifndef MCP23S17_H
#define MCP23S17_H

#include "../include/SPI.h"

#include <avr/io.h>

#ifdef __cplusplus
extern "C"
{
#endif
#ifndef SPI_CS_MCP23S17
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
#    define SPI_CS_PIN = 9;
#endif

/** \brief Settings of the CS pin
 *
 * This function is used for setting of the CS pin. CS signal
 * is inverted, so input 1 (1) means zero on the output.
 * Otherwise is analogically the same.
 *
 * \warning This is platform-dependent method!
 * \param state Wished state
 */
#define MCP23S17_SELECT() SPI_PORT_MCP23S17 &= ~(0x01 << SPI_CS_MCP23S17); /* laag */

/** \brief Settings of the CS pin
 *
 * This function is used for setting of the CS pin. CS signal
 * is inverted, so input 1 (1) means zero on the output.
 * Otherwise is analogically the same.
 *
 * \warning This is platform-dependent method!
 * \param state Wished state
 */
#define MCP23S17_UNSELECT() SPI_PORT_MCP23S17 |= (0x01 << SPI_CS_MCP23S17); /* high */

enum BANK0 {IODIRA, IODIRB,
            IPOLA, IPOLB,
            GPINTENA, GPINTENB,
            DEFVALA, DEFVALB,
            INTCONA, INTCONB,
            IOCONA, IOCONB,
            GPPUA, GPPUB,
            INTFA, INTFB,
            INTCAPA, INTCAPB,
            GPIOA, GPIOB,
            OLATA, OLATB
           };

void MCP23S17_init(void);
void MCP23S17_reset(void);
void MCP23S17_write_register(unsigned char slave_address, unsigned char address, unsigned char value);
unsigned char MCP23S17_read_register(unsigned char slave_address, unsigned char address);
void          MCP23S17_bit_modify(
        unsigned char slave_address,
        unsigned char address,
        unsigned char bitmask,
        unsigned char value);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // MCP23S17_H
