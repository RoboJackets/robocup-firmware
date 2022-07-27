#ifndef CONTROL_MACRO_HPP
#define CONTROL_MACRO_HPP
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "delay.h"
#include "mtrain.hpp"
#include "robocup.hpp"
#include "task.h"

/**
 * @def delay wrapper that considers whether or not the rtos is running
 * @param time time in FreeRTOS ticks to delay the method
 */
static inline void delay_from_tick(uint32_t time) {
#ifdef USING_RTOS
    vTaskDelay(time);
#else
    DWT_Delay(time * configTICK_RATE_HZ);
#endif
}

/**
 * @def delay wrapper that considers whether or not the rtos is running
 * @param time time in microseconds to delay the method
 */
static inline void delay_from_microseconds(uint32_t time) {
#ifdef USING_RTOS
    vTaskDelay(time / configTICK_RATE_HZ);
#else
    DWT_Delay(time);
#endif
}
#endif  // CONTROL_MACRO_HPP
