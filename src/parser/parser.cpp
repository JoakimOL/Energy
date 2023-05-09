#include "parser.hpp"

#include <iostream>

ParserWrapper::ParserWrapper(std::string_view program)
    : lexer(&input), tokens(&lexer), parser(&tokens) {
    input = antlr4::ANTLRInputStream(program.data());
}

bool ParserWrapper::validate_lex() {
    auto num_errors = lexer.getNumberOfSyntaxErrors();
    std::cout << "lexer errors: " << num_errors << std::endl << std::flush;
    if (num_errors) {
        std::cerr << "Lexer error! Ignoring and attempting to go on"
                  << std::endl;
        return false;
    }
    return true;
}

MaybeAST ParserWrapper::parse() {
    energy::EnergyParser::ProgramContext* res = parser.program();
    auto num_errors = parser.getNumberOfSyntaxErrors();
    std::cout << "parser errors: " << num_errors << std::endl << std::flush;
    if (num_errors) {
        std::cerr << "Syntax error! Go fuck yourself!" << std::endl;
        ast = std::nullopt;
    }
    else
        ast = res;
    return ast;
}

void ParserWrapper::printAst() {
    if (ast.has_value())
        std::cout << (*ast)->toStringTree(&parser) << std::endl << std::flush;
    return;
}
