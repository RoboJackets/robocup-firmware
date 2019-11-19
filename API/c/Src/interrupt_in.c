#include "interrupt_in.h"

static void (*interrupt0)() = NULL;
static void (*interrupt4)() = NULL;
static void (*interrupt5)() = NULL;
static void (*interrupt6)() = NULL;
static void (*interrupt7)() = NULL;
static void (*interrupt8)() = NULL;
static void (*interrupt9)() = NULL;
static void (*interrupt10)() = NULL;
static void (*interrupt11)() = NULL;
static void (*interrupt12)() = NULL;
static void (*interrupt14)() = NULL;
static void (*interrupt15)() = NULL;

static int interrupt_priority = 10;

static void NVIC_config(IRQn_Type IRQn) {
    HAL_NVIC_SetPriority(IRQn, interrupt_priority, interrupt_priority);
    HAL_NVIC_EnableIRQ(IRQn);
}

 static void interrupt_config(uint16_t pin, void (*function)()) {
    if (pin == GPIO_PIN_0) {
        interrupt0 = function;
        NVIC_config(EXTI0_IRQn);
    } else if (pin == GPIO_PIN_4) {
        interrupt4 = function;
        NVIC_config(EXTI4_IRQn);
    } else if (pin == GPIO_PIN_5) {
        interrupt5 = function;
        NVIC_config(EXTI9_5_IRQn);
    } else if (pin == GPIO_PIN_6) {
        interrupt6 = function;
        NVIC_config(EXTI9_5_IRQn);
    } else if (pin == GPIO_PIN_7) {
        interrupt7 = function;
        NVIC_config(EXTI9_5_IRQn);
    } else if (pin == GPIO_PIN_8) {
        interrupt8 = function;
        NVIC_config(EXTI9_5_IRQn);
    } else if (pin == GPIO_PIN_9) {
        interrupt9 = function;
        NVIC_config(EXTI9_5_IRQn);
    } else if (pin == GPIO_PIN_10) {
        interrupt10 = function;
        NVIC_config(EXTI15_10_IRQn);
    } else if (pin == GPIO_PIN_11) {
        interrupt11 = function;
        NVIC_config(EXTI15_10_IRQn);
    } else if (pin == GPIO_PIN_12) {
        interrupt12 = function;
        NVIC_config(EXTI15_10_IRQn);
    } else if (pin == GPIO_PIN_14) {
        interrupt14 = function;
        NVIC_config(EXTI15_10_IRQn);
    } else if (pin == GPIO_PIN_15) {
        interrupt15 = function;
        NVIC_config(EXTI15_10_IRQn);
    }
}

 void EXTI0_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) { // Handles A0 (31)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
      (interrupt0)();
    }
}

 void EXTI4_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET) { // Handles B4 (35)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
      (interrupt4)();
    }
}

 void EXTI9_5_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET) { // Handles D5 (16)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
      (interrupt5)();
    } else if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) != RESET) { // Handles F6 (30)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
      (interrupt6)();
    } else if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_7) != RESET) { // Handles D7 (18)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_7);
      (interrupt7)();
    } else if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_8) != RESET) { // Handles D8 (20)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
      (interrupt8)();
    } else if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_9) != RESET) { // Handles C9 (11)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);
      (interrupt9)();
    }
}

 void EXTI15_10_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET) { // Handle C10 (34)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);
      (interrupt10)();
    } else if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11) != RESET) { // Handle H11 (13)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
      (interrupt11)();
    } else if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET) { // Handle C12 (36)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
      (interrupt12)();
    } else if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14) != RESET) { // Handle B14 (5)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);
      (interrupt14)();
    } else if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET) { // Handle B15 (4)
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);
      (interrupt15)();
    }
}

 void interruptin_init(pin_name pin, void (*function)()) {
    GPIO_InitTypeDef pin_structure = {};
    pin_structure.Pin = pin.pin;
    pin_structure.Mode = GPIO_MODE_IT_RISING;
    pin_structure.Pull = GPIO_NOPULL;

     HAL_GPIO_Init(pin.port, &pin_structure);
    interrupt_config(pin.pin, function);
}

 void interruptin_init_ex(pin_name pin, void (*function)(), pull_type pull,
    interrupt_mode mode) {
    GPIO_InitTypeDef pin_structure = {};
    pin_structure.Pin = pin.pin;
    pin_structure.Pull = pull;
    pin_structure.Mode = mode;

     HAL_GPIO_Init(pin.port, &pin_structure);

     interrupt_config(pin.pin, function);
}

 void interruptin_deinit(pin_name pin) {
    HAL_GPIO_DeInit(pin.port, pin.pin);
}

int interruptin_read(pin_name pin) {
    return HAL_GPIO_ReadPin(pin.port, pin.pin);
}