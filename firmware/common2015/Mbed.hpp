#pragma once
/* clang-format off */

#if defined(__MBED_TEST__)
    #include "FakeMbed.hpp"
    namespace mbed = MbedTest;
    using namespace mbed;
    using namespace std;
#else
    #include "mbed.h"
#endif  // __MBED_TEST__

/* clang-format on */
