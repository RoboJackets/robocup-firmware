#ifndef __PIN_DEFS_H
#define __PIN_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

// TODO: find a way to scope this
#include "stm32f7xx_hal.h"

typedef struct pin_name {
  GPIO_TypeDef* port;
  uint16_t pin;
} pin_name;

typedef struct adc_pin_name {
  pin_name pin;
  ADC_TypeDef* adc;
  uint32_t channel;
} adc_pin_name;

#define PIN3 (pin_name) {GPIOA, GPIO_PIN_8}
#define PIN4 (pin_name) {GPIOB, GPIO_PIN_15}
#define PIN5 (pin_name) {GPIOB, GPIO_PIN_14}
#define PIN6 (pin_name) {GPIOA, GPIO_PIN_12}
#define PIN7 (pin_name) {GPIOA, GPIO_PIN_11}
#define PIN8 (pin_name) {GPIOC, GPIO_PIN_6}
#define PIN9 (pin_name) {GPIOC, GPIO_PIN_7}
#define PIN10 (pin_name) {GPIOC, GPIO_PIN_8}
#define PIN11 (pin_name) {GPIOC, GPIO_PIN_9}
#define PIN12 (pin_name) {GPIOH, GPIO_PIN_10}
#define PIN13 (pin_name) {GPIOH, GPIO_PIN_11}
#define PIN14 (pin_name) {GPIOH, GPIO_PIN_12}
#define PIN15 (pin_name) {GPIOI, GPIO_PIN_0}
#define PIN16 (pin_name) {GPIOD, GPIO_PIN_5}
#define PIN17 (pin_name) {GPIOD, GPIO_PIN_6}
#define PIN18 (pin_name) {GPIOD, GPIO_PIN_7}
#define PIN19 (pin_name) {GPIOB, GPIO_PIN_9}
#define PIN20 (pin_name) {GPIOB, GPIO_PIN_8}
#define PIN25 (pin_name) {GPIOB, GPIO_PIN_6}
#define PIN26 (pin_name) {GPIOB, GPIO_PIN_7}
#define PIN27 (pin_name) {GPIOF, GPIO_PIN_9}
#define PIN28 (pin_name) {GPIOF, GPIO_PIN_8}
#define PIN29 (pin_name) {GPIOF, GPIO_PIN_7}
#define PIN30 (pin_name) {GPIOF, GPIO_PIN_6}
#define PIN31 (pin_name) {GPIOA, GPIO_PIN_0}
#define PIN32 (pin_name) {GPIOA, GPIO_PIN_6}
#define PIN33 (pin_name) {GPIOA, GPIO_PIN_4}
#define PIN34 (pin_name) {GPIOC, GPIO_PIN_10}
#define PIN35 (pin_name) {GPIOB, GPIO_PIN_4}
#define PIN36 (pin_name) {GPIOC, GPIO_PIN_12}

#define LED1 (pin_name) {GPIOE, GPIO_PIN_4}
#define LED2 (pin_name) {GPIOE, GPIO_PIN_3}
#define LED3 (pin_name) {GPIOE, GPIO_PIN_6}
#define LED4 (pin_name) {GPIOE, GPIO_PIN_5}

#define adc3 (adc_pin_name) {PIN30, ADC3, ADC_CHANNEL_4} // pin 30

typedef enum {
    PULL_NONE = GPIO_NOPULL,
    PULL_UP = GPIO_PULLUP,
    PULL_DOWN = GPIO_PULLDOWN
} pull_type;

#ifdef __cplusplus
}
#endif

#endif
