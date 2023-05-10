#include "scopes.hpp"

#include "spdlog/spdlog.h"

void Scope::insertSymbol(const std::string& identifier, llvm::Value* value) {
    localSymbolTable.insert({identifier, value});
}

llvm::Value* Scope::getSymbol(const std::string& identifier) {
    auto symbol = localSymbolTable.find(identifier);
    if (symbol == localSymbolTable.end()) {
        spdlog::warn("returning nullptr");
        return nullptr;
    }
    return symbol->second;
}
