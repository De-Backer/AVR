cmake_minimum_required(VERSION 3.14...3.19)

## AVR Chip Configuration

# CPU, you can find the list here:
# https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
set(MCU atmega1284p)
add_definitions(-D__AVR_ATmega1284P__)
# 20Mhz
set(F_CPU 16000000)
add_definitions(-DF_CPU=${F_CPU})
#SPI
add_definitions(-DSPI_PORT=PORTB)
add_definitions(-DSPI_PIN=PINB)
add_definitions(-DSPI_DDR=DDRB)
add_definitions(-DSPI_CS=4)
add_definitions(-DSPI_SCK=7)
add_definitions(-DSPI_MOSI=5)
add_definitions(-DSPI_MISO=6)

# AVR Fuses, must be in concordance with your hardware and F_CPU
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega1284p
set(E_FUSE 0xFC)  # Brown-out 4.3V
set(H_FUSE 0xD2)  # On-Chip_Debug=OFF SPI=ON Watchdog=OFF Preserve_EEPROM=ON Boot=01
set(L_FUSE 0xD6)  # F S crystal osc. 258K CK + 65ms
set(LOCK_BIT 0xff)# No lock
