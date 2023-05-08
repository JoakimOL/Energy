#include "src/IR/astvisitor.hpp"
#include "TestLexer.h"
#include "TestParser.h"

int main() {
    std::string program = R"(
main ( i8 argc, [string] argv ) -> i8
main(i8 argc, [string] argv) = {
    i8 foo = 42;
    print("%d\n", foo);
}
)";
    // Lexical and semantic analysis
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

    antlr4::ANTLRInputStream input(program);
    antlrparser::TestLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    std::cout << "lexer errors: " << lexer.getNumberOfSyntaxErrors()
              << std::endl;
    tokens.fill();
    std::cout << tokens.getText() << std::endl;
    auto foo = tokens.getTokens();

    for (auto token : foo) {
        std::cout << token->getText() << std::endl;
    }

    antlrparser::TestParser parser(&tokens);

    antlrparser::TestParser::ProgramContext* ast = parser.program();
    std::cout << "parser errors: " << parser.getNumberOfSyntaxErrors()
              << std::endl
              << std::flush;
    if (parser.getNumberOfSyntaxErrors()) {
        std::cerr << "Syntax error! Go fuck yourself!" << std::endl;
        exit(1);
    }

    std::cout << ast->toStringTree(&parser) << std::endl;
    std::cout << std::flush;

    AstVisitor compiler;

    compiler.compile(ast);
}
