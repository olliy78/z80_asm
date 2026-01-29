/**
 * @file symbol_table.cpp
 * @brief Implementation of symbol table management
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include "symbol_table.h"

namespace z80 {

SymbolTable::SymbolTable() {
}

void SymbolTable::addSymbol(const std::string& name, const Symbol& symbol) {
    symbols_[name] = symbol;
}

bool SymbolTable::hasSymbol(const std::string& name) const {
    return symbols_.find(name) != symbols_.end();
}

Symbol* SymbolTable::getSymbol(const std::string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return &it->second;
    }
    return nullptr;
}

const Symbol* SymbolTable::getSymbol(const std::string& name) const {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return &it->second;
    }
    return nullptr;
}

} // namespace z80
