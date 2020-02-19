#include "mtrain.hpp"
#include "SPI.hpp"
#include "drivers/ICM20948.hpp"

int main() {
    std::shared_ptr<SPI> imuSPI = std::make_shared<SPI>(SpiBus::SpiBus2, std::nullopt, 100'000);
    ICM20948 imu(imuSPI, p18);
    
    imu.initialize();

    while(true) {
        float imu_data = imu.gyro_z();
        printf("Data received: %d\r\n", (int) (imu_data * 1000));
    }    
}
