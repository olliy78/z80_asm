/**
 * @file symbol_table.h
 * @brief Symbol table management
 * 
 * Manages labels, constants (EQU), variables (ASET), and macros.
 * Handles PUBLIC/EXTERNAL symbols for linking.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include "common/types.h"
#include <map>
#include <string>

namespace z80 {

/**
 * @class SymbolTable
 * @brief Manages symbols during assembly
 * 
 * The symbol table stores all labels, constants, and variables
 * defined in the assembly source. Supports:
 * - Label addresses
 * - EQU constants (immutable)
 * - ASET variables (mutable)
 * - PUBLIC symbols (exported)
 * - EXTERNAL symbols (imported)
 */
class SymbolTable {
public:
    /** @brief Construct an empty symbol table */
    SymbolTable();
    
    /**
     * @brief Add or update a symbol
     * @param name Symbol name
     * @param symbol Symbol data
     */
    void addSymbol(const std::string& name, const Symbol& symbol);
    
    /**
     * @brief Check if symbol exists
     * @param name Symbol name to check
     * @return true if symbol exists
     */
    bool hasSymbol(const std::string& name) const;
    
    /**
     * @brief Get mutable symbol reference
     * @param name Symbol name
     * @return Pointer to symbol, or nullptr if not found
     */
    Symbol* getSymbol(const std::string& name);
    
    /**
     * @brief Get const symbol reference
     * @param name Symbol name
     * @return Const pointer to symbol, or nullptr if not found
     */
    const Symbol* getSymbol(const std::string& name) const;
    
    /**
     * @brief Get all PUBLIC symbols
     * @return Vector of public symbols
     */
    std::vector<const Symbol*> getPublicSymbols() const;
    
    /**
     * @brief Get all EXTERNAL symbols
     * @return Vector of external symbols
     */
    std::vector<const Symbol*> getExternalSymbols() const;
    
    /**
     * @brief Get all symbols (for iteration)
     * @return Const reference to symbol map
     */
    const std::map<std::string, Symbol>& getAllSymbols() const { return symbols_; }
    
private:
    /**
     * @brief Convert symbol name to uppercase (M80 is case-insensitive)
     * @param name Symbol name
     * @return Uppercase version of name
     */
    std::string toUpper(const std::string& name) const;
    
    std::map<std::string, Symbol> symbols_;  ///< Map of symbol names to data (keys are uppercase)
};

} // namespace z80
