#include "mtrain.h"
#include  <unistd.h>

USBD_HandleTypeDef USBD_Device;

int main(void)
{
    USBD_Init(&USBD_Device, &VCP_Desc, 0);
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
    USBD_Start(&USBD_Device);

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = LED1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_PORT, LED1_PIN, GPIO_PIN_SET);

    fflush(stdout);

    int count = 0;
    while(1) {
        HAL_Delay(100);
        printf("\r%d", count);
        fflush(stdout);
        count++;
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