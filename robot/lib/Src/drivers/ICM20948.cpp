#include "drivers/ICM20948.hpp"
#include <cmath>

#define READ_BIT (1 << 7)

#define WHOAMI_VAL 0xEA

namespace Registers {
    // Bank select
    constexpr uint8_t REG_BANK_SEL  = 0x7F;

    // Bank 0
    constexpr uint8_t WHO_AM_I      = 0x00;
    constexpr uint8_t USER_CTRL     = 0x03;
    constexpr uint8_t LP_CONFIG     = 0x05; 
    constexpr uint8_t PWR_MGMT_1    = 0x06;
    constexpr uint8_t PWR_MGMT_2    = 0x07;

    constexpr uint8_t GYRO_Z_OUT_H  = 0x37;
    constexpr uint8_t GYRO_Z_OUT_L  = 0x38;

    constexpr uint8_t FIFO_MODE     = 0x69;
   
    // Bank 2 
    constexpr uint8_t GYRO_CONFIG_1 = 0x01;

    enum GYRO_FS_SEL {
        DPS_2000 = 3,
        DPS_1000 = 2,
        DPS_500 = 1,
        DPS_250 = 0,
    };

    enum PWR_MGMT {
        DEVICE_RESET = 1,
        SLEEP = 1,  // 1: enter sleep mode
        CLKSEL = 1, // 1-5: Auto selects best available clock source
    };

    enum LP_CONFIG_REG {
        I2C_MST_CYCLE = 1,
    };

    enum USER_CTRL_REG {
        I2C_IF_DIS = 1, // disable I2C, enable SPI
    }; 
    
};

ICM20948::ICM20948(std::shared_ptr<SPI> imuSPI, PinName cs_pin) 
    : imuSPI(std::move(imuSPI)),
      nCs(cs_pin) {
    //chip_select_pin.write(false);
    nCs = 1;
    HAL_Delay(1500);
}

void ICM20948::initialize() {
    // SPI pulse to clear pins. 
    imuSPI->transmit(0);    
    
    uint8_t whoami = read_register(0, Registers::WHO_AM_I);
    
    while (whoami != WHOAMI_VAL) {
        printf("Failed to connect to IMU.");
        whoami = read_register(Registers::WHO_AM_I);
    }
    
    // Reset
    write_register( 
        Registers::PWR_MGMT_1,
        Registers::PWR_MGMT::DEVICE_RESET << 7
        | Registers::PWR_MGMT::SLEEP << 6
        | Registers::PWR_MGMT::CLKSEL);

    HAL_Delay(100);

    // Check whoami again
    whoami = read_register(0, Registers::WHO_AM_I);
    
    while (whoami != WHOAMI_VAL) {
        printf("Failed to connect to IMU.");
        whoami = read_register(Registers::WHO_AM_I);
    }
    
    // Auto select best clock source
    write_register(
        Registers::PWR_MGMT_1, 
        Registers::PWR_MGMT::CLKSEL);
    
    // Operate I2C in duty cycled mode and disable GYRO duty cycle mode
    write_register(
        Registers::LP_CONFIG, 
        Registers::LP_CONFIG_REG::I2C_MST_CYCLE << 6);
    
    // Disable I2C
    write_register(
        Registers::USER_CTRL,
        Registers::USER_CTRL_REG::I2C_IF_DIS << 4);

    // Turn FIFO off
    write_register(Registers::FIFO_MODE, 0x00);

    // Enable gyro
    write_register(Registers::PWR_MGMT_2, 0x00);
    
    // Configure gyro
    write_register(
        2, 
        Registers::GYRO_CONFIG_1, 
        Registers::GYRO_FS_SEL::DPS_1000 << 2);
    
    
    printf("Initialization Complete!");
}

float ICM20948::gyro_z() {

    uint16_t hi = read_register(0, Registers::GYRO_Z_OUT_H),
             lo = read_register(0, Registers::GYRO_Z_OUT_L);


    // Reinterpret the bits as a signed 16-bit integer
    int16_t total = hi << 8 | lo;

    // +-1000dps max, for 16-bit signed integers, comes out to 32.8lsb/deg.
    const float lsbToDps = 1000.0f / 32768.f;
    const float degToRad = M_PI / 180.0f;
    return total * (degToRad * lsbToDps);
}


void ICM20948::chip_select(bool cs_state) {
    nCs.write(!cs_state); 
}


void ICM20948::write_register(uint8_t bank, uint8_t address, uint8_t value) {
    chip_select(true);
    imuSPI->transmit(Registers::REG_BANK_SEL);
    imuSPI->transmit(bank);
    chip_select(false);

    chip_select(true);
    imuSPI->transmit(address);
    imuSPI->transmit(value);
    chip_select(false);
}

void ICM20948::write_register(uint8_t address, uint8_t value) {
    chip_select(true);
    imuSPI->transmit(address);
    imuSPI->transmit(value);
    chip_select(false);
}


uint8_t ICM20948::read_register(uint8_t bank, uint8_t address) { 
    chip_select(true);
    imuSPI->transmit(Registers::REG_BANK_SEL);
    imuSPI->transmit(bank);
    chip_select(false);
    
    chip_select(true);
    imuSPI->transmit(READ_BIT | address);
    uint8_t data = imuSPI->transmitReceive(0x00);
    chip_select(false);

    return data;    
}


uint8_t ICM20948::read_register(uint8_t address) { 
    chip_select(true);
    imuSPI->transmit(READ_BIT | address);
    uint8_t data = imuSPI->transmitReceive(0x00);
    chip_select(false);

    return data;    
}
