#include <optional>

#include "SPI.hpp"
#include "bsp.h"
#include "delay.h"
#include "drivers/ISM43340.hpp"
#include "macro.hpp"
#include "mtrain.hpp"
#include "radio/RadioLink.hpp"
#include "drivers/FPGA.hpp"

#undef USING_RTOS

int main() {
    static std::unique_ptr<SPI> fpgaSPI = std::make_unique<SPI>(FPGA_SPI_BUS, std::nullopt, 16'000'000);
    auto fpga = FPGA(std::move(fpgaSPI), FPGA_CS, FPGA_INIT, FPGA_PROG, FPGA_DONE);

    fpga.configure();
    int16_t cycle = fpga.MAX_DUTY_CYCLE / 50;
    std::array<int16_t, 5> dutyCycles{cycle, cycle, cycle, cycle, cycle};
    std::array<int16_t, 5> encDeltas{};
    while (true) {
        uint8_t status = fpga.set_duty_get_enc(
                dutyCycles.data(), dutyCycles.size(),
                encDeltas.data(), encDeltas.size());
        printf("Status\r\n: %d", status);
    }
}
