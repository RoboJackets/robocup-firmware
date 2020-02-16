#include "drivers/ICM42605.hpp"
#include <cmath>

#define READ_BIT (1 << 7)

namespace Registers {
    constexpr uint8_t DEVICE_CONFIG = 0x11;
    constexpr uint8_t DRIVE_CONFIG = 0x13;
    constexpr uint8_t INT_CONFIG = 0x14;
    constexpr uint8_t FIFO_CONFIG = 0x1;
    constexpr uint8_t TEMP_DATA1 = 0x1D;
    constexpr uint8_t TEMP_DATA0 = 0x1E;
    constexpr uint8_t ACCEL_DATA_X1 = 0x1F;
    constexpr uint8_t ACCEL_DATA_X0 = 0x20;
    constexpr uint8_t ACCEL_DATA_Y1 = 0x21;
    constexpr uint8_t ACCEL_DATA_Y0 = 0x22;
    constexpr uint8_t ACCEL_DATA_Z1 = 0x23;
    constexpr uint8_t ACCEL_DATA_Z0 = 0x24;
    constexpr uint8_t GYRO_DATA_X1 = 0x25;
    constexpr uint8_t GYRO_DATA_X0 = 0x26;
    constexpr uint8_t GYRO_DATA_Y1 = 0x27;
    constexpr uint8_t GYRO_DATA_Y0 = 0x28;
    constexpr uint8_t GYRO_DATA_Z1 = 0x29;
    constexpr uint8_t GYRO_DATA_Z0 = 0x2A;
    constexpr uint8_t TMST_FSYNCH = 0x2B;
    constexpr uint8_t TMST_FSYNCL = 0x2C;
    constexpr uint8_t INT_STATUS = 0x2D;
    constexpr uint8_t FIFO_COUNTH = 0x2E;
    constexpr uint8_t FIFO_COUNTL = 0x2F;
    constexpr uint8_t FIFO_DATA = 0x30;
    constexpr uint8_t APEX_DATA1 = 0x32;
    constexpr uint8_t APEX_DATA2 = 0x33;
    constexpr uint8_t APEX_DATA3 = 0x34;
    constexpr uint8_t APEX_DATA4 = 0x35;
    constexpr uint8_t APEX_DATA5 = 0x36;
    constexpr uint8_t INT_STATUS2 = 0x37;
    constexpr uint8_t INT_STATUS3 = 0x38;
    constexpr uint8_t SIGNAL_PATH_RESET = 0x4B;
    constexpr uint8_t INTF_CONFIG0 = 0x4C;
    constexpr uint8_t INTF_CONFIG1 = 0x4D;
    constexpr uint8_t PWR_MGMT0 = 0x4E;
    constexpr uint8_t GYRO_CONFIG0 = 0x4F;
    constexpr uint8_t ACCEL_CONFIG0 = 0x50;
    constexpr uint8_t GYRO_CONFIG1 = 0x51;
    constexpr uint8_t GYRO_ACCEL_CONFIG0 = 0x52;
    constexpr uint8_t ACCEL_CONFIG1 = 0x53;
    constexpr uint8_t TMST_CONFIG = 0x54;
    constexpr uint8_t APEX_CONFIG0 = 0x56;
    constexpr uint8_t SMD_CONFIG = 0x57;
    constexpr uint8_t FIFO_CONFIG1 = 0x5F;
    constexpr uint8_t FIFO_CONFIG2 = 0x60;
    constexpr uint8_t FIFO_CONFIG3 = 0x61;
    constexpr uint8_t FSYNC_CONFIG = 0x62;
    constexpr uint8_t INT_CONFIG0 = 0x63;
    constexpr uint8_t INT_CONFIG1 = 0x64;
    constexpr uint8_t INT_SOURCE0 = 0x65;
    constexpr uint8_t INT_SOURCE1 = 0x66;
    constexpr uint8_t INT_SOURCE2 = 0x67;
    constexpr uint8_t INT_SOURCE3 = 0x68;
    constexpr uint8_t INT_SORUCE4 = 0x69;
    constexpr uint8_t FIFO_LOST_PKTO = 0x6C;
    constexpr uint8_t FIFO_LOST_PKT1 = 0x6D;
    constexpr uint8_t SELF_TEST_CONFIG = 0x70;
    constexpr uint8_t WHO_AM_I = 0x75;
    constexpr uint8_t REG_BANK_SEL = 0x76;

    constexpr uint8_t INTF_CONFIG4 = 0x7A;
    constexpr uint8_t INTF_CONFIG6 = 0x7C;

    // Bank 1
    constexpr uint8_t OTP_SEC_STATUS = 0x70;

    constexpr uint8_t BIT_STATUS_RESET_DONE = 0x10;

    enum SpiSlewRate {
        NS_20_TO_60 = 0,
        NS_12_TO_36 = 1,
        NS_6_TO_18 = 2,
        NS_4_TO_12 = 3,
        NS_2_TO_6 = 4,
        NS_LT_2 = 5
    };

    enum GyroFS {
        DPS_2000 = 0,
        DPS_1000 = 1,
        DPS_500 = 2,
        DPS_250 = 3,
        DPS_125 = 4,
        DPS_62_5 = 5,
        DPS_32_25 = 6,
        DPS_15_625 = 7
    };

