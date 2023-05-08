#include "programreader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

void ProgramReader::readFromFile(const std::string& filename) {
    std::ifstream file{filename};
    if(!file.is_open()) {
        std::cout << "File opening failed! Does it exist?" << std::endl;
        return;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    program = ss.str();
}
void ProgramReader::readFromStdin() {
    std::cout << "ay lmao" << std::endl;
    return;
}
std::optional<std::string_view> ProgramReader::getProgram() const { return program; }
