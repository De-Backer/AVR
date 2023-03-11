include(cmake/hoto_update_cmake_message.cmake)

cmake_minimum_required(VERSION 3.14...3.19 FATAL_ERROR)

project(AVR
  LANGUAGES C
  VERSION 0.0.0
  DESCRIPTION ""
  HOMEPAGE_URL ""
)

include(cmake/copyright_message.cmake)
include(cmake/git_update_submodules.cmake)
include(cmake/git_last_commit_sha.cmake)
include(cmake/atmega1284p.cmake)
include(cmake/avr_pre.cmake)

add_library(USART src/usart.c include/usart.h)

#CAN_MCP2515
add_definitions(-DSPI_DDR_CAN=DDRB)
add_definitions(-DSPI_PORT_CAN=PORTB)
add_definitions(-DSPI_CS_CAN=1)
#CAN interupt pin
add_definitions(-DINT_DDR_CAN=DDRB)
add_definitions(-DINT_PORT_CAN=PORTB)
add_definitions(-DINT_PIN_CAN=PINB)
add_definitions(-DINT_CAN=2)
#CAN id
add_definitions(-DID_CAN=0x10)
add_library(CAN_MCP2515 src/CAN_MCP2515.c include/CAN_MCP2515.h)

#MCP23S17
add_definitions(-DSPI_DDR_MCP23S17=DDRB)
add_definitions(-DSPI_PORT_MCP23S17=PORTB)
add_definitions(-DSPI_CS_MCP23S17=0)
#reset pin
add_definitions(-DRESET_DDR_MCP23S17=DDRB)
add_definitions(-DRESET_PORT_MCP23S17=PORTB)
add_definitions(-DRESET_PIN_MCP23S17=3)
#Fix PCB bug
add_definitions(-DPCB_dev)
add_library(MCP23S17 src/mcp23s17.c include/mcp23s17.h)

add_library(SPI src/SPI.c include/SPI.h)

add_library(BUFFER src/RingBuffer.c include/RingBuffer.h)

add_library(EBUSD src/ebusd.c include/ebusd.h)

# Create one target
add_executable(AVR src/main.c)

target_link_libraries(AVR PRIVATE USART BUFFER CAN_MCP2515 MCP23S17 SPI EBUSD)

include(cmake/avr_post.cmake)

message("npx mcp-can-boot-flash-app -f ${PROJECT_NAME}_${GIT_COMMIT_SHA}.hex -p atmega1284p -m 0xffff -R 7ff#0404")
