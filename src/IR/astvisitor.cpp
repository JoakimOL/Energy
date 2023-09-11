#include "astvisitor.hpp"

#include <algorithm>
#include <llvm/ADT/ArrayRef.h>

#include "spdlog/spdlog.h"

llvm::Type *AstVisitor::map_type_to_llvm_type(const std::string &type) {
    if (type == "int") {
        spdlog::debug("returning int type");
        return llvm::Type::getInt32Ty(*ctx);
    } else if (type == "string") {
        spdlog::debug("returning string type (i8 array)");
        return llvm::Type::getInt8PtrTy(*ctx);
    }
    spdlog::debug("couldn't find type! returning");
    exit(1);
}

void AstVisitor::compile(energy::EnergyParser::ProgramContext *program, const std::string& outfile) {
    this->visitProgram(program);

    saveModuleToFile(outfile);
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
        spdlog::debug("found function declaration");
        visitFunctionDeclaration(funcDec);
    } else if (auto funcDef = context->functionDefinition()) {
        spdlog::debug("found function definition");
        visitFunctionDefinition(funcDef);
    } else if (auto typeDef = context->typeDefinition()) {
        spdlog::info("found type definition");
        visitTypeDefinition(typeDef);
    }
}


/**
 * typeDefinition: NEWTYPE id '=' parameterList;
 */
void AstVisitor::visitTypeDefinition(energy::EnergyParser::TypeDefinitionContext *context){
    auto typeName = context->id()->getText();
    spdlog::info("new type name: {}", typeName);

    std::vector<llvm::Type *> fields;
    for (auto typedValue : context->parameterList()->params) {
        fields.push_back(
            map_type_to_llvm_type(typedValue->type()->getText()));
    }
    llvm::StructType* newtype = llvm::StructType::create(module->getContext(), fields, typeName, false);

    spdlog::info("hopefully this works right? {}", llvm::StructType::getTypeByName(module->getContext(),typeName)->getName().data());
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
        spdlog::debug("found variableDeclaration");
        visitVariableDeclaration(varDec);
    } else if (auto returnStat = context->returnStatement()) {
        spdlog::debug("found return statement");
        visitReturnStatement(returnStat);
    } else if (auto block = context->block()) {
        spdlog::debug("found block");
        visitBlock(block);
    } else if (auto ifstat = context->ifStatement()) {
        spdlog::debug("found if statement");
        visitIfStatement(ifstat);
    } else {
        spdlog::debug("i dunno what this is lol bai");
        return;
    }
    return;
}

/**
 * A function declaration is just a type signature
 * functionDeclaration: id parameterList '->' type;
 */
void AstVisitor::visitFunctionDeclaration(
    energy::EnergyParser::FunctionDeclarationContext *context) {
    spdlog::debug(context->getText());

    auto returnType = map_type_to_llvm_type(context->type()->getText());
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
        if(it == nullptr)
            spdlog::warn("it is nullptr!");
        auto param = builder->CreateAlloca(llvm::Type::getInt32Ty(*ctx), nullptr, parametercontexts[i]->id()->getText());
        builder->CreateStore(it, param);
        scope.insertSymbol(parametercontexts[i]->id()->getText(), param);
        i++;
    }

    if (auto block = context->block())
        visitBlock(context->block(), scope);
    else
        visitStatement(context->statement());
}

void AstVisitor::visitBlock(energy::EnergyParser::BlockContext *context,
                            const std::string& name) {
    Scope scope(name);
    visitBlock(context, scope);
}

void AstVisitor::visitBlock(energy::EnergyParser::BlockContext *context,
                            Scope scope) {
    scopeManager_.pushScope(scope);

    spdlog::debug("entering block. depth: {}", scopeManager_.depth());
    // scopeManager_.pushScope(name);
    for (const auto &statement : context->statement())
        visitStatement(statement);
    scopeManager_.popScope();
    spdlog::debug("exiting block");
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
 * variableDeclaration: type id '=' value;
 */
void AstVisitor::visitVariableDeclaration(
    energy::EnergyParser::VariableDeclarationContext *context) {
    auto name = context->id()->getText();
    auto type_name = context->type()->getText();
    auto value = visitExpression(context->expression());

    // Lookup struct type in particular.
    auto type = llvm::StructType::getTypeByName(module->getContext(), context->type()->getText());
    llvm::AllocaInst* allocation;
    if(type==NULL){
        // Was not a struct, go for int
        // should expand to simple type/struct/list
        allocation =
            builder->CreateAlloca(llvm::Type::getInt32Ty(*ctx), nullptr, name);
    }
    else {
        // Was a struct, use it
        allocation =
            builder->CreateAlloca(type, nullptr, name);
   }
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
            map_type_to_llvm_type(typedValue->type()->getText()));
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
        spdlog::debug("found an identifier expression. name: {}", name);
        // scopeManager_.printAllIdentifiersInLocal();
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
        spdlog::debug("found function call");
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
