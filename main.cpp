#include "TestLexer.h"
#include "TestParser.h"
#include "src/IR/astvisitor.hpp"
#include "src/parser/parser.hpp"
#include "src/utils/cli.hpp"
#include "src/utils/programreader.hpp"

int main(int argc, char** argv) {

    ArgParser argParser(argc, argv);

    ProgramReader programReader;
    if(argParser.hasArg("--stdin"))
        programReader.readFromStdin();
    else
        programReader.readFromFile(argParser.getArgValue("-f","../demo.en"));

    auto program = programReader.getProgram();
    if(!program.has_value()) {
        std::cout << "Could not read program. Exiting" << std::endl;
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
