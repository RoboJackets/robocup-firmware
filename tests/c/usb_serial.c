#include "mtrain.h"

#include "bsp.h"

//for debugging
#include  <unistd.h>
USBD_HandleTypeDef USBD_Device;

int main(void) {

    digitalout_init(LED1);
    digitalout_init(LED2);
    digitalout_init(LED3);
    digitalout_init(LED4);
    
    pin_name leds[] = {LED1, LED2, LED3, LED4};
    
    USBD_Init(&USBD_Device, &VCP_Desc, 0);
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
    USBD_Start(&USBD_Device);

    fflush(stdout);

    for (;;) {
        digitalout_toggle(leds[0]);
        HAL_Delay(5);
        digitalout_toggle(leds[1]);
        HAL_Delay(5);
        digitalout_toggle(leds[2]);
        HAL_Delay(5);
        
        digitalout_toggle(leds[0]);
        HAL_Delay(5);
        digitalout_toggle(leds[1]);
        HAL_Delay(5);
        digitalout_toggle(leds[2]);
        HAL_Delay(5);

        printf("t\r\n");
        printf("a\r\n");
        fflush(stdout);
    }
}

int _write(int file, char *data, int len)
{
    if (file == STDOUT_FILENO) {
        USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)data, len);
        USBD_CDC_TransmitPacket(&USBD_Device);
    }
    return 0;
}
