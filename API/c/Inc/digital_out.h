#ifndef __DIGITAL_OUT_H
#define __DIGITAL_OUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pin_defs.h"

typedef enum {
    PUSH_PULL = GPIO_MODE_OUTPUT_PP,
    OPEN_DRAIN = GPIO_MODE_OUTPUT_OD
} pin_mode;

typedef enum {
    LOW = GPIO_SPEED_FREQ_LOW,
    MEDIUM = GPIO_SPEED_FREQ_MEDIUM,
    HIGH = GPIO_SPEED_FREQ_HIGH,
    VERY_HIGH = GPIO_SPEED_FREQ_VERY_HIGH
} pin_speed;

/** Configures GPIO pin for digital out, push pull
 *
 * Defaults to push-pull mode, no pull type, low gpio frequency
 * 
 * @param pin Pin def external to board
 */
void digitalout_init(pin_name pin);

/** Configures GPIO pin for digital out
 *
 * @param pin Pin def external to board
 * @param pull Pin pull type
 * @param mode Output mode (push-pull or open-drain)
 * @param speed GPIO frequency
 */
void digitalout_init_ex(pin_name pin, pull_type pull,
    pin_mode mode, pin_speed speed);

/** Deinit GPIO pin
 *
 * @param pin Pin def external to board
 */
void digitalout_deinit(pin_name pin);

/** Change output state of pin
 *
 * @param pin Pin def external to board
 * @param state Set output value of pin (0 or 1)
 *     In open-drain 0 is high-z, 1 is drive low
 */
void digitalout_write(pin_name pin, int state);

/** Toggle pin output
 *
 * @param pin Pin def external to board
 */
void digitalout_toggle(pin_name pin);

/** Read current value of pin
 * 
 * @param pin  Pin def external to board
 * @return 1 (high) or 0 (low)
 */
int digitalout_read(pin_name p);

#ifdef __cplusplus
}
#endif

#endif
