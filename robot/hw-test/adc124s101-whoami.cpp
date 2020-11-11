#include "mtrain.hpp"
#include "drivers/ADC124S101.hpp"
#include <delay.h>

void adc_task(void*) {
    LockedStruct<SPI> spi(SpiBus::SpiBus3, std::nullopt, 100'000);
    ADC124S101 adc{spi, p31};

    vTaskDelay(100);
    printf("Initializing\r\n");

    adc.initialize();
}

int main() {
    DWT_Delay(5'000'000);
    xTaskHandle handle;
    xTaskCreate(&adc_task, "ADC", 1024, nullptr, 1, &handle);
    printf("Starting scheduler\r\n");
    vTaskStartScheduler();
    printf("Died\r\n");
    for (;;) {}
}
