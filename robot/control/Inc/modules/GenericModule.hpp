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
    virtual void start(void) {}

    // Called at most once a frame to execute the module
    virtual void entry(void) = 0;

    // The period of this module in milliseconds
    std::chrono::milliseconds period;

    // A human-readable name for the process
    const char *name;

    // The priority of this module (default: low)
    int priority = 1;

    int stackSize = 1024;

    TaskHandle_t handle;
};