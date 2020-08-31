#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

struct Logger {
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
};

void initLogger();

// Core logging macros

#define TRACE(...) Logger::s_CoreLogger->trace    (__VA_ARGS__)
#define INFO(...)  Logger::s_CoreLogger->info     (__VA_ARGS__)
#define WARN(...)  Logger::s_CoreLogger->warn     (__VA_ARGS__)
#define ERROR(...) Logger::s_CoreLogger->error    (__VA_ARGS__)
#define FATAL(...) Logger::s_CoreLogger->fatal    (__VA_ARGS__)

#endif