#ifndef SCOPES_H
#define SCOPES_H
#include <deque>
#include <map>
#include <optional>
#include <string>

#include "llvm/IR/Value.h"

class Scope {
   public:
    Scope(const std::string& name = "") : name(name){};
    void insertSymbol(const std::string& identifier, llvm::Value* value);
    std::optional<llvm::Value*> getSymbol(const std::string& identifier);
    size_t size() const { return localSymbolTable.size(); };

   private:
    const std::string name;
    std::map<std::string, llvm::Value*> localSymbolTable;
};

class ScopeManager {
   public:
    std::optional<llvm::Value*> getSymbol(const std::string& identifier);
    Scope& pushScope(const std::string& scopeName) {
        scopes.push_front(Scope(scopeName));
        return currentScope();
    };
    void popScope() { scopes.pop_front(); }
    size_t depth() { return scopes.size(); }
    Scope& globalScope() { return globalScope_; }
    Scope& currentScope() { return scopes.front(); }

   private:
    std::deque<Scope> scopes;
    Scope globalScope_{"global"};
};

#endif  // SCOPES_H
