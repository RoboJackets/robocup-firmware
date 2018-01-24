#pragma once

#include <cstdarg>

/**
 * A macro for assertion checks.
 * @param e The variable to check.
 * @return Nothing is returned.
 */
/* clang-format off */
#ifndef NDEBUG
#   define ASSERT(e)  \
        if (!(e)) assertFail((#e), __FILE__, __LINE__)
#else
#   define ASSERT(x)
#endif
/* clang-format on */

/**
 * This is called when an assertion fails.
 */
void assertFail(const char* expr, const char* file, int line);
