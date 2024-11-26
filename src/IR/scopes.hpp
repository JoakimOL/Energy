#ifndef SCOPES_H
#define SCOPES_H
#include <deque>
#include <map>
#include <optional>
#include <string>

#include "spdlog/spdlog.h"
#include "llvm/IR/Value.h"

class Scope {
   public:
    Scope(const std::string& name = "") : name(name){};
    void insertSymbol(const std::string& identifier, llvm::Value* value);
    std::optional<llvm::Value*> getSymbol(const std::string& identifier);
    size_t size() const { return localSymbolTable.size(); };
    bool named() const {
        return !name.empty();
    }
    void printAllSymbols(){
        spdlog::info("printing all identifiers in {}", name);
        for (const auto& [key, value] : localSymbolTable){
            spdlog::info(key);
        }
    }

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
    Scope& pushScope(Scope scope) {
        spdlog::debug("size before pushing {}", scopes.size());
        scopes.push_front(scope);
        return currentScope();
    };
    void printAllIdentifiersInLocal() {
        currentScope().printAllSymbols();
    }
    void popScope() { scopes.pop_front(); }
    size_t depth() { return scopes.size(); }
    Scope& globalScope() { return globalScope_; }
    Scope& currentScope() { return scopes.front(); }
    // bool add_new_user_defined_type(const std::string& type_name, const std::vector<std::string>& parameter_names);
    bool add_new_user_defined_type(const std::string& type_name, const std::vector<std::string> &parameter_names, const std::vector<llvm::Type*> types);
    std::optional<std::vector<std::string>> get_user_defined_type(const std::string& type_name);

   private:
    std::deque<Scope> scopes;
    Scope globalScope_{"global"};
    std::map<std::string, std::vector<std::string>> user_defined_types; // this is global for now.
};

#endif  // SCOPES_H
