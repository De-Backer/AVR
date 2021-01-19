cmake_minimum_required(VERSION 3.14...3.19)

## AVR Chip Configuration
# 20Mhz
set(F_CPU 20000000)

# CPU, you can find the list here:
# https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
set(MCU atmega1284p)
add_definitions(-D__AVR_ATmega1284P__)
add_definitions(-DF_CPU=${F_CPU})

# AVR Fuses, must be in concordance with your hardware and F_CPU
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega1284p
set(E_FUSE 0xFC)  # Brown-out 4.3V
set(H_FUSE 0x99)  # On-Chip_Debug=OFF JTAG=ON SPI=ON Watchdog=OFF Preserve_EEPROM=OFF Boot=Default
set(L_FUSE 0xFF)  # crystal osc. 16K CK + 65ms
set(LOCK_BIT 0xff)# No lock
