cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set(CMAKE_C_STANDARD 11 CACHE STRING "")
set(CMAKE_C_STANDARD_REQUIRED ON CACHE BOOL "")

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_PROCESSOR  avr)
set(CMAKE_SYSTEM_VERSION    1)
# the docs aren't installed to man
# refer to http://www.nongnu.org/avr-libc/user-manual/using_tools.html
set(AVR_TARGET_ARCH         atmega32a)

set(AVR_PREFIX                  avr)
find_program(AVR_C_COMPILER     ${AVR_PREFIX}-gcc)
find_program(AVR_CXX_COMPILER   ${AVR_PREFIX}-g++)
find_program(AVR_OBJCOPY        ${AVR_PREFIX}-objcopy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -O3 -Wfatal-errors -O3 -mmcu=${AVR_TARGET_ARCH} -DF_CPU=8000000" CACHE STRING "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O3 -Wfatal-errors -O3 -mmcu=${AVR_TARGET_ARCH} -DF_CPU=8000000" CACHE STRING "")

set(CMAKE_C_COMPILER    ${AVR_C_COMPILER})
set(CMAKE_CXX_COMPILER  ${AVR_CXX_COMPILER})
