#include "astvisitor.hpp"

#include <algorithm>

#include "spdlog/spdlog.h"

llvm::Type *AstVisitor::map_type_to_llvm_type(const std::string &type) {
    if (type == "i8") {
        spdlog::debug("returning int type");
        return llvm::Type::getInt32Ty(*ctx);
    } else if (type == "string") {
        spdlog::debug("returning string type (i8 array)");
        return llvm::Type::getInt8PtrTy(*ctx);
    }
    spdlog::debug("couldn't find type! returning");
    exit(1);
}

void AstVisitor::compile(energy::EnergyParser::ProgramContext *program) {
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
    for (const auto &statement : program->statement())
        visitStatement(statement);

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
    if (auto funcDec = context->functionDeclaration()) {
        spdlog::info("found function declaration");
        visitFunctionDeclaration(funcDec);
    } else if (auto funcDef = context->functionDefinition()) {
        spdlog::info("found function definition");
        visitFunctionDefinition(funcDef);
    } else if (auto funcCall = context->functionCall()) {
        spdlog::info("found function call");
        visitFunctionCall(funcCall);
    } else if (auto varDec = context->variableDeclaration()) {
        spdlog::info("found variableDeclaration");
        visitVariableDeclaration(varDec);
    } else if (auto returnStat = context->returnStatement()) {
        spdlog::info("found return statement");
        visitReturnStatement(returnStat);
    } else {
        spdlog::info("i dunno what this is lol bai");
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
    spdlog::debug(context->getText());

    auto returnType = map_type_to_llvm_type(context->TYPENAME()->getText());
    auto name = context->id()->getText();

    std::vector<llvm::Type *> paramTypes =
        visitParameterList(context->parameterList());

    auto functionType = llvm::FunctionType::get(returnType, paramTypes, false);
    auto function = llvm::Function::Create(
        functionType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name,
        this->module.get());

    scopeManager_.globalScope().insertSymbol(name, function);
}

/**
 * functionDefinition: id parameterList '=' block;
 */
void AstVisitor::visitFunctionDefinition(
    energy::EnergyParser::FunctionDefinitionContext *context) {
    spdlog::debug(context->getText());

    auto basicBlock = llvm::BasicBlock::Create(builder->getContext());
    auto name = context->id()->getText();
    auto function = static_cast<llvm::Function *>(
        scopeManager_.globalScope().getSymbol(name).value_or(nullptr));

    basicBlock->insertInto(function);
    builder->SetInsertPoint(basicBlock);
    scopeManager_.pushScope(name);
    visitBlock(context->block());

    // builder->CreateRet(
    // llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 0, true));
}

void AstVisitor::visitBlock(energy::EnergyParser::BlockContext *context) {
    spdlog::debug(context->getText());
    for (const auto &statement : context->statement())
        visitStatement(statement);
}

/**
 * returnStatement: RETURNKEYWORD expression;
 */
void AstVisitor::visitReturnStatement(
    energy::EnergyParser::ReturnStatementContext *context) {
    spdlog::debug(context->getText());
    auto value = visitExpression(context->expression());
    builder->CreateRet(value);
}

/**
 * variableDeclaration: TYPENAME id '=' value;
 */
void AstVisitor::visitVariableDeclaration(
    energy::EnergyParser::VariableDeclarationContext *context) {
    auto name = context->id()->getText();
    auto value = visitExpression(context->expression());
    auto allocation =
        builder->CreateAlloca(llvm::Type::getInt32Ty(*ctx), nullptr, name);
    scopeManager_.currentScope().insertSymbol(name, allocation);
    builder->CreateStore(/* value*/ value, /*allocated memory */ allocation);
    return;
}

/**
 *  parameterList: '(' ')'
 *               | '(' params+=typedValue(COMMA params+=typedValue)* ')';
 */
std::vector<llvm::Type *> AstVisitor::visitParameterList(
    energy::EnergyParser::ParameterListContext *context) {
    spdlog::debug(context->getText());
    std::vector<llvm::Type *> paramTypes;
    for (auto typedValue : context->params) {
        paramTypes.push_back(
            map_type_to_llvm_type(typedValue->TYPENAME()->getText()));
    }
    return paramTypes;
}

void AstVisitor::visitFunctionCall(
    energy::EnergyParser::FunctionCallContext *context) {
    spdlog::debug(context->getText());
}

/**
 * expression: id
 *           | literal
 *           | '(' expression ')';
 */
llvm::Value *AstVisitor::visitExpression(
    energy::EnergyParser::ExpressionContext *context) {
    if (auto id = context->id()) {
        spdlog::info("found an identifier expression");
        auto name = id->getText();
        auto *value = static_cast<llvm::AllocaInst *>(scopeManager_.getSymbol(name).value());
        auto loadinst = builder->CreateLoad(value->getAllocatedType(), value, name);
        return loadinst;
    } else if (auto literal = context->literal()) {
        spdlog::debug("found a literal");
        return visitLiteral(literal);
    } else if (auto expression = context->expression()) {
        spdlog::debug("found an expression");
        return visitExpression(expression);
    }
    return nullptr;
}

llvm::Value *AstVisitor::visitLiteral(
    energy::EnergyParser::LiteralContext *context) {
    // spdlog::debug(context->getText());
    if (auto INT = context->INT()) {
        int value;
        llvm::StringRef(INT->getText()).getAsInteger(/*radix=*/10, value);
        spdlog::debug("found an int literal: {}", value);
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), value);
    } else if (auto STRING = context->STRINGLITERAL()) {
        auto value = STRING->getText();
        return builder->CreateGlobalStringPtr(value);
    }
    spdlog::error("Couldn't find right literal type");
    return nullptr;
}
