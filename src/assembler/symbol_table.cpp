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

std::vector<const Symbol*> SymbolTable::getPublicSymbols() const {
    std::vector<const Symbol*> publicSymbols;
    for (const auto& pair : symbols_) {
        if (pair.second.isPublic && pair.second.defined) {
            publicSymbols.push_back(&pair.second);
        }
    }
    return publicSymbols;
}

std::vector<const Symbol*> SymbolTable::getExternalSymbols() const {
    std::vector<const Symbol*> externalSymbols;
    for (const auto& pair : symbols_) {
        if (pair.second.isExternal) {
            externalSymbols.push_back(&pair.second);
        }
    }
    return externalSymbols;
}

} // namespace z80
