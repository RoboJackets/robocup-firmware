#include "Logger.hpp"

#include "Mbed.hpp"
#include "Rtos.hpp"

#include <algorithm>
#include <cstdarg>

constexpr const char* LOG_LEVEL_STRING[] = {FOREACH_LEVEL(GENERATE_STRING)};
bool isLogging;
uint8_t rjLogLevel;

namespace {
Mutex m;
}

LogHelper::LogHelper(uint8_t logLevel, const char* source, int line,
                     const char* func)
    : m_source(source), m_func(func), m_line(line), m_logLevel(logLevel) {}

LogHelper::~LogHelper() {
    globalLogger(m_logLevel, m_source, m_line, m_func, "%s", str().c_str());
}

/**
 * The system-wide logging interface function. All log messages go through
 * this.
 * @param logLevel The "importance level" of the called log message.
 * @param source   The source of the message.
 * @param format   The string format for displaying the log message.
 */
void globalLogger(uint8_t logLevel, const char* source, int line,
                  const char* func, const char* format, ...) {
    const auto showLogLine = isLogging && logLevel <= rjLogLevel;
    if (showLogLine) {
        m.lock();

        va_list args;
        static char newFormat[300];
        static char timeBuf[25];
        auto sysTime = time(nullptr);
        strftime(timeBuf, 25, "%H:%M:%S", localtime(&sysTime));

        snprintf(newFormat, sizeof(newFormat),
                 "%s [%s] [%s:%d] <%s>\r\n  %s\r\n\r\n", timeBuf,
                 LOG_LEVEL_STRING[logLevel], source, line, func, format);

        va_start(args, format);

        fflush(stdout);
        vprintf(newFormat, args);
        fflush(stdout);

        va_end(args);
        m.unlock();
    }
}

int logLvlChange(const std::string& s) {
    return std::count(s.begin(), s.end(), '+') -
           std::count(s.begin(), s.end(), '-');
}
