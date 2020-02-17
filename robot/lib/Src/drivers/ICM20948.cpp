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
    //constexpr uint8_t 
   
    // Bank 2 
    constexpr uint8_t GYRO_CONFIG_1 = 0x01;
    //constexpr uint8_t 


    enum GyroFS {
        DPS_2000 = 3,
        DPS_1000 = 2,
        DPS_500 = 1,
        DPS_250 = 0,
    };

};

ICM20948::ICM20948(std::unique_ptr<SPI> imuSPI, PinName cs_pin) 
    : imuSPI(std::move(imuSPI)),
      nCs(cs_pin) {
    //chip_select_pin.write(false);
    nCs = 1;
    HAL_Delay(1500);
}

bool ICM20948::initialize() {
    // chip_select(true);

    // SPI pulise to clear pins. 
    imuSPI->transmit(0);    
    
    HAL_Delay(100);

    uint8_t whoami = read_register(0, Registers::WHO_AM_I);
    bool failed_init = false;

    if (whoami != WHOAMI_VAL) {
        printf("Failed to connect to IMU.");
        failed_init = true;
    }

    // Reset
    write_register(0, Registers::PWR_MGMT_1, 0xC1);

    HAL_Delay(100);

    // Check whoami again
    whoami = read_register(0, Registers::WHO_AM_I);

    if (whoami != WHOAMI_VAL && failed_init) {
        printf("Failed to connect to IMU.");
        return false;
    }
    
    // Auto select best clock source
    write_register(0, Registers::PWR_MGMT_1, 0x01);
    
    // Operate I2C in duty cycled mode and disable GYRO duty cycle mode
    write_register(0, Registers::LP_CONFIG, 0x40);
    
    // Disable I2C
    write_register(0, Registers::USER_CTRL, 0x10);

    // Turn FIFO off
    write_register(0, Registers::FIFO_MODE, 0x00);

    // Enable gyro
    write_register(0, Registers::PWR_MGMT_2, 0x00);
    
    // Configure gyro
    write_register(2, Registers::GYRO_CONFIG_1, 0x04);
    
    // Check whoami again
    whoami = read_register(0, Registers::WHO_AM_I);

    if (whoami != WHOAMI_VAL) {
        printf("Failed to connect to IMU.");
        return false;
    } else {
        printf("Initialization Complete!");
        return true;
    }
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

/*
uint8_t ICM20948::read_register(uint8_t address) {
    bool was_cs_active = currently_active;

    chip_select(true);

    uint8_t data_tx[2] = {address | READ_BIT, 0x0};
    uint8_t data_rx[2] = {0x0, 0x0};
    imuSPI->transmitReceive(data_tx, data_rx, 2);

    printf("%x%x -> %x%x\r\n", data_tx[0], data_tx[1], data_rx[0], data_rx[1]);

    // Restore previous CS state
    chip_select(was_cs_active);

    return data_rx[1];
}

void ICM20948::write_register(uint8_t address, uint8_t value) {
    bool was_cs_active = currently_active;

    chip_select(true);

    uint8_t data_tx[2] = {address, value};
    uint8_t data_rx[2] = {0x0, 0x0};
    imuSPI->transmitReceive(data_tx, data_rx, 2);

    // Restore previous CS state
    chip_select(was_cs_active);
}

void ICM20948::chip_select(bool active) {
    if (currently_active != active) {
        currently_active = active;
        chip_select_pin.write(active);
    }
}
*/

void ICM20948::chip_select(bool cs_state) {
    if (cs_state) {
        nCs.write(0);
    } else {
        nCs.write(1);
    }
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
