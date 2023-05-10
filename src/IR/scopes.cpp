#include "scopes.hpp"

#include "spdlog/spdlog.h"

void Scope::insertSymbol(const std::string& identifier, llvm::Value* value) {
    localSymbolTable.insert({identifier, value});
}

std::optional<llvm::Value*> Scope::getSymbol(const std::string& identifier) {
    auto symbol = localSymbolTable.find(identifier);
    if (symbol == localSymbolTable.end()) {
        return std::nullopt;
    }
    return symbol->second;
}

std::optional<llvm::Value*> ScopeManager::getSymbol(
    const std::string& identifier) {
    for (auto scope : scopes)
        if (auto symbol = scope.getSymbol(identifier); symbol.has_value())
            return symbol;
    if (auto symbol = globalScope_.getSymbol(identifier); symbol.has_value())
        return symbol;
    return std::nullopt;
}

