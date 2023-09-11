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
        : ctx(std::make_unique<llvm::LLVMContext>()),
          module(std::make_unique<llvm::Module>("EnergyLLVM", *ctx)),
          builder(std::make_unique<llvm::IRBuilder<>>(*ctx)) {
              auto bytePtrTy = builder->getInt8Ty()->getPointerTo();
              module->getOrInsertFunction("printf",
                      llvm::FunctionType::get(builder->getInt32Ty(), bytePtrTy, true));
              auto printf = module->getFunction("printf");
              scopeManager_.globalScope().insertSymbol("printf", printf);
          }

    void compile(energy::EnergyParser::ProgramContext *program, const std::string & outfile="./out.ll");
    llvm::Function *currentFunction;  // need function in order to correctly
                                      // place basic blocks

   private:
    ScopeManager scopeManager_;
    void saveModuleToFile(const std::string &filename);

    void visitProgram(energy::EnergyParser::ProgramContext *context);

    void visitToplevel(energy::EnergyParser::ToplevelContext *context);

    void visitTypeDefinition(energy::EnergyParser::TypeDefinitionContext *context);

    void visitStatement(energy::EnergyParser::StatementContext *context);

    void visitFunctionDeclaration(
        energy::EnergyParser::FunctionDeclarationContext *context);

    void visitFunctionDefinition(
        energy::EnergyParser::FunctionDefinitionContext *context);

    llvm::Value *visitFunctionCall(
        energy::EnergyParser::FunctionCallContext *context);

    void visitVariableDeclaration(
        energy::EnergyParser::VariableDeclarationContext *context);

    void visitArgumentList(energy::EnergyParser::ArgumentListContext *context);

    std::vector<llvm::Type *> visitParameterList(
        energy::EnergyParser::ParameterListContext *context);

    void visitBlock(energy::EnergyParser::BlockContext *context,
                    Scope scope);

    void visitBlock(energy::EnergyParser::BlockContext *context,
                    const std::string& name="");

    void visitTypedValue(energy::EnergyParser::TypedValueContext *context);

    void visitReturnStatement(
        energy::EnergyParser::ReturnStatementContext *context);

    llvm::Value *visitLiteral(energy::EnergyParser::LiteralContext *context);

    llvm::Value *visitExpression(
        energy::EnergyParser::ExpressionContext *context);

    void visitIfStatement(energy::EnergyParser::IfStatementContext *context);

    std::unique_ptr<llvm::LLVMContext> ctx;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    llvm::Type *map_type_to_llvm_type(const std::string &type);
};

#endif  // ASTVISITOR_H
