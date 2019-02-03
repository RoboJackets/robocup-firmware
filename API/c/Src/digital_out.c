#include "digital_out.h"

void digitalout_init(pin_name pin) {
    GPIO_InitTypeDef pin_structure;
    pin_structure.Pin = pin.pin;
    pin_structure.Mode = GPIO_MODE_OUTPUT_PP;
    pin_structure.Pull = GPIO_NOPULL;
    pin_structure.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(pin.port, &pin_structure);
}

void digitalout_init_ex(pin_name pin, pull_type pull, pin_mode mode, pin_speed speed) {
    GPIO_InitTypeDef pin_structure;
    pin_structure.Pin = pin.pin;
    pin_structure.Mode = mode;
    pin_structure.Pull = pull;
    pin_structure.Speed = speed;
    
    HAL_GPIO_Init(pin.port, &pin_structure);
}

void digitalout_deinit(pin_name pin) {
    HAL_GPIO_DeInit(pin.port, pin.pin);
}

void digitalout_write(pin_name pin, int state) {
    HAL_GPIO_WritePin(pin.port, pin.pin, state != 0);
}

void digitalout_toggle(pin_name pin) {
    HAL_GPIO_TogglePin(pin.port, pin.pin);
}

int digitalout_read(pin_name pin) {
    return HAL_GPIO_ReadPin(pin.port, pin.pin);
}
