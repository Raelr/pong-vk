#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

struct Logger {
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
};

void initLogger();

// Core logging macros

#ifdef DEBUG

    #define PONG_TRACE(...) Logger::s_CoreLogger->trace    (__VA_ARGS__)
    #define PONG_INFO(...)  Logger::s_CoreLogger->info     (__VA_ARGS__)
    #define PONG_WARN(...)  Logger::s_CoreLogger->warn     (__VA_ARGS__)
    #define PONG_ERROR(...) Logger::s_CoreLogger->error    (__VA_ARGS__)
    #define PONG_FATAL(...) Logger::s_CoreLogger->fatal    (__VA_ARGS__)

#elif defined(RELEASE)

    #define PONG_TRACE(...)
    #define PONG_INFO(...)
    #define PONG_WARN(...)
    #define PONG_ERROR(...)
    #define PONG_FATAL(...)

#endif

#endif // LOGGER_H