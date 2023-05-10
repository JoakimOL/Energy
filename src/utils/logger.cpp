#include "logger.hpp"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

void setup_logger(const std::string& name) {
    size_t max_size = 1048576 * 5;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    // console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/multisink.txt", max_size, 2);
    rotating_file_sink->set_level(spdlog::level::debug);

    spdlog::logger logger("multi_sink", {console_sink});
    // spdlog::logger logger("multi_sink", {console_sink, rotating_file_sink});
    // logger.set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
}