    enum GyroODR {
        KHZ_8 = 3,
        KHZ_4 = 4,
        KHZ_2 = 5,
        KHZ_1 = 6,
        HZ_200 = 7,
        HZ_100 = 8,
        HZ_50 = 9,
        HZ_25 = 10,
        HZ_12_5 = 11,
        HZ_500 = 15
    };

    enum GyroTempFiltBW {
        HZ_4000 = 0,
        HZ_170 = 1,
        HZ_82 = 2,
        HZ_40 = 3,
        HZ_20 = 4,
        HZ_10 = 5,
        HZ_5 = 6
    };

    enum GyroFiltOrd {
        ORD_1 = 0,
        ORD_2 = 1,
        ORD_3 = 2
    };

    enum GyroMode {
        OFF = 0,
        STANDBY = 1,
        LOW_NOISE = 3
    };
};

ICM42605::ICM42605(std::shared_ptr<SPI> spi_bus, PinName cs_pin)
    : spi_bus(spi_bus),
      chip_select_pin(cs_pin, PullType::PullNone, PinMode::PushPull, PinSpeed::Low, true) {
    chip_select_pin.write(false);
    HAL_Delay(1500);
}

void ICM42605::initialize() {
    // chip_select(true);

    write_register(Registers::REG_BANK_SEL, 1);
    write_register(Registers::INTF_CONFIG4, 2);

    // The ICM42605 has several banks of registers, each with a different set of
    // properties. The bank in use will be selected by REG_BANK_SEL.
    //
    // Select register bank 0. All of our access is to this register, but it's
    // a good idea to select it anyway in case we were in an invalid state.
    write_register(Registers::REG_BANK_SEL, 0);

    write_register(Registers::DEVICE_CONFIG, 1);

    HAL_Delay(1);

    write_register(Registers::REG_BANK_SEL, 1);

    uint8_t data;

    printf("Start waiting");
    do {
        write_register(Registers::INTF_CONFIG4, 2);
        read_register(Registers::OTP_SEC_STATUS);
        HAL_Delay(1);
    } while (!(data & Registers::BIT_STATUS_RESET_DONE));
    printf("Boot complete\n");

    write_register(Registers::REG_BANK_SEL, 0);

    // Maybe reading this bit clears it?
    read_register(Registers::INT_STATUS);

    // Disable things
    write_register(Registers::REG_BANK_SEL, 1);
    write_register(Registers::INTF_CONFIG6, 0);

    // Enable the gyro in low-noise mode.
    uint8_t old = read_register(Registers::PWR_MGMT0);
    write_register(Registers::PWR_MGMT0, Registers::GyroMode::LOW_NOISE << 2);

    // Check WHO_AM_I register. It shouldn't have changed from the default value.
    uint8_t id = read_register(Registers::WHO_AM_I);
    if (id != 0x42) {
        // Error handling
    }

    // Run a self-test on the gyro z-axis only.
    write_register(Registers::SELF_TEST_CONFIG, 1 << 6 | 1 << 2);

    // Set gyro parameters.
    // Default is 2000 degrees/s and 1kHz. That should be fine for now.
    write_register(
            Registers::GYRO_CONFIG0,
            Registers::GyroFS::DPS_2000 << 5 | Registers::GyroODR::KHZ_1);

    // Default filtering is 4000Hz bandwidth DLPF on the temperature filter.
    // Keep that because we want low latency.
    write_register(
            Registers::GYRO_CONFIG1,
            Registers::GyroTempFiltBW::HZ_4000 << 5
            | Registers::GyroFiltOrd::ORD_1 << 2
            | Registers::GyroFiltOrd::ORD_3);

    // chip_select(false);
}

float ICM42605::gyro_z() {
    // chip_select(true);

    uint16_t hi = read_register(Registers::WHO_AM_I),
             lo = read_register(Registers::WHO_AM_I);

    // chip_select(false);

    // Reinterpret the bits as a signed 16-bit integer
    int16_t total = hi << 8 | lo;

    // +-2000dps max, for 16-bit signed integers, comes out to 32.8lsb/deg.
    const float lsbToDps = 1.0f / 16.4f;
    const float degToRad = M_PI / 180.0f;
    return total * (degToRad * lsbToDps);
}

uint8_t ICM42605::read_register(uint8_t address) {
    bool was_cs_active = currently_active;

    chip_select(true);

    uint8_t data_tx[2] = {address | READ_BIT, 0x0};
    uint8_t data_rx[2] = {0x0, 0x0};
    spi_bus->transmitReceive(data_tx, data_rx, 2);

    printf("%x%x -> %x%x\r\n", data_tx[0], data_tx[1], data_rx[0], data_rx[1]);

    // Restore previous CS state
    chip_select(was_cs_active);

    return data_rx[1];
}

void ICM42605::write_register(uint8_t address, uint8_t value) {
    bool was_cs_active = currently_active;

    chip_select(true);

    uint8_t data_tx[2] = {address, value};
    uint8_t data_rx[2] = {0x0, 0x0};
    spi_bus->transmitReceive(data_tx, data_rx, 2);

    // Restore previous CS state
    chip_select(was_cs_active);
}

void ICM42605::chip_select(bool active) {
    if (currently_active != active) {
        currently_active = active;
        chip_select_pin.write(active);
    }
}
