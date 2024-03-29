#include <optional>

#include "SPI.hpp"
#include "bsp.h"
#include "delay.h"
#include "drivers/ISM43340.hpp"
#include "macro.hpp"
#include "mtrain.hpp"
#include "radio/RadioLink.hpp"

#undef USING_RTOS

int main() {
    std::unique_ptr radioSPI = std::make_unique<SPI>(SpiBus5, std::nullopt, 16'000'000);
    auto radioDriver =
        std::make_unique<ISM43340>(std::move(radioSPI), RADIO_R0_CS, RADIO_GLB_RST, RADIO_R0_INT);

    const char* data = "hello world";
    const unsigned int num_bytes = strlen(data);

    char buffer[1024];

    while (true) {
        delay_from_microseconds(1000);
        if (radioDriver->isAvailable()) {
            int num = radioDriver->receive((unsigned char*)buffer, 1024);
            radioDriver->send((uint8_t*)buffer, num);
        }
    }
}
