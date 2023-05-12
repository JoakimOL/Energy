#include "logger.hpp"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

void setup_logger(const std::string& name) {
    size_t max_size = 1048576 * 5;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);

    // Not used
    // auto rotating_file_sink =
    //     std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
    //         "logs/multisink.txt", max_size, 2);
    // rotating_file_sink->set_level(spdlog::level::trace);

    spdlog::logger logger("logger", console_sink);
    spdlog::flush_on(spdlog::level::trace);
}
