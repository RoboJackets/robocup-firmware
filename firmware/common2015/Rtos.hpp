#pragma once
/* clang-format off */

#if defined(__MBED_TEST__)
    #include "FakeRtos.hpp"
    namespace rtos = RtosTest;
    using namespace rtos;
#else
    #include "rtos.h"
#endif  // __MBED_TEST__

/* clang-format on */
