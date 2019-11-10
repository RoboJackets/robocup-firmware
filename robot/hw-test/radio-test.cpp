#include "mtrain.hpp"
#include "SPI.hpp"
#include <vector>
#include <optional>

#include "drivers/ISM43340.hpp"
#include "bsp.h"

int main() {
    DigitalOut l1 = DigitalOut(LED1);

    l1 = 1;

    LOG(LOG_INFO, LOG_MAIN, "Starting\r\n");


    fflush(stdout);

    std::shared_ptr<SPI> radioSPI = std::make_shared<SPI>(SpiBus5, std::nullopt, 1'000'000);
    ISM43340 radioDriver(radioSPI, p17, p19, p30);

    radioDriver.pingRouter();
    radioDriver.testPrint();

    LOG(LOG_INFO, LOG_MAIN, "Finished\r\n");

  while (true) {
      l1.toggle();
      HAL_Delay(100);
  }
}