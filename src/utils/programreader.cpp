#include "programreader.hpp"

#include <fstream>
#include <sstream>

#include "spdlog/spdlog.h"

void ProgramReader::readFromFile(const std::string& filename) {
    std::ifstream file{filename};
    if (!file.is_open()) {
        spdlog::info("File opening failed! Does it exist?");
        return;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    program = ss.str();
}
void ProgramReader::readFromStdin() {
    spdlog::info("ay lmao");
    return;
}
std::optional<std::string_view> ProgramReader::getProgram() const {
    return program;
}
