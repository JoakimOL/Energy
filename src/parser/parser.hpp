#ifndef PARSER_HPP
#define PARSER_HPP

#include <optional>

#include "TestLexer.h"
#include "TestParser.h"

using MaybeAST = std::optional<antlrparser::TestParser::ProgramContext*>;

class ParserWrapper {
    /**
     * XXX: Note on memory management from ANTLR4:
     *
     * Memory Management
     * Since C++ has no built-in memory management we need to take extra care.
     * For that we rely mostly on smart pointers, which however might cause time
     * penalties or memory side effects (like cyclic references) if not used
     * with care. Currently however the memory household looks very stable.
     * Generally, when you see a raw pointer in code consider this as being
     * managed elsewhere.  You should never try to manage such a pointer
     * (delete, assign to smart pointer etc.).
     */
   public:
    ParserWrapper(const std::string& program);
    bool validate_lex();
    MaybeAST parse();
    void printAst();

    MaybeAST ast = std::nullopt;

   private:
    antlr4::ANTLRInputStream input;
    antlrparser::TestLexer lexer;
    antlr4::CommonTokenStream tokens;
    antlrparser::TestParser parser;
};

#endif  // PARSER_HPP
