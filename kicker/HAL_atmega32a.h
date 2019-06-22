#ifndef HAL_ATMEGA32A_H
#define HAL_ATMEGA32A_H

#include <stdbool.h>
#include <avr/io.h>

typedef struct pin_name {
    uint32_t reg;
    uint8_t pin;
} pin_name;

#define PIN_A0 (pin_name) {PORTA, PA0}
#define PIN_A1 (pin_name) {PORTA, PA1}
#define PIN_A2 (pin_name) {PORTA, PA2}
#define PIN_A3 (pin_name) {PORTA, PA3}
#define PIN_A4 (pin_name) {PORTA, PA4}
#define PIN_A5 (pin_name) {PORTA, PA5}
#define PIN_A6 (pin_name) {PORTA, PA6}
#define PIN_A7 (pin_name) {PORTA, PA7}

#define PIN_B0 (pin_name) {PORTB, PB0}
#define PIN_B1 (pin_name) {PORTB, PB1}
#define PIN_B2 (pin_name) {PORTB, PB2}
#define PIN_B3 (pin_name) {PORTB, PB3}
#define PIN_B4 (pin_name) {PORTB, PB4}
#define PIN_B5 (pin_name) {PORTB, PB5}
#define PIN_B6 (pin_name) {PORTB, PB6}
#define PIN_B7 (pin_name) {PORTB, PB7}

#define PIN_C0 (pin_name) {PORTC, PC0}
#define PIN_C1 (pin_name) {PORTC, PC1}
#define PIN_C2 (pin_name) {PORTC, PC2}
#define PIN_C3 (pin_name) {PORTC, PC3}
#define PIN_C4 (pin_name) {PORTC, PC4}
#define PIN_C5 (pin_name) {PORTC, PC5}
#define PIN_C6 (pin_name) {PORTC, PC6}
#define PIN_C7 (pin_name) {PORTC, PC7}

#define PIN_D0 (pin_name) {PORTD, PD0}
#define PIN_D1 (pin_name) {PORTD, PD1}
#define PIN_D2 (pin_name) {PORTD, PD2}
#define PIN_D3 (pin_name) {PORTD, PD3}
#define PIN_D4 (pin_name) {PORTD, PD4}
#define PIN_D5 (pin_name) {PORTD, PD5}
#define PIN_D6 (pin_name) {PORTD, PD6}
#define PIN_D7 (pin_name) {PORTD, PD7}

/**
 * Most LED's are inverted so this deals with that
 * in a nice way
 */
void HAL_SetLED(pin_name p, bool on);
bool HAL_IsSet(pin_name p);
void HAL_SetPin(pin_name p);
void HAL_ClearPin(pin_name p);

#endif /* HAL_ATMEGA32A_H */