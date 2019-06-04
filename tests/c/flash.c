#include "mtrain.h"
#include "bsp.h"


#define BUFFER_SIZE 10
#define ADDR 0x0000

int main(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    GPIO_InitStruct.Pin = LED1.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED1.port, &GPIO_InitStruct);

    fflush(stdout);
    HAL_Delay(3000);

    uint8_t out = BSP_QSPI_Init();
    printf("Out1: %d\r\n", out);

    out = BSP_print_regs();
    printf("Out1a: %d\r\n", out);

    uint8_t data[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        data[i] = 0;
    }
    // for (int i = 0; i < BUFFER_SIZE; i++) {
    //     printf(" %x", data[i]);
    // }
    // printf("\r\n");

    out = BSP_QSPI_Read(data, ADDR, BUFFER_SIZE);
    printf("Out2: %d\r\n", out);

    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf(" %x", data[i]);
    }
    printf("\r\n");

    uint8_t start = 0;
    if (data[1] + BUFFER_SIZE < 0xFF) {
        start = data[1];
    }

    out = BSP_QSPI_Erase_Sector(ADDR);
    // out = BSP_QSPI_Erase_Chip();
    printf("Out3: %d\r\n", out);

    out = BSP_QSPI_Read(data, ADDR, BUFFER_SIZE);
    printf("Out4: %d\r\n", out);

    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf(" %x", data[i]);
    }
    printf("\r\n");


    for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
        data[i] = i + start;
    }

    out = BSP_QSPI_Write(data, ADDR, BUFFER_SIZE);
    printf("Out5: %d\r\n", out);

    out = BSP_QSPI_Read(data, ADDR, BUFFER_SIZE);
    printf("Out6: %d\r\n", out);

    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf(" %x", data[i]);
    }
    printf("\r\n");



    HAL_GPIO_WritePin(LED1.port, LED1.pin, GPIO_PIN_SET);

    printf("\r\n");

    while(1) {
    }
}
