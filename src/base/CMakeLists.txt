# set the ARM toolchain
set(CMAKE_TOOLCHAIN_FILE ${ARM_TOOLCHAIN_FILE})
include(${ARM_TOOLCHAIN_FILE})

# Set compiler and linker flags
set(CMAKE_CXX_FLAGS ${MBED_CMAKE_CXX_FLAGS})
set(CMAKE_C_FLAGS ${MBED_CMAKE_C_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS ${MBED_CMAKE_EXE_LINKER_FLAGS})

# firmware source files
set(base_SRC
    main.cpp
    RJBaseUSBDevice.cpp
)

add_executable(base_elf ${base_SRC})
target_link_libraries(base_elf firm-lib)

# the final product is the .bin file, not the elf one.  We hide this away in the build dir
set_target_properties(base_elf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# only build base station firmware if specifically instructed
set_target_properties(base_elf PROPERTIES EXCLUDE_FROM_ALL TRUE)

# custom target for creating a .bin file from an elf binary
add_custom_target(base
    ${ARM_OBJCOPY} -O binary base_elf rj-base.bin
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/rj-base.bin ${PROJECT_SOURCE_DIR}/run/
    DEPENDS base_elf
    COMMENT "objcopying to make MBED-compatible executable"
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_SOURCE_DIR}/run/rj-base.bin)

add_custom_target(base-prog
    COMMAND ${MBED_COPY_SCRIPT} ${PROJECT_SOURCE_DIR}/run/rj-base.bin
    DEPENDS base
    COMMENT "Copying base station's binary over to the mbed"
)
