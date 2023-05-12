#include "scopes.hpp"

#include "spdlog/spdlog.h"

void Scope::insertSymbol(const std::string& identifier, llvm::Value* value) {
    spdlog::debug("inserting {} into {}", identifier, name);
    localSymbolTable.insert({identifier, value});
}

std::optional<llvm::Value*> Scope::getSymbol(const std::string& identifier) {
    auto symbol = localSymbolTable.find(identifier);
    if (symbol == localSymbolTable.end()) {
        spdlog::warn("could not find {}, returning nullopt", identifier);
        return std::nullopt;
    }
    if(!symbol->second)
        spdlog::warn("{} is nullptr", identifier);
    return symbol->second;
}

std::optional<llvm::Value*> ScopeManager::getSymbol(
    const std::string& identifier) {
    spdlog::debug("searching for {}",identifier);
    for (auto scope : scopes)
        if (auto symbol = scope.getSymbol(identifier); symbol.has_value())
        {
            if(!symbol)
                spdlog::warn("{} is nullptr", identifier);
            return symbol;
        }
    if (auto symbol = globalScope_.getSymbol(identifier); symbol.has_value())
    {
        if(!symbol)
            spdlog::warn("{} is nullptr", identifier);
        return symbol;
    }
    spdlog::warn("could not find {}, returning nullopt", identifier);
    return std::nullopt;
}

