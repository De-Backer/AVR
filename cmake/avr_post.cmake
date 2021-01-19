cmake_minimum_required(VERSION 3.14...3.19)

# Rename the output to .elf as we will create multiple files
set_target_properties(${PROJECT_NAME}
PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.elf)

# Strip binary for upload
add_custom_target(strip ALL avr-strip ${PROJECT_NAME}.elf
DEPENDS ${PROJECT_NAME})

# Transform binary into hex file, we ignore the eeprom segments in the step
add_custom_target(hex ALL
avr-objcopy
 -R .eeprom
 -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
 DEPENDS strip)

# Transform binary into hex file, this is the eeprom part
# (empty if you don't use eeprom static variables)
add_custom_target(eeprom
avr-objcopy
 -j .eeprom
 --set-section-flags=.eeprom="alloc,load"
 --change-section-lma .eeprom=0
 -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.eep
 DEPENDS strip)

# Upload the firmware with avrdude
add_custom_target(upload
avrdude
 -c ${PROG_TYPE}
 -P ${PROG_PORT}
 -p ${MCU}
 -v
 -U flash:w:${PROJECT_NAME}.hex
 DEPENDS hex)

# Upload the eeprom with avrdude
add_custom_target(upload_eeprom
avrdude
 -c ${PROG_TYPE}
 -P ${PROG_PORT}
 -p ${MCU}
 -v
 -U eeprom:w:${PROJECT_NAME}.eep
 DEPENDS eeprom)

# Burn fuses
add_custom_target(fuses
avrdude
 -c ${PROG_TYPE}
 -P ${PROG_PORT}
 -p ${MCU}
 -v
 -U lfuse:w:${L_FUSE}:m
 -U hfuse:w:${H_FUSE}:m
 -U efuse:w:${E_FUSE}:m
 -U lock:w:${LOCK_BIT}:m )

# Clean extra files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
"${PROJECT_NAME}.hex;${PROJECT_NAME}.eeprom;${PROJECT_NAME}.lst")
