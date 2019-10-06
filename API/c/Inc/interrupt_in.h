#ifndef __INTERRUPT_IN_H
#define __INTERRUPT_IN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pin_defs.h"

typedef enum {
    INTERRUPT_RISING = GPIO_MODE_IT_RISING,
    INTERRUPT_FALLING = GPIO_MODE_IT_FALLING,
    INTERRUPT_RISING_FALLING = GPIO_MODE_IT_RISING_FALLING
} interrupt_mode;

 /** Configures GPIO pin for interruptin
 *
 * Defaults to no pull type and both edge detection
 *
 * @param pin Pin def external to board
 * @param function Function linked to interrupt
 */
void interruptin_init(pin_name pin, void (*function)()); // void (*function)(void)

 /** Configures GPIO pin for interruptin
 *
 * @param pin Pin def external to board
 * @param function Function linked to interrupt
 * @param pull Pin pull type (pull up, pull down, or no pull)
 * @param mode Interrupt mode (rising edge, falling edge, or both edges)
 */
void interruptin_init_ex(pin_name pin, void (*function)(), pull_type pull,
    interrupt_mode mode);

 /** Deinits GPIO pin
 *
 * @param pin Pin def external to board
 */
void interruptin_deinit(pin_name pin);

 /** Reads current value of pin
 *
 * @param pin Pin def external to board
 * @return 1 (high) or 0 (low)
 */
int interruptin_read(pin_name pin);

#ifdef __cplusplus
}
#endif

#endif
