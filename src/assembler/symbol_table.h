#pragma once

#include "common/types.h"
#include <map>
#include <string>

namespace z80 {

class SymbolTable {
public:
    SymbolTable();
    
    void addSymbol(const std::string& name, const Symbol& symbol);
    bool hasSymbol(const std::string& name) const;
    Symbol* getSymbol(const std::string& name);
    const Symbol* getSymbol(const std::string& name) const;
    
private:
    std::map<std::string, Symbol> symbols_;
};

} // namespace z80
