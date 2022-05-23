#pragma once

#include <chrono>
#include <cstdint>

#include "FreeRTOS.h"
#include "common.hpp"
#include "task.h"

/** @class GenericModule
 *  Interface for all modules, so that they can be run by the scheduler
 */
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
     * Code to run when called by RTOS once per system tick (`kperiod`)
     */
    virtual void entry() = 0;

    /**
     * The module period (milliseconds)
     */
    std::chrono::milliseconds period;

    /**
     * A human-readable name for the process
     */
    const char *name;

    /**
    * The priority of the module (default: low)
    */
    int priority = 1;

    int stackSize = 1024;

    TaskHandle_t handle = nullptr;
};
