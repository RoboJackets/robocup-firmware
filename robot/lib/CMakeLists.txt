
add_subdirectory(robocup-fshare)

add_library(firm-lib
    Src/drivers/FPGA.cpp
)
target_link_libraries(firm-lib
    CONAN_PKG::mTrain
    rc-fshare
)

target_include_directories( firm-lib PUBLIC
    Inc
)
