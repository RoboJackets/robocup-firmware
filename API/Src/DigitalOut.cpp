#include "DigitalOut.hpp"

DigitalOut::DigitalOut(PinName pin, PullType pull, PinMode mode, PinSpeed speed,
        bool inverted) : pin(pin), inverted(inverted) {
    GPIO_InitTypeDef pin_structure = {};
    pin_structure.Pin = pin.pin;
    pin_structure.Mode = mode;
    pin_structure.Pull = pull;
    pin_structure.Speed = speed;
    
    HAL_GPIO_Init(pin.port, &pin_structure);
}
DigitalOut::~DigitalOut() {
    HAL_GPIO_DeInit(pin.port, pin.pin);
}

void DigitalOut::write(bool state) {
    HAL_GPIO_WritePin(pin.port, pin.pin, (GPIO_PinState)(state != inverted));
}

void DigitalOut::toggle() {
    HAL_GPIO_TogglePin(pin.port, pin.pin);
}

bool DigitalOut::read() {
    return HAL_GPIO_ReadPin(pin.port, pin.pin);
}
