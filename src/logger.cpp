#include "logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;

void initLogger() {
    spdlog::set_pattern("%^[%T] %n: %v%$");
    Logger::s_CoreLogger = spdlog::stdout_color_mt("PONG");
    Logger::s_CoreLogger -> set_level(spdlog::level::trace);
}