#ifndef PROGRAMREADER_HPP
#define PROGRAMREADER_HPP
#include <string>
#include <optional>

class ProgramReader {
   public:
    void readFromFile(const std::string& filename);
    void readFromStdin();

    std::optional<std::string_view> getProgram() const;

   private:
    std::optional<std::string> program = std::nullopt;
};

#endif  // PROGRAMREADER_HPP
