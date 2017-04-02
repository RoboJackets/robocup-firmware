#pragma once

#include <cstdarg>
#include <sstream>
#include <string>

#include "MacroHelpers.hpp"

// Macros for logging the filename and line for every call to LOG(...)
// disabled for Release builds
/* clang-format off */
#ifndef NDEBUG
    // Gets curent file name without path
    // see http://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#   ifdef __FILE_NAME__
#       define __BASE_FILE_NAME__                                                           \
            (strrchr(__FILE_NAME__, '/') ? strrchr(__FILE_NAME__, '/') + 1 : __FILE_NAME__)
#   else
#       define __BASE_FILE_NAME__                                                           \
            (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#   endif

#   define LOG(logLevel, ...)                                                               \
        do {                                                                                \
            globalLogger(logLevel, __BASE_FILE_NAME__, __LINE__, __func__, __VA_ARGS__);    \
        } while (false)

    /**
     * Example usage:
     *   S_LOG(INFO) << "Example";
     */
#   define S_LOG(logLevel)                                                                  \
        do {                                                                                \
            LogHelper(logLevel, __BASE_FILE_NAME__, __LINE__, __func__);                    \
        } while (false)

#else
#   define LOG(...)
#   define S_LOG(...) ((void)0)
#endif
/* clang-format on */

#define FOREACH_LEVEL(LEVEL) \
    LEVEL(LOG_LEVEL_START)   \
    LEVEL(SEVERE)            \
    LEVEL(WARN)              \
    LEVEL(OK)                \
    LEVEL(INFO)              \
    LEVEL(DEBUG)             \
    LEVEL(LOG_LEVEL_END)

/// The available log levels
enum LOG_LEVEL { FOREACH_LEVEL(GENERATE_ENUM) };

/// String mappings for the enum log levels
extern const char* const LOG_LEVEL_STRING[];

/// Enable/disable logging globally
extern bool isLogging;

/// The runtime log level
extern uint8_t rjLogLevel;

/**
 * Collects the stream log message into a single string to print
 *
 * @param logLevel The "importance level" of the called log message.
 * @param source   The source of the message.
 * @param format   The string format for displaying the log message.
 */
class LogHelper : public std::stringstream {
public:
    LogHelper(uint8_t logLevel, const char* source, int line, const char* func);
    ~LogHelper();

private:
    const char* m_source;
    const char* m_func;
    int m_line;
    uint8_t m_logLevel;
};

/**
 * The system-wide logging interface function. All log messages go through
 * this.
 *
 * @param logLevel The "importance level" of the called log message.
 * @param source   The source of the message.
 * @param format   The string format for displaying the log message.
 */
void globalLogger(uint8_t logLevel, const char* source, int line,
                  const char* func, const char* format, ...);

int logLvlChange(const std::string& s);
