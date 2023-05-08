#ifndef ASTVISITOR_H
#define ASTVISITOR_H
#include <memory>
#include <string>

#include "TestParser.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class AstVisitor {
   public:
    AstVisitor()
        : ctx(std::make_unique<llvm::LLVMContext>()),
          module(std::make_unique<llvm::Module>("EnergyLLVM", *ctx)),
          builder(std::make_unique<llvm::IRBuilder<>>(*ctx)) {}

    void compile(antlrparser::TestParser::ProgramContext* program);

   private:
    void saveModuleToFile(const std::string& filename);

    void visitProg(antlrparser::TestParser::ProgramContext* program);

    std::unique_ptr<llvm::LLVMContext> ctx;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif  // ASTVISITOR_H
