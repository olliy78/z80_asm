/**
 * @file parser.h
 * @brief Parser for Z80 assembly language
 * 
 * The parser performs syntactic analysis and code generation.
 * It implements a two-pass assembly process:
 * - Pass 1: Build symbol table and determine addresses
 * - Pass 2: Generate machine code and relocation information
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "lexer.h"
#include "symbol_table.h"
#include "z80_instructions.h"
#include "rel_writer.h"
#include "errors.h"
#include "macro.h"
#include "conditional.h"

namespace z80 {

/**
 * @struct ParsedLine
 * @brief Represents a parsed assembly line
 */
struct ParsedLine {
    std::string label;                  ///< Optional label (empty if none)
    std::string mnemonic;               ///< Instruction or directive mnemonic
    std::vector<std::string> operands;  ///< Operands (expressions)
    std::string operandString;          ///< Full operand string for instruction lookup
    std::string comment;                ///< Comment text
    int lineNumber;                     ///< Source line number
    Address address;                    ///< Address assigned in pass 1
    std::vector<Byte> code;             ///< Generated machine code (pass 2)
    SegmentType segment;                ///< Current segment type
};

/**
 * @class Parser
 * @brief Syntax analyzer and code generator for Z80 assembly
 * 
 * Implements two-pass assembly:
 * - Pass 1: Collects symbols and calculates addresses
 * - Pass 2: Generates machine code using resolved symbols
 */
class Parser {
public:
    /**
     * @brief Construct a new Parser
     */
    Parser();
    
    /**
     * @brief Parse and assemble a source file
     * 
     * @param filename Source file path
     * @return true if assembly succeeded
     * @return false if errors occurred
     */
    bool assemble(const std::string& filename);
    
    /**
     * @brief Get assembled lines (after pass 2)
     * @return const std::vector<ParsedLine>& Parsed lines with code
     */
    const std::vector<ParsedLine>& getLines() const { return lines_; }
    
    /**
     * @brief Get symbol table
     * @return const SymbolTable& Symbol table
     */
    const SymbolTable& getSymbolTable() const;
    
    /**
     * @brief Get errors encountered during assembly
     * @return const std::vector<AssemblyError>& List of errors
     */
    const std::vector<AssemblyError>& getErrors() const;
    
    /**
     * @brief Check if assembly had errors
     * @return true if errors occurred
     */
    bool hasErrors() const { return !errors_.empty(); }
    
    /**
     * @brief Write assembled output to .REL file
     * @param filename Output .REL filename
     * @param moduleName Module name (optional, derived from filename if empty)
     * @return true if successful
     * @deprecated Use Assembler + RELOutputWriter instead for better separation of concerns
     */
    [[deprecated("Use Assembler + RELOutputWriter instead")]]
    bool writeREL(const std::string& filename, const std::string& moduleName = "");
    
private:
    /**
     * @brief Perform pass 1: build symbol table
     * @param sourceLines Raw source code lines
     * @param filename Name of source file
     * @return true if pass 1 succeeded
     */
    bool pass1(const std::vector<std::string>& sourceLines, const std::string& filename);
    
    /**
     * @brief Perform pass 2: generate code
     * @param sourceLines Raw source code lines
     * @param filename Name of source file
     * @return true if pass 2 succeeded
     */
    bool pass2(const std::vector<std::string>& sourceLines, const std::string& filename);
    
    /**
     * @brief Generate code for DB directive
     * @param line Parsed line
     * @param filename Source filename
     * @return true if successful
     */
    bool generateDB(ParsedLine& line, const std::string& filename);
    
    /**
     * @brief Generate code for DW directive
     * @param line Parsed line
     * @param filename Source filename
     * @return true if successful
     */
    bool generateDW(ParsedLine& line, const std::string& filename);
    
    /**
     * @brief Generate code for instruction
     * @param line Parsed line
     * @param filename Source filename
     * @return true if successful
     */
    bool generateInstruction(ParsedLine& line, const std::string& filename);
    
    /**
     * @brief Parse operands from token stream
     * @param lexer Lexer positioned after mnemonic
     * @param operands Output vector of operand strings
     * @return Combined operand string for logging
     */
    std::string parseOperands(Lexer& lexer, std::vector<std::string>& operands);
    
    /**
     * @brief Find best matching instruction variant for given operands
     * @param mnemonic Instruction mnemonic
     * @param operands Parsed operand list
     * @return Pointer to instruction info, or nullptr
     */
    const InstructionInfo* findInstructionVariant(const std::string& mnemonic,
                                                   const std::vector<std::string>& operands);
    
    /**
     * @brief Convert operand to addressing pattern
     * @param operand Single operand string
     * @param mnemonic Instruction mnemonic (for context-sensitive parsing)
     * @return Pattern like "A", "N", "NN", "(HL)", "(IX+D)", etc.
     */
    std::string operandToPattern(const std::string& operand, const std::string& mnemonic = "");
    
    /**
     * @brief Check if operand is a register
     * @param operand Operand string
     * @return true if it's a register name
     */
    bool isRegisterOperand(const std::string& operand);
    
    /**
     * @brief Expand source lines with macro processing
     * @param sourceLines Original source lines
     * @param expandedLines Output: expanded lines with macros processed
     * @param filename Source filename
     * @return true if successful
     */
    bool expandSourceWithMacros(const std::vector<std::string>& sourceLines,
                                std::vector<std::string>& expandedLines,
                                const std::string& filename);
    
    SymbolTable symbolTable_;           ///< Symbol table
    Z80Instructions instructions_;      ///< Z80 instruction set
    MacroProcessor macroProcessor_;     ///< Macro processor
    ConditionalProcessor conditionalProcessor_; ///< Conditional assembly processor
    std::vector<ParsedLine> lines_;     ///< Parsed lines
    std::vector<AssemblyError> errors_; ///< Assembly errors
    
    Address locationCounter_;           ///< Current address (location counter)
    SegmentType currentSegment_;        ///< Current segment (CSEG default)
    Address csegOrigin_;                ///< CSEG origin
    Address dsegOrigin_;                ///< DSEG origin
    Address asegOrigin_;                ///< ASEG origin
    std::string moduleName_;            ///< Module name (from NAME/TITLE directive)
    
    // PHASE/DEPHASE support
    bool inPhase_;                      ///< True if in PHASE block
    Address phaseOrigin_;               ///< Origin address before PHASE
    Address phaseOffset_;               ///< Phase offset (runtime - assembly address)
    
    // MACRO/ENDM support
    bool inMacroDefinition_;            ///< True if defining a macro
    MacroDefinition currentMacro_;      ///< Current macro being defined
    std::vector<std::string> macroBody_; ///< Lines of current macro body
};

} // namespace z80
