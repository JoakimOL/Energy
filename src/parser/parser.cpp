#include "parser.hpp"

#include "spdlog/spdlog.h"

ParserWrapper::ParserWrapper(std::string_view program)
    : lexer(&input), tokens(&lexer), parser(&tokens) {
    input = antlr4::ANTLRInputStream(program.data());
}

bool ParserWrapper::validate_lex() {
    auto num_errors = lexer.getNumberOfSyntaxErrors();
    spdlog::info("lexer errors: {}", num_errors);
    if (num_errors) {
        spdlog::warn("Lexer error! Ignoring and attempting to go on");
        return false;
    }
    return true;
}

MaybeAST ParserWrapper::parse() {
    energy::EnergyParser::ProgramContext* res = parser.program();
    auto num_errors = parser.getNumberOfSyntaxErrors();
    spdlog::info("parser errors: {}", num_errors);
    if (num_errors) {
        spdlog::warn("Syntax error! Go fuck yourself!");
        ast = std::nullopt;
    } else
        ast = res;
    return ast;
}

void ParserWrapper::printAst() {
    if (ast.has_value()) spdlog::info((*ast)->toStringTree(&parser));
    return;
}
