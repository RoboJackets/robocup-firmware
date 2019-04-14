set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_PROCESSOR  avr)
set(CMAKE_SYSTEM_VERSION    1)
set(AVR_TARGET_ARCH         attiny167)

# narrow down the search scope of where cmake looks for programs/libraries
# for cross compilation
# set(CMAKE_FIND_ROOT_PATH        ${PROJECT_SOURCE_DIR})
# # search for programs in the build host directories
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# # for libraries and headers in the target directories
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(AVR_PREFIX                  avr)
find_program(AVR_C_COMPILER     ${AVR_PREFIX}-gcc)
find_program(AVR_CXX_COMPILER   ${AVR_PREFIX}-g++)
find_program(AVR_OBJCOPY        ${AVR_PREFIX}-objcopy)

set(CMAKE_C_COMPILER    ${AVR_C_COMPILER})
set(CMAKE_CXX_COMPILER  ${AVR_CXX_COMPILER})

# F_CPU specifies the frequency (in Hz) of the cpu clock so that the delay functions work appropriately
# the ATTiny13 has an internal 9.6MHz clock with the prescaler, don't scale back
# change above to match ATTINY84A =====================================================================
# to keep timing for neopixel
# The -O2 turns on optimizations, which are required to use the delay_*() functions
set(CMAKE_C_FLAGS "-std=c11 -O2 -mmcu=attiny167 -DF_CPU=8000000")