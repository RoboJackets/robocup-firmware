#ifndef __DIGITAL_IN_H
#define __DIGITAL_IN_H

#include "pin_defs.h"

/**
* Initializes Digital In
*
* @param pin_name pin
* @default pull_type PULL_NONE, pin_mode GPIO_MODE_INPUT
*/
void digitalin_init(pin_name p);
/**
* Initializes Digital In
*
* @param pin_name pin, pull_type
* @default pin_mode GPIO_MODE_INPUT
*/
void digitalin_init_ex(pin_name p, pull_type type);
/**
* Deinitalizes Digital In
*
* @param pin_name pin
*/
void digitalin_deinit(pin_name p);

/**
* Returns pin state
*
* @param pin_name pin
* @return int as 1 (high) or 0 (low)
*/
int digitalin_read(pin_name p);

#endif
