#ifndef HAL_ATTINY167_H
#define HAL_ATTINY167_H

#include <stdbool.h>
#include <avr/io.h>

typedef struct pin_name {
    volatile uint8_t *dd_reg;
    volatile uint8_t *port_reg;
    volatile uint8_t *pin_reg;
    volatile uint8_t pin;
} pin_name;

#define PIN_A0 (pin_name) {&DDRA, &PORTA, &PINA, PA0}
#define PIN_A1 (pin_name) {&DDRA, &PORTA, &PINA, PA1}
#define PIN_A2 (pin_name) {&DDRA, &PORTA, &PINA, PA2}
#define PIN_A3 (pin_name) {&DDRA, &PORTA, &PINA, PA3}
#define PIN_A4 (pin_name) {&DDRA, &PORTA, &PINA, PA4}
#define PIN_A5 (pin_name) {&DDRA, &PORTA, &PINA, PA5}
#define PIN_A6 (pin_name) {&DDRA, &PORTA, &PINA, PA6}
#define PIN_A7 (pin_name) {&DDRA, &PORTA, &PINA, PA7}

#define PIN_B0 (pin_name) {&DDRB, &PORTB, &PINB, PB0}
#define PIN_B1 (pin_name) {&DDRB, &PORTB, &PINB, PB1}
#define PIN_B2 (pin_name) {&DDRB, &PORTB, &PINB, PB2}
#define PIN_B3 (pin_name) {&DDRB, &PORTB, &PINB, PB3}
#define PIN_B4 (pin_name) {&DDRB, &PORTB, &PINB, PB4}
#define PIN_B5 (pin_name) {&DDRB, &PORTB, &PINB, PB5}
#define PIN_B6 (pin_name) {&DDRB, &PORTB, &PINB, PB6}
#define PIN_B7 (pin_name) {&DDRB, &PORTB, &PINB, PB7}

/**
 * Most LED's are inverted so this deals with that
 * in a nice way
 */
void HAL_SetLED(pin_name p, bool on);
bool HAL_IsSet(pin_name p);
void HAL_SetPin(pin_name p);
void HAL_ClearPin(pin_name p);
void HAL_SetInputPin(pin_name p);
void HAL_SetOutputPin(pin_name p);

#endif /* HAL_ATTINY167_H */