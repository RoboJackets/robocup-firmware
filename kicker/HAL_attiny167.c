#include "HAL_attiny167.h"

void HAL_SetLED(pin_name p, bool on) {
    if (on)
        HAL_ClearPin(p);
    else
        HAL_SetPin(p);
}

bool HAL_IsSet(pin_name p) {
    return *p.pin_reg & _BV(p.pin);
}

void HAL_SetPin(pin_name p) {
    *p.port_reg |= _BV(p.pin);
}

void HAL_ClearPin(pin_name p) {
    *p.port_reg &= ~_BV(p.pin);
}

void HAL_SetInputPin(pin_name p) {
    *p.dd_reg &= ~_BV(p.pin);
}

void HAL_SetOutputPin(pin_name p) {
    *p.dd_reg |= _BV(p.pin);
}