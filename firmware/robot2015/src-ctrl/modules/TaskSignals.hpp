#pragma once

#include <cstdint>

// This is where the signals for different threads are defined for use across
// mutliple files

constexpr uint32_t MAIN_TASK_CONTINUE = 1 << 0;
constexpr uint32_t SUB_TASK_CONTINUE = 1 << 1;
