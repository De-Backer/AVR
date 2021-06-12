cmake_minimum_required(VERSION 3.14...3.19 FATAL_ERROR)

# The programmer to use, read avrdude manual for list
set(PROG_TYPE dragon_isp)

# Specify connection port.
set(PROG_PORT usb 03eb:2107)

# Use AVR GCC toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

# Pass defines to compiler
add_definitions(
    -DF_CPU=${F_CPU}
    -DBAUD=${BAUD}
)

# mmcu MUST be passed to bot the compiler and linker, this handle the linker
set(CMAKE_EXE_LINKER_FLAGS -mmcu=${MCU})

add_compile_options(
    -mmcu=${MCU} # MCU
    -std=gnu99 # C99 standard
    -Os # optimize
    -Wall # enable warnings
    -save-temps # Do not delete intermediate files.
#    -Wno-main
#    -Wundef
#    -pedantic
#    -Wpedantic # fix warning: binary constants are a GCC extension 0b00000000
#    -Wstrict-prototypes
#    -Werror
#    -Wfatal-errors
#    -Wl,--relax,--gc-sections
#    -g
#    -gdwarf-2
#    -funsigned-char # a few optimizations
#    -funsigned-bitfields
#    -fpack-struct
#    -fshort-enums
#    -ffunction-sections
#    -fdata-sections
#    -fno-split-wide-types
#    -fno-tree-scev-cprop
)


# directorie to AVR lib
include_directories(/usr/lib/avr/include/)
include_directories(/usr/lib/)
