#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

#include "spdlog/spdlog.h"

void setup_logger(const std::string& name = "basic_logger");

#endif  // LOGGER_HPP
