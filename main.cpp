#include "src/IR/astvisitor.hpp"
#include "TestLexer.h"
#include "TestParser.h"
#include "src/parser/parser.hpp"

int main() {
    std::string program = R"(
main ( i8 argc, [string] argv ) -> i8
main(i8 argc, [string] argv) = {
    i8 foo = 42;
    print("%d\n", foo);
}
)";
    ParserWrapper parser(program);

    parser.validate_lex();
    MaybeAST ast = parser.parse();


    if(ast.has_value()) {
        parser.printAst();
        AstVisitor compiler;
        compiler.compile(parser.ast.value());
    }
}
