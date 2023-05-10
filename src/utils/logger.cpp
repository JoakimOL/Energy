#include "logger.hpp"

#include "spdlog/sinks/rotating_file_sink.h"

void setup_logger(const std::string& name) {
    size_t max_size = 1048576 * 5;
    auto logger =
        spdlog::rotating_logger_mt(name, "logs/basic-log.txt", max_size, 1);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}
