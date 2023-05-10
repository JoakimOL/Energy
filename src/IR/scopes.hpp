#ifndef SCOPES_H
#define SCOPES_H
#include <map>
#include <string>

#include "llvm/IR/Value.h"

class Scope {
   public:
    Scope(const std::string& name) : name(name){};
    void insertSymbol(const std::string& identifier, llvm::Value* value);
    llvm::Value* getSymbol(const std::string& identifier);
    size_t size() const { return localSymbolTable.size(); };

   private:
    const std::string name;
    std::map<std::string, llvm::Value*> localSymbolTable;
};

#endif  // SCOPES_H
