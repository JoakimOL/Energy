#include "astvisitor.hpp"

llvm::Type *AstVisitor::map_type_to_llvm_type(const std::string &type) {
    if (type == "i8") {
        std::cout << "returning int type" << std::endl;
        return llvm::Type::getInt32Ty(*ctx);
    }
    else if( type == "string") {
        std::cout << "returning string type (i8 array)" << std::endl;
        return llvm::Type::getInt8PtrTy(*ctx);
    }
    std::cout << "couldn't find type! returning" << std::endl;
    exit(1);
}

void AstVisitor::compile(energy::EnergyParser::ProgramContext *program) {
    // energy::EnergyParserVisitor visitor;
    // visitor.visitProgram(program);

    this->visitProgram(program);

    saveModuleToFile("./out.ll");
}

void AstVisitor::saveModuleToFile(const std::string &filename) {
    module->print(llvm::outs(), nullptr);
    std::error_code errorCode;
    llvm::raw_fd_ostream outLL(filename, errorCode);
    module->print(outLL, nullptr);
}

////////////////////////////////
//
// Visitor implementation below
//
////////////////////////////////

/**
 * Program is the main entry point of the grammar
 * It consists of one of more statements, so this visitor
 * will have to recursively visit the statements.
 *
 * program: statement+ EOF;
 *
 * ~Since this is the entry point, the visitor will create
 * a main function.~
 *
 * ^ this is dumb, so lets not
 */
void AstVisitor::visitProgram(energy::EnergyParser::ProgramContext *program) {
    // auto functionType =
    //     llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx), false);
    // auto function = llvm::Function::Create(
    //     functionType, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
    //     "main", this->module.get());
    // auto block = llvm::BasicBlock::Create(builder->getContext());
    // block->insertInto(function);
    // builder->SetInsertPoint(block);

    for (const auto &statement : program->statement())
        visitStatement(statement);

    // builder->CreateRet(
    //     llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 0, true));
    return;
}

/**
 * A  statement can be a bunch of things.
 *
 * statement: functionDeclaration
 *          | functionDefinition
 *          | functionCall SEMICOLON
 *          | variableDeclaration SEMICOLON;
 *
 */
void AstVisitor::visitStatement(
    energy::EnergyParser::StatementContext *context) {
    // std::cout << context->getText() << std::endl;
    if (auto funcDec = context->functionDeclaration()) {
        std::cout << "found function declaration" << std::endl;
        visitFunctionDeclaration(funcDec);
    } else if (auto funcDef = context->functionDefinition()) {
        std::cout << "found function definition" << std::endl;
        visitFunctionDefinition(funcDef);
    } else if (auto funcCall = context->functionCall()) {
        std::cout << "found function call" << std::endl;
        visitFunctionCall(funcCall);
    } else if (auto varDec = context->variableDeclaration()) {
        std::cout << "found variableDeclaration" << std::endl;
        visitVariableDeclaration(varDec);
    } else if (auto returnStat = context->returnStatement()) {
        std::cout << "found return statement" << std::endl;
        visitReturnStatement(returnStat);
    } else {
        std::cout << "i dunno what this is lol bai" << std::endl;
        return;
    }
    return;
}

/**
 * A function declaration is just a type signature
 * functionDeclaration: id parameterList '->' TYPENAME;
 */
void AstVisitor::visitFunctionDeclaration(
    energy::EnergyParser::FunctionDeclarationContext *context) {
    std::cout << context->getText() << std::endl;

    auto type = map_type_to_llvm_type(context->TYPENAME()->getText());
    auto name = context->id()->getText();

    auto functionType = llvm::FunctionType::get(type, false);
    auto function = llvm::Function::Create(
        functionType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name,
        this->module.get());

    globalScope().insertSymbol(name, function);
    std::cout << "number of symbols after " << name << ": "
              << globalScope().size() << std::endl;
}

/**
 * functionDefinition: id parameterList '=' block;
 */
void AstVisitor::visitFunctionDefinition(
    energy::EnergyParser::FunctionDefinitionContext *context) {
    std::cout << context->getText() << std::endl;

    auto basicBlock = llvm::BasicBlock::Create(builder->getContext());
    auto name = context->id()->getText();
    auto function =
        static_cast<llvm::Function *>(globalScope().getSymbol(name));

    basicBlock->insertInto(function);
    builder->SetInsertPoint(basicBlock);
    scopes.emplace_back(Scope(name));
    visitBlock(context->block());

    // builder->CreateRet(
    // llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 0, true));
}

void AstVisitor::visitBlock(energy::EnergyParser::BlockContext *context) {
    std::cout << context->getText() << std::endl;
    for (const auto &statement : context->statement())
        visitStatement(statement);
}

/**
 * returnStatement: RETURNKEYWORD expression;
 */
void AstVisitor::visitReturnStatement(
    energy::EnergyParser::ReturnStatementContext *context) {
    std::cout << context->getText() << std::endl;
    auto value = visitExpression(context->expression());
    builder->CreateRet(value);
}

/**
 * variableDeclaration: TYPENAME id '=' value;
 */
void AstVisitor::visitVariableDeclaration(
    energy::EnergyParser::VariableDeclarationContext *context) {
    // builder->CreateStore(
    auto name = context->id()->getText();
    auto value = visitExpression(context->expression());
    std::cout << "name: " << name;
    auto allocation =
        builder->CreateAlloca(llvm::Type::getInt32Ty(*ctx), nullptr, name);
    // builder->createStore(
    return;
}

void AstVisitor::visitParameterList(
    energy::EnergyParser::ParameterListContext *context) {
    std::cout << context->getText() << std::endl;
}

void AstVisitor::visitFunctionCall(
    energy::EnergyParser::FunctionCallContext *context) {
    std::cout << context->getText() << std::endl;
}

/**
 * expression: id
 *           | literal
 *           | '(' expression ')';
 */
llvm::Value *AstVisitor::visitExpression(
    energy::EnergyParser::ExpressionContext *context) {
    // std::cout << context->getText() << std::endl;
    if (auto id = context->id()) {
        std::cout << "found an identifier" << std::endl;
    } else if (auto literal = context->literal()) {
        std::cout << "found a literal" << std::endl;
        return visitLiteral(literal);
    } else if (auto expression = context->expression()) {
        std::cout << "found an expression" << std::endl;
        return visitExpression(expression);
    }
    return nullptr;
}

llvm::Value *AstVisitor::visitLiteral(
    energy::EnergyParser::LiteralContext *context) {
    std::cout << context->getText() << std::endl;
    if (auto INT = context->INT()) {
        int value;
        llvm::StringRef(INT->getText()).getAsInteger(/*radix=*/10, value);
        std::cout << "found an int literal: " << value << std::endl;
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), value);
    } else if (auto STRING = context->STRINGLITERAL()) {
        auto value = STRING->getText();
        return builder->CreateGlobalStringPtr(value);
    }
    return nullptr;
}
