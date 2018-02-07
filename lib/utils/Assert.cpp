#include "Assert.hpp"

#include "Mbed.hpp"

/**
 * This is called when an assertion fails and does not return.
 *
 * @param exp  The expected value for the assertion check.
 * @param file The filename where the failure occured.
 * @param line The line number where the failure occured.
 */
[[noreturn]] void assertFail(const char* expr, const char* file, int line) {
    __disable_irq();
    error("\r\nAssertation failed: %s, file: %s, line %d \n", expr, file, line);
    while (true) {
    }
}
