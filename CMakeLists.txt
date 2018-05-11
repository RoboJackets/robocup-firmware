cmake_minimum_required(VERSION 3.1)
project(Unnamed LANGUAGES C)

if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "ARM")
    message(FATAL_ERROR "Cannot be compiled for architectures other than ARM \
    (Current=${CMAKE_SYSTEM_PROCESSOR})")
endif()

# TODO: Confirm these flags and make them not flags if possible
set (CMAKE_C_FLAGS "-g -mthumb -fno-builtin -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 \
    -mfloat-abi=softfp -Wall -std=gnu99 -ffunction-sections -fdata-sections \
    -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math \
    -ftree-vectorize --specs=nosys.specs" CACHE INTERNAL "c compiler flags")
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set (CMAKE_ASM_FLAGS "-g -mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 \
    -mfloat-abi=softfp -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")
set (CMAKE_EXE_LINKER_FLAGS "-g -Wl,--gc-sections -mthumb -mcpu=cortex-m7 \
    -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -mabi=aapcs"
    CACHE INTERNAL "executable linker flags")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
-T ${CMAKE_SOURCE_DIR}/library/Drivers/STM32F769NIHx_FLASH.ld")

# TODO: What does the "USE_HAL_DRIVER" flag do?
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D USE_HAL_DRIVER -D STM32F769xx")

add_subdirectory(library/Drivers)
add_subdirectory(library/Middleware/FreeRTOS)

add_subdirectory(library/Drivers/BSP/STM32F769I_EVAL)
add_subdirectory(test)
