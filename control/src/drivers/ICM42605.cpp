#include "drivers/ICM42605.hpp"

#define READ_BIT (1 << 7)

#define WHOAMI_VAL 0x42

namespace Registers {
    // Bank select
    constexpr uint8_t REG_BANK_SEL = 0x76;

    // Bank 0
    constexpr uint8_t WHO_AM_I = 0x75;
    constexpr uint8_t PWR_MGMT0 = 0x4E;

    enum PwrMgmt {
        TEMP_DIS = 0b1 << 5,
        IDLE = 0b1 << 4,
        GYRO_MODE_OFF = 0b00 << 2,
        GYRO_MODE_STANDBY = 0b01 << 2,
        GYRO_MODE_LOW_NOISE = 0b11 << 2,
        ACCEL_MODE_OFF = 0b00,
        ACCEL_MODE_LOW_POWER = 0b10,
        ACCEL_MODE_LOW_NOISE = 0b11,
    };

    constexpr uint8_t DEVICE_CONFIG = 0x11;

    enum DeviceConfig {
        SPI_MODE_0 = 0b0 << 4,
        SPI_MODE_1 = 0b1 << 4,
        SPI_MODE_2 = 0b1 << 4,
        SPI_MODE_3 = 0b0 << 4,
        RESET = 0b1,
    };

    constexpr uint8_t ACCEL_DATA_X1 = 0x1F;
    constexpr uint8_t ACCEL_DATA_X0 = 0x20;
    constexpr uint8_t ACCEL_DATA_Y1 = 0x21;
    constexpr uint8_t ACCEL_DATA_Y0 = 0x22;
    constexpr uint8_t GYRO_DATA_Z1 = 0x29;
    constexpr uint8_t GYRO_DATA_Z0 = 0x2A;

    // Bank 2
    constexpr uint8_t GYRO_CONFIG0 = 0x4F;
    constexpr uint8_t ACCEL_CONFIG0 = 0x50;

    enum GyroConfig {
        GYRO_FS_2000 = 0b000 << 5,
        GYRO_FS_1000 = 0b001 << 5,
        GYRO_FS_500 = 0b010 << 5,
        GYRO_FS_250 = 0b011 << 5,
        GYRO_FS_125 = 0b110 << 5,
        GYRO_FS_62_5 = 0b101 << 5,
        GYRO_FS_31_25 = 0b110 << 5,
        GYRO_FS_15_625 = 0b111 << 5,

        GYRO_ODR_8kHz = 0b0011,
        GYRO_ODR_4kHz = 0b0100,
        GYRO_ODR_2kHz = 0b0101,
        GYRO_ODR_1kHz = 0b0110,
        GYRO_ODR_200Hz = 0b0111,
        GYRO_ODR_100Hz = 0b1000,
        GYRO_ODR_50Hz = 0b1001,
        GYRO_ODR_25Hz = 0b1010,
        GYRO_ODR_12_5Hz = 0b1011,
        GYRO_ODR_500Hz = 0b1111,
    };

    enum AccelConfig {
        ACCEL_FS_16 = 0b000 << 5,
        ACCEL_FS_8 = 0b001 << 5,
        ACCEL_FS_4 = 0b010 << 5,
        ACCEL_FS_2 = 0b011 << 5,

        ACCEL_ODR_8kHz = 0b0011,
        ACCEL_ODR_4kHz = 0b0100,
        ACCEL_ODR_2kHz = 0b0101,
        ACCEL_ODR_1kHz = 0b0110,
        ACCEL_ODR_200Hz = 0b0111,
        ACCEL_ODR_100Hz = 0b1000,
        ACCEL_ODR_50Hz = 0b1001,
        ACCEL_ODR_25Hz = 0b1010,
        ACCEL_ODR_12_5Hz = 0b1011,
        ACCEL_ODR_6_25Hz = 0b1100,
        ACCEL_ODR_3_125Hz = 0b1101,
        ACCEL_ODR_1_5625Hz = 0b1110,
        ACCEL_ODR_500Hz = 0b1111,
    };
} // namespace Registers


