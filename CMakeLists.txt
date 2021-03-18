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

add_library(SPI src/SPI.c include/SPI.h)

add_library(BUFFER src/RingBuffer.c include/RingBuffer.h)

# Create one target
add_executable(AVR src/main.c)

target_link_libraries(AVR PRIVATE USART BUFFER CAN_MCP2515 SPI)

include(cmake/avr_post.cmake)

