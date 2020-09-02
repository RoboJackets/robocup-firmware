
message("------------------- avr toolchain ---------------------")

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_SYSTEM_VERSION    1)
# the docs aren't installed to man
# refer to http://www.nongnu.org/avr-libc/user-manual/using_tools.html
set(AVR_TARGET_ARCH attiny167)
set(AVR_PREFIX avr)

find_program(AVR_C_COMPILER     ${AVR_PREFIX}-gcc)
find_program(AVR_CXX_COMPILER   ${AVR_PREFIX}-g++)
find_program(AVR_OBJCOPY        ${AVR_PREFIX}-objcopy)

# set(CMAKE_C_COMPILER    ${AVR_C_COMPILER})
# set(CMAKE_CXX_COMPILER  ${AVR_CXX_COMPILER})
# set(CMAKE_OBJCOPY  ${AVR_OBJCOPY})

# F_CPU specifies the frequency (in Hz) of the cpu clock so that the delay functions work appropriately
# the ATTiny13 has an internal 9.6MHz clock with the prescaler, don't scale back
# change above to match ATTINY84A =====================================================================
# to keep timing for neopixel
# The -O2 turns on optimizations, which are required to use the delay_*() functions
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -O2 -mmcu=${AVR_TARGET_ARCH} -DF_CPU=8000000")
