#include "mtrain.hpp"
#include "SPI.hpp"
#include <vector>

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

    fflush(stdout);


    DigitalOut l1 = DigitalOut(LED1);
    DigitalOut l2 = DigitalOut(LED2);
    DigitalOut l3 = DigitalOut(LED3);
    l1 = 1;

    HAL_Delay(10000);

    l2 = 1;
    printf("testing");
    fflush(stdout);

    SPI radioSPI(SpiBus5, p17, 6'000'000);
    ISM43340 radioDriver(radioSPI, p17, p19, p18);

    bool test = radioDriver.selfTest();
    // bool test = true;

    l2 = 1;
    l3 = test;

  while (true) { }
  /*

        for (int i = 0; i <= 100; i++) {
            spi2.transmitReceive(i);
        }

        spi2.frequency(8'000'000);

        std::vector<uint8_t> nums;
        for (int i = 1; i <= 100; i++) {
            nums.push_back(i);
        }
        spi2.transmitReceive(nums);

        spi2.frequency(1);
        for (uint8_t i = 0; i <= 100; i++) {
            spi2.transmit(i);
        }

        spi2.frequency(15'000'000);
        spi2.transmit(nums);
  */
}


int _write(int file, char *data, int len)
{
    if (file == STDOUT_FILENO) {
        USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)data, len);
        USBD_CDC_TransmitPacket(&USBD_Device);
    }
    return 0;
}
