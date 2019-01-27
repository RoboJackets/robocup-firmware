#include "digital_in.h"

void digitalin_init(pin_name pin)  {
    GPIO_InitTypeDef pin_structure;
    pin_structure.Pin = pin.pin;
    pin_structure.Mode = GPIO_MODE_INPUT;
    pin_structure.Pull = GPIO_NOPULL;
    
    HAL_GPIO_Init(pin.port, &pin_structure);
}

void digitalin_init_ex(pin_name pin, pull_type pull)  {
    GPIO_InitTypeDef pin_structure;
    pin_structure.Pin = pin.pin;
    pin_structure.Mode = GPIO_MODE_INPUT;
    pin_structure.Pull = pull;
    
    HAL_GPIO_Init(pin.port, &pin_structure);
}

void digitalin_deinit(pin_name pin) {
    HAL_GPIO_DeInit(pin.port, pin.pin);
}

int digitalin_read(pin_name pin) {
    return HAL_GPIO_ReadPin(pin.port, pin.pin);
}
