#ifndef LOG_H
#define LOG_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void initLogger() {
    spdlog::set_pattern("%^[%T] %n: %v%$");
    spdlog::stdout_color_mt("PONG");
    spdlog::set_level(spdlog::level::info);
}

// Core logging macros

#define TRACE(...) spdlog::trace    (__VA_ARGS__)
#define INFO(...)  spdlog::info     (__VA_ARGS__)
#define WARN(...)  spdlog::warn     (__VA_ARGS__)
#define ERROR(...) spdlog::error    (__VA_ARGS__)
#define FATAL(...) spdlog::fatal    (__VA_ARGS__)

#endif