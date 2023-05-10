#include "EnergyLexer.h"
#include "EnergyParser.h"
#include "src/IR/astvisitor.hpp"
#include "src/parser/parser.hpp"
#include "src/utils/cli.hpp"
#include "src/utils/logger.hpp"
#include "src/utils/programreader.hpp"

int main(int argc, char** argv) {
    ArgParser argParser(argc, argv);

    setup_logger();

    ProgramReader programReader;
    if (argParser.hasArg("--stdin"))
        programReader.readFromStdin();
    else
        programReader.readFromFile(argParser.getArgValue("-f", "../demo.en"));

    auto program = programReader.getProgram();
    if (!program.has_value()) {
        spdlog::info("Could not read program. Exiting");
        return 1;
    }
    ParserWrapper parser(*program);
    parser.validate_lex();
    MaybeAST ast = parser.parse();

    if (ast.has_value()) {
        parser.printAst();
        AstVisitor compiler;
        compiler.compile(parser.ast.value());
    }
}
