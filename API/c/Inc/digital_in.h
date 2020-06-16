#ifndef __DIGITAL_IN_H
#define __DIGITAL_IN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pin_defs.h"

/** Configures GPIO pin for digital in
 * 
 * Defaults to no pull type
 * 
 * @param pin Pin def external to board
 */
void digitalin_init(pin_name pin);

/** Configures GPIO pin for digital in
 * 
 * @param pin Pin def external to board
 * @param pull Pin pull type
 */
void digitalin_init_ex(pin_name pin, pull_type pull);

/** Deinit GPIO pin
 *
 * @param pin  Pin def external to board
 */
void digitalin_deinit(pin_name pin);

/** Read current value of pin
 * 
 * @param pin  Pin def external to board
 * @return 1 (high) or 0 (low)
 */
int digitalin_read(pin_name pin);

#ifdef __cplusplus
}
#endif

#endif

