#include "mtrain.hpp"
#include "drivers/TMC4671.hpp"
#include <delay.h>

void tmc_task(void*) {
    LockedStruct<SPI> spi(SpiBus::SpiBus2, std::nullopt, 100'000);
    TMC4671 tmc{spi, p8};

    vTaskDelay(100);
    printf("Initializing\r\n");

    tmc.initialize();
}

int main() {
    DWT_Delay(5'000'000);
    xTaskHandle handle;
    xTaskCreate(&tmc_task, "TMC4671", 1024, nullptr, 1, &handle);
    printf("Starting scheduler\r\n");
    vTaskStartScheduler();
    printf("Died\r\n");
    for (;;) {}
}
