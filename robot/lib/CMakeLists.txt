
add_subdirectory(robocup-fshare)

add_library(firm-lib
  Src/drivers/AVR910.cpp
  Src/drivers/Battery.cpp
  Src/drivers/FPGA.cpp
  Src/drivers/I2Cdev.cpp
  Src/drivers/ISM43340.cpp
  Src/drivers/KickerBoard.cpp
  Src/drivers/MCP23017.cpp
  Src/drivers/MPU6050.cpp
)
target_link_libraries(firm-lib
    CONAN_PKG::mTrain
    CONAN_PKG::Eigen3
    rc-fshare
)

target_include_directories( firm-lib PUBLIC
    Inc
    robocup-fshare/include
)
