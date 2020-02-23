#pragma once

#include <cstdint>
#include <chrono>

#include "FreeRTOS.h"
#include "task.h"

// All modules must implement this interface
// It allows the scheduler to call the module to run it
class GenericModule {
public:
    GenericModule(std::chrono::milliseconds period, const char *name, int priority = 1, int stackSize = 1024)
        : period(period), name(name), priority(priority), stackSize(stackSize) {}


    /**
     * Called once to initialize the module. All initialization work should be
     * done here instead of in the constructor.
     */
    virtual void start(void) {}

    /**
     * Called by the RTOS at the desired update rate.
     */
    virtual void entry() = 0;

    // The period of this module in milliseconds
    std::chrono::milliseconds period;

    // A human-readable name for the process
    const char *name;

    // The priority of this module (default: low)
    int priority = 1;

    int stackSize = 1024;

    TaskHandle_t handle = nullptr;
};