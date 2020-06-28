int main() {}
/*#include "mtrain.hpp"

int main() {
    std::shared_ptr<SPI> imuSPI = std::make_shared<SPI>(SpiBus::SpiBus2, std::nullopt, 100'000);
    DigitalOut nCs = p18;
    nCs = 1;

    // SPI pulse to clear pins.
    imuSPI->transmit(0);

    HAL_Delay(100);

    uint8_t whoami = 0;
    while (whoami != 0xEA) {
        printf("Reading WHOAMI\r\n");
        // Bank select 0
        nCs.write(0);
        imuSPI->transmit(0x7F);
        imuSPI->transmit(0x00);
        nCs.write(1);

        HAL_Delay(1);

        // Read WHOAMI
        nCs.write(0);
        imuSPI->transmit(RD | 0x00);
        whoami = imuSPI->transmitReceive(0x00);
        nCs.write(1);

        HAL_Delay(1);
    }

    // Reset
    nCs.write(0);
    imuSPI->transmit(0x06);
    imuSPI->transmit(0xC1);
    nCs.write(1);

    // Wait for reset
    whoami = 0;
    while (whoami != 0xEA) {
        printf("Reading WHOAMI\r\n");
        // Bank select 0
        nCs.write(0);
        imuSPI->transmit(0x7F);
        imuSPI->transmit(0x00);
        nCs.write(1);

        HAL_Delay(1);

        // Read WHOAMI
        nCs.write(0);
        imuSPI->transmit(RD | 0x00);
        whoami = imuSPI->transmitReceive(0x00);
        nCs.write(1);

        HAL_Delay(1);
    }

    // LPConfig
    nCs.write(0);
    imuSPI->transmit(0x06);
    imuSPI->transmit(0x01);
    nCs.write(1);

    HAL_Delay(1);

    // Operate in duty cycled mode
    nCs.write(0);
    imuSPI->transmit(0x05);
    imuSPI->transmit(0x40);
    nCs.write(1);

    HAL_Delay(1);

    // Disable I2C
    nCs.write(0);
    imuSPI->transmit(0x03);
    imuSPI->transmit(0x10);
    nCs.write(1);

    whoami = 0;
    while (whoami != 0xEA) {
        // Read WHOAMI
        nCs.write(0);
        imuSPI->transmit(RD | 0x00);
        whoami = imuSPI->transmitReceive(0x00);
        nCs.write(1);

        HAL_Delay(1);
    }

    // FIFO off
    nCs.write(0);
    imuSPI->transmit(0x69);
    imuSPI->transmit(0x00);
    nCs.write(1);

    HAL_Delay(1);

    // Don't disable gyro
    nCs.write(0);
    imuSPI->transmit(0x07);
    imuSPI->transmit(0x00);
    nCs.write(1);

    HAL_Delay(1);

    // Bank select 2
    nCs.write(0);
    imuSPI->transmit(0x7F);
    imuSPI->transmit(0x20);
    nCs.write(1);

    HAL_Delay(1);

    // Gyro config
    nCs.write(0);
    imuSPI->transmit(0x01);
    imuSPI->transmit(0b00'000'10'0);
    nCs.write(1);

    HAL_Delay(1);

    // Bank select 0
    nCs.write(0);
    imuSPI->transmit(0x7F);
    imuSPI->transmit(0x00);
    nCs.write(1);

    HAL_Delay(100);

    while (true) {
        // Bank select 0
        nCs.write(0);
        imuSPI->transmit(0x7F);
        imuSPI->transmit(0x00);

        nCs.write(1);
        HAL_Delay(1);
        nCs.write(0);

        imuSPI->transmit(RD | 0x37);
        uint16_t hi = imuSPI->transmitReceive(0);

        nCs.write(1);
        HAL_Delay(1);
        nCs.write(0);

        imuSPI->transmit(RD | 0x38);
        uint16_t lo = imuSPI->transmitReceive(0);

        nCs.write(1);
        HAL_Delay(1);
        nCs.write(0);

        imuSPI->transmit(RD | 0x00);
        uint8_t whoami = imuSPI->transmitReceive(0);

        nCs.write(1);

        int result = (short) (hi << 8 | lo);
        printf("Got whoami: %x\r\n", whoami);
        printf("Got data: %d (%x %x)\r\n", result, hi, lo);
        HAL_Delay(100);
    }
}*/
