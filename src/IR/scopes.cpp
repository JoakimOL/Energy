#include "scopes.hpp"
#include <iostream>

void Scope::insertSymbol(const std::string& identifier, llvm::Value* value){
    localSymbolTable.insert({identifier, value});
}

llvm::Value* Scope::getSymbol(const std::string& identifier){
    auto symbol = localSymbolTable.find(identifier);
    if(symbol == localSymbolTable.end())
    {
        std::cout << "returning nullptr" << std::endl;
        return nullptr;
    }
    return symbol->second;
}
