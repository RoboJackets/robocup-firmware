#pragma once

#include <cstdint>

// All modules must implement this interface
// It allows the scheduler to call the module to run it
class GenericModule {
public:
    // Double check these values against the super loop timings
    // Anything fast will not be executed at that rate
    // Anything with a longer runtime than the loop will never run

    // How many times per second this module should run
    static constexpr float freq = 1.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000000L / freq);

    // How long a single call to this module takes
    // Take the safe estimate since any time over may
    // cause the loop to run long
    // Can be 0us if no long IO is done
    //
    // Note: Even if this is less than the super loop time,
    // it may not run due to the priority of the module
    static constexpr uint32_t runtime = 10; // us

    // Called at most once a frame to execute the module
    virtual void entry(void) = 0;
};