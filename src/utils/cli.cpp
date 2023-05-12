#include "cli.hpp"

#include <algorithm>

#include "spdlog/spdlog.h"

ArgParser::ArgParser(int argc, char** argv) : executableName(argv[0]) {
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    for (auto arg : args) spdlog::info(arg);
}
bool ArgParser::hasArg(const std::string& arg) const {
    return std::find(args.cbegin(), args.cend(), arg) != args.cend();
}
std::optional<std::string> ArgParser::getArgValue(const std::string& arg) const {
    // find the thing
    auto it = std::find(args.cbegin(), args.cend(), arg);
    // if we're at the end, theres no value after it, return.
    if (it == args.cend()) return std::nullopt;
    // go further to get the value
    it++;
    // if we're at the end, theres no value
    if (it == args.cend()) return std::nullopt;
    // return the value
    return *it;
}
