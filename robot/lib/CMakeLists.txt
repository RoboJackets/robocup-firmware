
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
  Src/drivers/ICM20948.cpp
  Src/drivers/ICM42605.cpp
  Src/drivers/TMC4671.cpp
  Src/drivers/TMC6200.cpp
  Src/drivers/TMC4671.cpp
  Src/drivers/ADC124S101.cpp
)
target_link_libraries(firm-lib
    CONAN_PKG::mTrain
    CONAN_PKG::eigen
    rc-fshare
)

target_include_directories( firm-lib PUBLIC
    Inc
    robocup-fshare/include
)
