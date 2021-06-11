#include "mtrain.hpp"
#include "drivers/ICM20948.hpp"

int main() {
    LockedStruct<SPI> spi(SpiBus::SpiBus2, std::nullopt, 100'000);
    ICM20948 imu{spi, p18};

    imu.initialize();

    while(true) {
        float imu_data = imu.gyro_z();
        printf("Data received: %d\r\n", int(imu_data * 1000));
    }
}
