#include "common.hpp"
#include <optional>

#include "drivers/ISM43340.hpp"

int main() {
    DigitalOut l1 = DigitalOut(LED1);

    l1 = 1;

    printf("[INFO] Starting\r\n");

    std::unique_ptr radioSPI = std::make_unique<SPI>(SpiBus5, std::nullopt, 16'000'000);
    ISM43340 radioDriver(std::move(radioSPI), RADIO_R0_CS, RADIO_GLB_RST, RADIO_R0_INT);

    radioDriver.pingRouter();
    radioDriver.testPrint();

    printf("[INFO] Finished\r\n");

  while (true) {
      l1.toggle();
      HAL_Delay(1000);
  }
}
