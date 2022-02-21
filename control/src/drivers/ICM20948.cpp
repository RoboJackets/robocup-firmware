#include "drivers/ICM20948.hpp"

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
    constexpr uint8_t ACCEL_X_OUT_H  = 0x2D;
    constexpr uint8_t ACCEL_X_OUT_L  = 0x2E;
    constexpr uint8_t ACCEL_Y_OUT_H  = 0x2F;
    constexpr uint8_t ACCEL_Y_OUT_L  = 0x30;

    constexpr uint8_t FIFO_MODE     = 0x69;

    // Bank 2
    constexpr uint8_t GYRO_CONFIG_1 = 0x01;
    constexpr uint8_t ACCEL_CONFIG  = 0x14;

    enum GYRO_FS_SEL {
        DPS_2000 = 3 << 2,
        DPS_1000 = 2 << 2,
        DPS_500 = 1 << 2,
        DPS_250 = 0 << 2,
    };

    enum PWR_MGMT {
        DEVICE_RESET = 1 << 7,
        SLEEP = 1 << 6,  // 1: enter sleep mode
        CLKSEL = 0b001, // 1-5: Auto selects best available clock source
    };

    enum USER_CTRL_REG {
        I2C_IF_DIS = 1 << 4, // disable I2C, enable SPI
    };
};

ICM20948::ICM20948(LockedStruct<SPI>& imuSPI, PinName cs_pin)
    : imuSPI(imuSPI),
      nCs(cs_pin) {
    nCs = 1;
}

bool ICM20948::initialize() {
    // SPI pulse to clear pins.
    {
        auto lock = imuSPI.lock();
        lock->transmit(0);
    }

    vTaskDelay(100);

    uint8_t whoami = read_register(0, Registers::WHO_AM_I);

    if (whoami != WHOAMI_VAL) {
        printf("[ERROR] Failed to connect to IMU.");
        return false;
    }

    // Reset
    write_register(
        0,
        Registers::PWR_MGMT_1,
        Registers::PWR_MGMT::DEVICE_RESET |
        Registers::PWR_MGMT::SLEEP |
        Registers::PWR_MGMT::CLKSEL);

    vTaskDelay(100);

    // Check whoami again
    whoami = read_register(0, Registers::WHO_AM_I);

    while (whoami != WHOAMI_VAL) {
        printf("[INFO] Waiting for IMU to boot.");
        whoami = read_register(0, Registers::WHO_AM_I);
        vTaskDelay(100);
    }

    // Wakes the IMU and auto selects best clock source
    write_register(
        0,
        Registers::PWR_MGMT_1,
        Registers::PWR_MGMT::CLKSEL);

    // Disable I2C
    write_register(
        0,
        Registers::USER_CTRL,
        Registers::USER_CTRL_REG::I2C_IF_DIS);

    // Enable gyro
    write_register(0, Registers::PWR_MGMT_2, 0x00);

    // Configure gyro
    write_register(
        2,
        Registers::GYRO_CONFIG_1,
        Registers::GYRO_FS_SEL::DPS_1000);


    // Configure acclerometer
    write_register(2, Registers::ACCEL_CONFIG, 0x02);

    if (whoami != WHOAMI_VAL) {
        printf("[ERROR] Failed to connect to IMU.");
        return false;
    } else {
        printf("[INFO] Initialization Complete!");
        return true;
    }
}

double ICM20948::gyro_z() {
    uint16_t hi = read_register(0, Registers::GYRO_Z_OUT_H),
             lo = read_register(0, Registers::GYRO_Z_OUT_L);

    // Reinterpret the bits as a signed 16-bit integer
    int16_t total = hi << 8 | lo;

    // +-1000dps max, for 16-bit signed integers, comes out to 32.8lsb/deg.
    const double lsbToDps = 1000.0f / 32768.f;
    const double degToRad = M_PI / 180.0f;
    return total * (degToRad * lsbToDps);
}

double ICM20948::accel_x() {
    uint16_t hi = read_register(0, Registers::ACCEL_X_OUT_H),
             lo = read_register(0, Registers::ACCEL_X_OUT_L);


    // Reinterpret the bits as a signed 16-bit integer
    int16_t total = hi << 8 | lo;

    // +-4g max, for 16-bit signed integers, comes out to 8192lsb/g.
    const double lsbToG = 4.0f / 32768.f;
    return total * lsbToG;
}

double ICM20948::accel_y() {
    uint16_t hi = read_register(0, Registers::ACCEL_Y_OUT_H),
             lo = read_register(0, Registers::ACCEL_Y_OUT_L);

    // Reinterpret the bits as a signed 16-bit integer
    int16_t total = hi << 8 | lo;

    // +-4g max, for 16-bit signed integers, comes out to 8192lsb/g.
    const double lsbToG = 4.0f / 32768.f;
    return total * lsbToG;
}

void ICM20948::chip_select(bool cs_state) {
    nCs.write(!cs_state);
}

void ICM20948::write_register(uint8_t bank, uint8_t address, uint8_t value) {
    auto lock = imuSPI.lock();

    if (bank != last_bank) {
        chip_select(true);
        lock->transmit(Registers::REG_BANK_SEL);
        lock->transmit(bank);
        chip_select(false);

        last_bank = bank;
    }

    chip_select(true);
    lock->transmit(address);
    lock->transmit(value);
    chip_select(false);
}

uint8_t ICM20948::read_register(uint8_t bank, uint8_t address) {
    auto lock = imuSPI.lock();

    if (bank != last_bank) {
        chip_select(true);
        lock->transmit(Registers::REG_BANK_SEL);
        lock->transmit(bank);
        chip_select(false);

        last_bank = bank;
    }

    chip_select(true);
    lock->transmit(READ_BIT | address);
    uint8_t data = lock->transmitReceive(0x00);
    chip_select(false);

    return data;
}

void ICM20948::burst_read(uint8_t bank, uint8_t address, uint8_t* buffer_out, size_t length) {
    auto lock = imuSPI.lock();

    if (bank != last_bank) {
        chip_select(true);
        lock->transmit(Registers::REG_BANK_SEL);
        lock->transmit(bank);
        chip_select(false);

        last_bank = bank;
    }

    chip_select(true);
    lock->transmit(address);
    for (int i = 0; i < length; i++) {
        buffer_out[i] = lock->transmitReceive(0x00);
    }
    chip_select(false);
}
