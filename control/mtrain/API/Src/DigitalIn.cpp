#include "DigitalIn.hpp"

DigitalIn::DigitalIn(PinName pin, PullType pull) : pin(pin) {
    GPIO_InitTypeDef pin_structure;
    pin_structure.Pin = pin.pin;
    pin_structure.Mode = GPIO_MODE_INPUT;
    pin_structure.Pull = pull;
    
    HAL_GPIO_Init(pin.port, &pin_structure);
}
DigitalIn::~DigitalIn() {
    HAL_GPIO_DeInit(pin.port, pin.pin);
}

bool DigitalIn::read() {
    return HAL_GPIO_ReadPin(pin.port, pin.pin);
}
