#include "mtrain.hpp"

#include "bsp.h"
#include  <unistd.h>

#include "PinDefs.hpp"

USBD_HandleTypeDef USBD_Device;

int main() {
    USBD_Init(&USBD_Device, &VCP_Desc, 0);
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
    USBD_Start(&USBD_Device);

    DigitalOut l1 = DigitalOut(LED2);

    fflush(stdout);

    uint8_t i = 0;

    for (;;) {
        l1 = 0;
        
        HAL_Delay(50);
        
        l1 = 1;

        HAL_Delay(50);

        printf("%u\r\n", i++);
        fflush(stdout);
    }
}

extern "C" {

int _write(int file, char *data, int len)
{
    if (file == STDOUT_FILENO) {
        USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)data, len);
        USBD_CDC_TransmitPacket(&USBD_Device);
    }
    return 0;
}
}
