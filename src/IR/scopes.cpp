#include "scopes.hpp"

#include "spdlog/spdlog.h"
#include <optional>

void Scope::insertSymbol(const std::string& identifier, llvm::Value* value) {
    spdlog::debug("inserting {} into {}", identifier, name);
    localSymbolTable.insert({identifier, value});
}

std::optional<llvm::Value*> Scope::getSymbol(const std::string& identifier) {
    auto symbol = localSymbolTable.find(identifier);
    if (symbol == localSymbolTable.end()) {
        spdlog::warn("{}::could not find {}, returning nullopt", __PRETTY_FUNCTION__, identifier);
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
    spdlog::warn("{}::could not find {}, returning nullopt", __PRETTY_FUNCTION__, identifier);
    return std::nullopt;
}

bool ScopeManager::add_new_user_defined_type(const std::string& type_name, const std::vector<std::string> &parameter_names, const std::vector<llvm::Type*> types){
    // type name collision, return false to signal failure
    if(user_defined_types.find(type_name) != user_defined_types.end())
        return false;

    // type name is original, we can proceed
    auto success = user_defined_types.emplace(type_name, parameter_names);
    return success.second;
    
}


std::optional<std::vector<std::string>> ScopeManager::get_user_defined_type(const std::string& type_name){
  auto search = user_defined_types.find(type_name);
  if(search == user_defined_types.end())
    return std::nullopt;
  return search->second;
}

