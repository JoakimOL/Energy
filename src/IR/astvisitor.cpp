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
 * It consists of one of more top level statements, so this visitor
 * will have to recursively visit them.
 *
 * program: toplevel+ EOF;
 *
 */
void AstVisitor::visitProgram(energy::EnergyParser::ProgramContext *program) {
    for (const auto &statement : program->toplevel()) visitToplevel(statement);

    return;
}

void AstVisitor::visitToplevel(energy::EnergyParser::ToplevelContext *context) {
    if (auto funcDec = context->functionDeclaration()) {
        spdlog::info("found function declaration");
        visitFunctionDeclaration(funcDec);
    } else if (auto funcDef = context->functionDefinition()) {
        spdlog::info("found function definition");
        visitFunctionDefinition(funcDef);
    }
}

/**
 * A  statement can be a bunch of things.
 *
 * statement: functionCall SEMICOLON
 *          | variableDeclaration SEMICOLON
 *          | returnStatement SEMICOLON
 *          | block;
 *
 */
void AstVisitor::visitStatement(
    energy::EnergyParser::StatementContext *context) {
    if (auto varDec = context->variableDeclaration()) {
        spdlog::info("found variableDeclaration");
        visitVariableDeclaration(varDec);
    } else if (auto returnStat = context->returnStatement()) {
        spdlog::info("found return statement");
        visitReturnStatement(returnStat);
    } else if (auto block = context->block()) {
        spdlog::info("found block");
        visitBlock(block);
    } else if (auto ifstat = context->ifStatement()) {
        spdlog::info("found if statement");
        visitIfStatement(ifstat);
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
    spdlog::info(context->getText());

    auto basicBlock = llvm::BasicBlock::Create(builder->getContext());
    auto name = context->id()->getText();
    auto function = static_cast<llvm::Function *>(
        scopeManager_.globalScope().getSymbol(name).value_or(nullptr));
    currentFunction = function;

    basicBlock->insertInto(function);
    builder->SetInsertPoint(basicBlock);

    Scope scope(name);

    // XXX
    // what the actual shit is this.
    // I have less than 1 hour left of work btw
    auto parametercontexts = context->parameterList()->params;
    std::vector<std::pair<std::string, llvm::Value *>> parameters;
    int i = 0;
    for(auto it = function->arg_begin(); it != function->arg_end(); it++){
        // parameters.push_back(std::make_pair(parametercontexts[i]->id()->getText(), static_cast<llvm::Value*>(it)));
        if(it == nullptr)
            spdlog::warn("it is nullptr!");
        spdlog::warn(parametercontexts[i]->id()->getText());
        auto param = builder->CreateAlloca(llvm::Type::getInt32Ty(*ctx), nullptr, parametercontexts[i]->id()->getText());
        builder->CreateStore(it, param);
        scope.insertSymbol(parametercontexts[i]->id()->getText(), param);
        i++;
    }

    if (auto block = context->block())
        visitBlock(context->block(), scope);
    else
        visitStatement(context->statement());

    // builder->CreateRet(
    // llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 0, true));
}

void AstVisitor::visitBlock(energy::EnergyParser::BlockContext *context,
                            const std::string& name) {
    Scope scope(name);
    visitBlock(context, scope);
}

void AstVisitor::visitBlock(energy::EnergyParser::BlockContext *context,
                            Scope scope) {
    scopeManager_.pushScope(scope);

    spdlog::info("entering block. depth: {}", scopeManager_.depth());
    // scopeManager_.pushScope(name);
    for (const auto &statement : context->statement())
        visitStatement(statement);
    scopeManager_.popScope();
    spdlog::info("exiting block");
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
    builder->CreateStore(/* value */ value, /* allocated memory */ allocation);
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

/**
 * functionCall: id argumentList;
 */
llvm::Value *AstVisitor::visitFunctionCall(
    energy::EnergyParser::FunctionCallContext *context) {
    spdlog::debug(context->getText());
    auto name = context->id()->getText();
    auto function = static_cast<llvm::Function *>(
        scopeManager_.globalScope().getSymbol(name).value());
    auto inst = llvm::CallInst::Create(function->getFunctionType(), function);

    auto args = context->argumentList()->args;

    std::vector<llvm::Value* > llvmargs;
    for(auto arg: args){
        llvmargs.push_back(visitExpression(arg));
    }

    return builder->CreateCall(function, llvmargs);
}

/**
 * expression: id
 *           | literal
 *           | expression binop expression
 *           | '(' expression ')'
 *           | functionCall;
 */
llvm::Value *AstVisitor::visitExpression(
    energy::EnergyParser::ExpressionContext *context) {
    if (auto id = context->id()) {
        auto name = id->getText();
        spdlog::info("found an identifier expression. name: {}", name);
        scopeManager_.printAllIdentifiersInLocal();
        auto *value = static_cast<llvm::AllocaInst *>(
            scopeManager_.getSymbol(name).value_or(nullptr));
        if (!value) spdlog::error("identifier not found");
        auto loadinst =
            builder->CreateLoad(value->getAllocatedType(), value, name);
        return loadinst;
    } else if (auto literal = context->literal()) {
        spdlog::debug("found a literal");
        return visitLiteral(literal);
    } else if (auto binopexpression = context->binop()) {
        auto LHS = visitExpression(context->expression(0));
        auto RHS = visitExpression(context->expression(1));
        if (binopexpression->LESSTHAN()) {
            return builder->CreateICmpSLT(LHS, RHS);
        } else if (binopexpression->GREATERTHAN()) {
            return builder->CreateICmpSGE(LHS, RHS);
        } else if (binopexpression->EQUALS()) {
            return builder->CreateICmpEQ(LHS, RHS);
        } else if (binopexpression->PLUS()) {
            return builder->CreateAdd(LHS, RHS);
        } else if (binopexpression->MINUS()) {
            return builder->CreateSub(LHS, RHS);
        } else if (binopexpression->MUL()) {
            return builder->CreateMul(LHS, RHS);
        }
    } else if (auto expression = context->expression(0)) {
        spdlog::debug("found an expression");
        return visitExpression(expression);
    } else if (auto funcCall = context->functionCall()) {
        spdlog::info("found function call");
        return visitFunctionCall(funcCall);
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

/*
 * ifStatement: IFKEYWORD expression statement;
 */
void AstVisitor::visitIfStatement(
    energy::EnergyParser::IfStatementContext *context) {
    llvm::Value *expressionResult = visitExpression(context->expression());
    auto current_block = builder->GetInsertBlock();
    auto type = expressionResult->getType();

    auto trueBasicBlock = llvm::BasicBlock::Create(builder->getContext());
    auto afterBasicBlock = llvm::BasicBlock::Create(builder->getContext());

    trueBasicBlock->insertInto(currentFunction);
    afterBasicBlock->insertInto(currentFunction);
    builder->CreateCondBr(expressionResult, trueBasicBlock, afterBasicBlock);
    builder->SetInsertPoint(trueBasicBlock);
    visitStatement(context->statement());
    builder->SetInsertPoint(afterBasicBlock);
}
