#ifndef ASTVISITOR_H
#define ASTVISITOR_H
#include <memory>
#include <string>

#include "EnergyParser.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "scopes.hpp"

class AstVisitor {
   public:
    AstVisitor()
        : ctx(std::make_unique<llvm::LLVMContext>())
        , module(std::make_unique<llvm::Module>("EnergyLLVM", *ctx))
        , builder(std::make_unique<llvm::IRBuilder<>>(*ctx))
        , scopes{Scope("global")}
        // , globalScope{scopes.front()}
        // , currentScope{scopes.front()}
    {}

    void compile(energy::EnergyParser::ProgramContext *program);
    // Scope& globalScope;
    // Scope& currentScope;
    Scope &globalScope() { return scopes.front(); }
    Scope &currentScope() { return scopes.back(); }

   private:
    void saveModuleToFile(const std::string &filename);

    // void visitProg(energy::EnergyParser::ProgramContext* program);

    void visitProgram(energy::EnergyParser::ProgramContext *context);

    void visitStatement(energy::EnergyParser::StatementContext *context);

    void visitFunctionDeclaration(
        energy::EnergyParser::FunctionDeclarationContext *context);

    void visitFunctionDefinition(
        energy::EnergyParser::FunctionDefinitionContext *context);

    void visitFunctionCall(energy::EnergyParser::FunctionCallContext *context);

    void visitVariableDeclaration(
        energy::EnergyParser::VariableDeclarationContext *context);

    void visitArgumentList(energy::EnergyParser::ArgumentListContext *context);

    void visitParameterList(
        energy::EnergyParser::ParameterListContext *context);

    void visitBlock(energy::EnergyParser::BlockContext *context);

    void visitTypedValue(energy::EnergyParser::TypedValueContext *context);

    void visitReturnStatement(energy::EnergyParser::ReturnStatementContext *context);

    // void visitId(energy::EnergyParser::IdContext *context);

    llvm::Value* visitLiteral(energy::EnergyParser::LiteralContext *context);

    llvm::Value *visitExpression(
        energy::EnergyParser::ExpressionContext *context);

    std::unique_ptr<llvm::LLVMContext> ctx;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    std::deque<Scope> scopes;

    llvm::Type *map_type_to_llvm_type(const std::string &type);
};

#endif  // ASTVISITOR_H
