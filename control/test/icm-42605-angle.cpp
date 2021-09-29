#include "mtrain.hpp"
#include "drivers/ICM42605.hpp"
#include <delay.hpp>

void imu_task(void*) {
    LockedStruct<SPI> spi(SpiBus::SpiBus2, std::nullopt, 100'000);
    ICM42605 imu{spi, p18};

    vTaskDelay(100);
    printf("Initializing\r\n");

    while (!imu.initialize()) {
        printf("Failed to initialize\r\n");
    }

    printf("Initialized\r\n");

    float angle = 0;
    int i = 0;
    while(true) {
        float imu_data = imu.gyro_z();
        vTaskDelay(5);
        angle += 0.005 * imu_data;

        if (i++ % 20 == 0) {
            printf("Angle: %f\r\n", angle);
        }
    }
}

int main() {
    DWT_Delay(5'000'000);
    xTaskHandle handle;
    xTaskCreate(&imu_task, "IMU", 1024, nullptr, 1, &handle);
    printf("Starting scheduler\r\n");
    vTaskStartScheduler();
    printf("Died\r\n");
    for (;;) {}
}
