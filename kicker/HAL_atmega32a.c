#include "HAL_atmega32a.h"

void HAL_SetLED(pin_name p, bool on) {
    if (on)
        HAL_ClearPin(p);
    else
        HAL_SetPin(p);
}

bool HAL_IsSet(pin_name p) {
    return p.reg & _BV(p.pin);
}

void HAL_SetPin(pin_name p) {
    p.reg |= _BV(p.pin);
}

void HAL_ClearPin(pin_name p) {
    p.reg &= ~_BV(p.pin);
}