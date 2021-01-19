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

add_library(ADC src/adc.c include/adc.h)
add_library(USART src/usart.c include/usart.h)

add_library(BUFFER src/RingBuffer.c include/RingBuffer.h)

# Create one target
add_executable(AVR src/main.c)

target_link_libraries(AVR PRIVATE ADC USART BUFFER)

include(cmake/avr_post.cmake)

