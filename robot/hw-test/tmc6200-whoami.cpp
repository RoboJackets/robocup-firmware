#include "mtrain.hpp"
#include "drivers/TMC6200.hpp"
#include <delay.h>

void tmc_task(void*) {
    LockedStruct<SPI> spi(SpiBus::SpiBus2, std::nullopt, 100'000);
    TMC6200 tmc{spi, p7};

    vTaskDelay(100);
    printf("Initializing\r\n");

    tmc.initialize();
    printf("Initialized\r\n");
    uint8_t version = tmc.read_register(Registers::IOIN) & Registers::IOIn::VERSION;
    printf("Version: %u\r\n", version);
    while (true) {
        printf("Pin Status [OT150C/OT143C/OT136C]: %lx \r\n", tmc.read_register(Registers::IOIN) & (0b111 << 8));
        printf("Pin Status [0/DRV_EN/WH/WL/VH/VL/UH/UL]: %lx \r\n", tmc.read_register(Registers::IOIN) & 0b111111);
    }
}

int main() {
    DWT_Delay(5'000'000);
    xTaskHandle handle;
    xTaskCreate(&tmc_task, "TMC6200", 1024, nullptr, 1, &handle);
    printf("Starting scheduler\r\n");
    vTaskStartScheduler();
    printf("Died\r\n");
    for (;;) {}
}
