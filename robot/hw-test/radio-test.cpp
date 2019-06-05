#include "mtrain.hpp"
#include "SPI.hpp"
#include <vector>
#include <optional>

#include "drivers/ISM43340.hpp"
#include "bsp.h"

//for debugging
#include  <unistd.h>
USBD_HandleTypeDef USBD_Device;


int main() {
    USBD_Init(&USBD_Device, &VCP_Desc, 0);
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
    USBD_Start(&USBD_Device);

    HAL_Delay(5000);

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

extern "C" {

int _write(int file, char *data, int len)
{
    if (file == STDOUT_FILENO && USBD_Device.dev_state == USBD_STATE_CONFIGURED ) {
        USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)data, len);
        USBD_CDC_TransmitPacket(&USBD_Device);
    }
    return 0;
}

}