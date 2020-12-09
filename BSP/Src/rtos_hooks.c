#include "FreeRTOS.h"
#include "task.h"

/**
 * RTOS Hook for the idle task's stack
 * @param ppxIdleTaskTCBBuffer
 * @param ppxIdleTaskStackBuffer
 * @param pulIdleTaskStackSize
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
    static StaticTask_t idleTaskTCB;
    static StackType_t idleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &idleTaskTCB;
    *ppxIdleTaskStackBuffer = idleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/**
 * RTOS Hook for the timer's stack
 * @param ppxIdleTaskTCBBuffer
 * @param ppxIdleTaskStackBuffer
 * @param pulIdleTaskStackSize
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    static StaticTask_t timerTaskTCB;
    static StackType_t timerTaskStack[configMINIMAL_STACK_SIZE];

    *ppxTimerTaskTCBBuffer = &timerTaskTCB;
    *ppxTimerTaskStackBuffer = timerTaskStack;
    *pulTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}