ICM42605::ICM42605(LockedStruct<SPI>& imuSPI, PinName cs_pin)
        : imuSPI(imuSPI),
          nCs(cs_pin) {
    nCs = 1;
}

bool ICM42605::initialize() {
    // SPI pulse to clear pins.
    {
        auto lock = imuSPI.lock();
        lock->transmit(0);
    }

    vTaskDelay(100);

    uint8_t whoami = read_register(0, Registers::WHO_AM_I);

    while (whoami != WHOAMI_VAL) {
        printf("[ERROR] Failed to connect to IMU.");
        vTaskDelay(100);
        whoami = read_register(0, Registers::WHO_AM_I);
    }

    // Reset
    write_register(
            0,
            Registers::DEVICE_CONFIG,
            Registers::DeviceConfig::SPI_MODE_0 | Registers::DeviceConfig::RESET);

    vTaskDelay(100);

    // Check whoami again
    whoami = read_register(0, Registers::WHO_AM_I);

    while (whoami != WHOAMI_VAL) {
        printf("[INFO] Waiting for IMU to boot.");
        whoami = read_register(0, Registers::WHO_AM_I);
        vTaskDelay(100);
    }

    vTaskDelay(100);

    // Turn on the gyro and accel
    write_register(
            0,
            Registers::PWR_MGMT0,
            Registers::PwrMgmt::GYRO_MODE_LOW_NOISE | Registers::PwrMgmt::ACCEL_MODE_LOW_NOISE);

    // From 14.36:
    // Gyroscope needs to be kept ON for a minimum of 45ms. When transitioning from OFF to any of the other modes, do not issue any register writes for 200μs.
    vTaskDelay(100);

    // Configure gyro and accelerometer
    write_register(
            0,
            Registers::GYRO_CONFIG0,
            Registers::GyroConfig::GYRO_FS_1000 | Registers::GyroConfig::GYRO_ODR_1kHz);

    write_register(
            0,
            Registers::ACCEL_CONFIG0,
            Registers::AccelConfig::ACCEL_FS_16 | Registers::AccelConfig::ACCEL_ODR_1kHz);

    vTaskDelay(100);

    if (whoami != WHOAMI_VAL) {
        printf("[ERROR] Failed to connect to IMU.");
        return false;
    } else {
        printf("[INFO] Initialization Complete!");
        return true;
    }
}

double ICM42605::gyro_z() {
    uint16_t hi = read_register(0, Registers::GYRO_DATA_Z1),
            lo = read_register(0, Registers::GYRO_DATA_Z0);

    // Reinterpret the bits as a signed 16-bit integer
    int16_t total = hi << 8 | lo;

    // +-1000dps max, for 16-bit signed integers, comes out to 32.8lsb/deg.
    const double lsbToDps = 1000.0f / 32768.f;
    const double degToRad = M_PI / 180.0f;
    return total * (degToRad * lsbToDps);
}

double ICM42605::accel_x() {
    uint16_t hi = read_register(0, Registers::ACCEL_DATA_X1),
            lo = read_register(0, Registers::ACCEL_DATA_X0);


    // Reinterpret the bits as a signed 16-bit integer
    int16_t total = hi << 8 | lo;

    // +-4g max, for 16-bit signed integers, comes out to 8192lsb/g.
    const double lsbToG = 4.0f / 32768.f;
    return total * lsbToG;
}

double ICM42605::accel_y() {
    uint16_t hi = read_register(0, Registers::ACCEL_DATA_Y1),
            lo = read_register(0, Registers::ACCEL_DATA_Y0);

    // Reinterpret the bits as a signed 16-bit integer
    int16_t total = hi << 8 | lo;

    // +-4g max, for 16-bit signed integers, comes out to 8192lsb/g.
    const double lsbToG = 4.0f / 32768.f;
    return total * lsbToG;
}

void ICM42605::chip_select(bool cs_state) {
    nCs.write(!cs_state);
}

void ICM42605::write_register(uint8_t bank, uint8_t address, uint8_t value) {
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

uint8_t ICM42605::read_register(uint8_t bank, uint8_t address) {
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

void ICM42605::burst_read(uint8_t bank, uint8_t address, uint8_t* buffer_out, size_t length) {
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
