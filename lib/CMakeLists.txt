# Several things depend on the headers in the 'common' directory
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common") # for headers in common/
include_directories("${CMAKE_CURRENT_BINARY_DIR}/common") # for generated protobuf headers

add_subdirectory(common)

# set the arm toolchain and include it so we can use it
set(CMAKE_TOOLCHAIN_FILE ${ARM_TOOLCHAIN_FILE})
include(${ARM_TOOLCHAIN_FILE})

# Set compiler and linker flags
set(CMAKE_CXX_FLAGS         ${MBED_CMAKE_CXX_FLAGS}         )
set(CMAKE_C_FLAGS           ${MBED_CMAKE_C_FLAGS}           )
set(CMAKE_EXE_LINKER_FLAGS  ${MBED_CMAKE_EXE_LINKER_FLAGS}  )

file(GLOB_RECURSE firm-lib_SRC
    "drivers/*.cpp"
    "modules/*.cpp"
    "utils/*.cpp"
)

# Find the assembly source files and make sure they're compiled using the C compiler
file(GLOB_RECURSE asm_SRC "*.S")
set_property(SOURCE ${asm_SRC} PROPERTY LANGUAGE C)
list(APPEND firm-lib_SRC ${asm_SRC})

# append all of the accessory library sources
list(APPEND firm-lib_SRC ${MBED_ASSEC_LIBS_SRCS})
# include PID from common
list(APPEND firm-lib_SRC ${CMAKE_CURRENT_SOURCE_DIR}/common/Pid.cpp)

# tell cmake that all of the source files aren't here yet
# set_source_files_properties(
#     ${MBED_ASSEC_LIBS_SRCS} PROPERTIES
#     GENERATED               TRUE
#     DEPENDS                 ${MBED_ASSEC_LIBS_DEPENDS}
# )

# build the 'common2015' library that contains all the MBED stuff needed for
# both the base station and robot firmware
add_library(firm-lib ${firm-lib_SRC} "${CMAKE_CURRENT_BINARY_DIR}/common/git_version.cpp")

target_include_directories( firm-lib PUBLIC
    utils/include
    modules/include
    drivers/include
    drivers/avr-isp
    drivers/ball-sense
    drivers/buzzer
    drivers/cc1201
    drivers/decawave
    drivers/ds2411
    drivers/fpga
    drivers/kicker-board
    drivers/mcp23017
    drivers/mpu-6050
    drivers/rotarySelector
    drivers/rtos-i2c
    drivers/shared-spi
    drivers/software-spi
    drivers/ws2811
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MBED_ASSEC_LIBS_INCLUDES}
)

# CC1201 Register settings
# Automatically generate a C header with the register values from a chosen smartrf xml file
if(DEFINED ENV{CC1201_CONFIG_NAME})
    set(CC1201_CONFIG_NAME $ENV{CC1201_CONFIG_NAME})
else()
    set(CC1201_CONFIG_NAME cc1101-compatible)
endif()
set(CC1201_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/drivers/cc1201/cfg/${CC1201_CONFIG_NAME}.xml)
add_custom_target(cc1201_register_export
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/cc1201/cfg/smartrf_exporter.py
        ${CC1201_CONFIG}
        ${CMAKE_CURRENT_BINARY_DIR}/cc1201_register_export.h
    DEPENDS ${CC1201_CONFIG}
    COMMENT "Generating C header file for cc1201 registers from '${CC1201_CONFIG_NAME}' smartrf xml file"
)
set_target_properties(cc1201_register_export PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_dependencies(firm-lib cc1201_register_export)
target_include_directories(firm-lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
# depends on the mbed libraries and all of the accessory libraries
add_dependencies(firm-lib mbed_libraries ${MBED_ASSEC_LIBS_DEPENDS})
target_link_libraries(firm-lib ${MBED_LIBRARY})
set_target_properties(firm-lib PROPERTIES EXCLUDE_FROM_ALL TRUE)

# See common/CMakeLists for more info on how this works
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/common/git_version.cpp
    DEPENDS git_version
)
