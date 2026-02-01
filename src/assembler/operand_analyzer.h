/**
 * @file operand_analyzer.h
 * @brief Operand parsing and pattern matching for Z80 instructions
 * 
 * Provides utilities for analyzing assembly operands and converting them
 * to patterns for instruction table lookup.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>
#include <vector>
#include "lexer.h"
#include "symbol_table.h"
#include "z80_instructions.h"

namespace z80 {

/**
 * @class OperandAnalyzer
 * @brief Analyzes and classifies Z80 assembly operands
 * 
 * Converts operand strings to abstract patterns used for instruction
 * table lookups. Handles register names, addressing modes, immediate
 * values, and context-sensitive pattern matching.
 */
class OperandAnalyzer {
public:
    /**
     * @brief Construct operand analyzer
     * 
     * @param instructions Reference to instruction set (for register validation)
     * @param symbolTable Reference to symbol table (for symbol resolution)
     */
    OperandAnalyzer(const Z80Instructions& instructions, 
                    const SymbolTable& symbolTable);
    
    /**
     * @brief Parses operands from a lexer token stream
     * 
     * Handles comma-separated operands with proper handling of:
     * - Parentheses depth tracking for expressions like (IX+5)
     * - String literals with quotes
     * - Whitespace between tokens
     * 
     * @param lexer Lexer positioned after the mnemonic
     * @param operands Output vector of parsed operand strings
     * @return Full operand string (all operands joined with commas)
     */
    std::string parseOperands(Lexer& lexer, std::vector<std::string>& operands);
    
    /**
     * @brief Searches instruction table for a matching variant
     * 
     * Tries to find an instruction that matches the mnemonic and operand patterns.
     * Implements fallback logic:
     * - First tries exact pattern match
     * - If NN (16-bit) in pattern, tries N (8-bit) variant
     * - If N (8-bit) in pattern, tries NN (16-bit) variant
     * 
     * This allows flexible matching where symbol values determine the instruction size.
     * 
     * @param mnemonic Instruction mnemonic (e.g., "LD", "ADD")
     * @param operands Vector of operand strings (e.g., {"A", "(HL)"})
     * @return Pointer to matching InstructionInfo, or nullptr if not found
     */
    const InstructionInfo* findInstructionVariant(const std::string& mnemonic,
                                                   const std::vector<std::string>& operands);
    
    /**
     * @brief Converts an operand string to a pattern for instruction matching
     * 
     * Analyzes operands and converts them to abstract patterns used by the instruction
     * table. Examples:
     * - "A" → "A" (register)
     * - "(HL)" → "(HL)" (indirect register)
     * - "(IX+5)" → "(IX+D)" (indexed addressing)
     * - "100" → "N" (8-bit immediate) or "NN" (16-bit immediate)
     * - "(port)" → "(N)" for IN/OUT, "(NN)" for other instructions
     * - "7" → "7" for BIT/SET/RES, "N" for other instructions
     * 
     * Special handling:
     * - Resolves symbols to determine value range
     * - Distinguishes 8-bit ports from 16-bit addresses based on instruction
     * - Handles bit numbers (0-7) for BIT/SET/RES instructions
     * 
     * @param operand Operand string (e.g., "A", "(HL)", "label", "255")
     * @param mnemonic Instruction mnemonic for context-sensitive pattern matching
     * @return Pattern string for instruction table lookup
     */
    std::string operandToPattern(const std::string& operand, const std::string& mnemonic = "");
    
    /**
     * @brief Checks if an operand string is a Z80 register name
     * 
     * @param operand Operand string to check (case-insensitive)
     * @return true if operand is a valid Z80 register (A, B, C, D, E, H, L, AF, BC, DE, HL, SP, IX, IY, I, R)
     */
    bool isRegisterOperand(const std::string& operand);

private:
    const Z80Instructions& instructions_;  ///< Reference to instruction set
    const SymbolTable& symbolTable_;       ///< Reference to symbol table
};

} // namespace z80
