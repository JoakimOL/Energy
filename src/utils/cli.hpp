#ifndef CLI_HPP
#define CLI_HPP

#include <string>
#include <optional>
#include <vector>

class ArgParser {
   public:
    ArgParser(int argc, char** argv);
    bool hasArg(const std::string& arg) const;
    std::optional<std::string> getArgValue(const std::string& arg) const;

   private:
    std::string executableName;
    std::vector<std::string> args;
};

#endif // CLI_HPP
