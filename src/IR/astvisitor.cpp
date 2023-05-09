#include "astvisitor.hpp"

void AstVisitor::compile(energy::EnergyParser::ProgramContext* program) {
    this->visitProg(program);

    saveModuleToFile("./out.ll");
}

void AstVisitor::visitProg(energy::EnergyParser::ProgramContext* program) {
    auto functionType =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx), false);
    auto function = llvm::Function::Create(
        functionType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, "main",
        this->module.get());
    auto block = llvm::BasicBlock::Create(builder->getContext());
    block->insertInto(function);
    builder->SetInsertPoint(block);
    builder->CreateRet(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 42, true));
}

void AstVisitor::saveModuleToFile(const std::string& filename) {
    module->print(llvm::outs(), nullptr);
    std::error_code errorCode;
    llvm::raw_fd_ostream outLL(filename, errorCode);
    module->print(outLL, nullptr);
}
