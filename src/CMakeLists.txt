
add_subdirectory(base)

# include all sub directories
add_subdirectory(control)
add_subdirectory(kicker)
add_subdirectory(hw-test)

# only include the FPGA things if we're running something
# unix related, but not something apple related
if( UNIX AND NOT APPLE )
    add_subdirectory(fpga)
else()
    message(WARNING "Skipping Xilinx configurations")
endif()