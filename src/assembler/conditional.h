/**
 * @file conditional.h
 * @brief Conditional assembly processor
 * 
 * Handles IF/ELSE/ENDIF conditional assembly directives.
 * Supports: IF, IFT, IFF, IFE, IF1, IF2, IFDEF, IFNDEF, IFB, IFNB, IFIDN, IFDIF
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>
#include <vector>
#include <stack>

namespace z80 {

// Forward declarations
class SymbolTable;

/**
 * @enum ConditionalType
 * @brief Type of conditional directive
 */
enum class ConditionalType {
    IF,         ///< IF/IFT - True if expression != 0
    IFE,        ///< IFE/IFF - True if expression == 0
    IF1,        ///< IF1 - True if pass 1
    IF2,        ///< IF2 - True if pass 2
    IFDEF,      ///< IFDEF - True if symbol defined
    IFNDEF,     ///< IFNDEF - True if symbol not defined
    IFB,        ///< IFB - True if argument blank
    IFNB,       ///< IFNB - True if argument not blank
    IFIDN,      ///< IFIDN - True if strings identical
    IFDIF       ///< IFDIF - True if strings different
};

/**
 * @struct ConditionalBlock
 * @brief State of a conditional assembly block
 */
struct ConditionalBlock {
    ConditionalType type;       ///< Type of conditional
    bool condition;             ///< Evaluated condition result
    bool elseEncountered;       ///< True if ELSE was encountered
    bool assembling;            ///< True if currently assembling (parent chain)
    int startLine;              ///< Line number where block started
};

/**
 * @class ConditionalProcessor
 * @brief Processes conditional assembly directives
 * 
 * Features:
 * - Nested conditionals (up to 255 levels)
 * - Pass-dependent conditionals (IF1/IF2)
 * - Symbol existence tests (IFDEF/IFNDEF)
 * - String comparison (IFIDN/IFDIF)
 * - Blank argument tests (IFB/IFNB)
 */
class ConditionalProcessor {
public:
    /**
     * @brief Construct a new ConditionalProcessor
     */
    ConditionalProcessor();
    
    /**
     * @brief Set current assembly pass (1 or 2)
     * @param pass Pass number
     */
    void setPass(int pass);
    
    /**
     * @brief Check if a line should be assembled
     * @return true if line should be assembled
     */
    bool shouldAssemble() const;
    
    /**
     * @brief Get current nesting depth
     * @return int Nesting depth (0 = no conditionals active)
     */
    int getDepth() const { return static_cast<int>(stack_.size()); }
    
    /**
     * @brief Process IF directive
     * @param expression Expression to evaluate
     * @param symbolTable Symbol table for symbol resolution
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIF(const std::string& expression, const SymbolTable& symbolTable, int lineNumber);
    
    /**
     * @brief Process IFE/IFF directive (true if expression == 0)
     * @param expression Expression to evaluate
     * @param symbolTable Symbol table for symbol resolution
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIFE(const std::string& expression, const SymbolTable& symbolTable, int lineNumber);
    
    /**
     * @brief Process IF1 directive (true on pass 1)
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIF1(int lineNumber);
    
    /**
     * @brief Process IF2 directive (true on pass 2)
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIF2(int lineNumber);
    
    /**
     * @brief Process IFDEF directive
     * @param symbolName Symbol to check
     * @param symbolTable Symbol table
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIFDEF(const std::string& symbolName, const SymbolTable& symbolTable, int lineNumber);
    
    /**
     * @brief Process IFNDEF directive
     * @param symbolName Symbol to check
     * @param symbolTable Symbol table
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIFNDEF(const std::string& symbolName, const SymbolTable& symbolTable, int lineNumber);
    
    /**
     * @brief Process IFB directive (true if argument blank)
     * @param argument Argument to test (in angle brackets)
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIFB(const std::string& argument, int lineNumber);
    
    /**
     * @brief Process IFNB directive (true if argument not blank)
     * @param argument Argument to test (in angle brackets)
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIFNB(const std::string& argument, int lineNumber);
    
    /**
     * @brief Process IFIDN directive (true if strings identical)
     * @param arg1 First argument (in angle brackets)
     * @param arg2 Second argument (in angle brackets)
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIFIDN(const std::string& arg1, const std::string& arg2, int lineNumber);
    
    /**
     * @brief Process IFDIF directive (true if strings different)
     * @param arg1 First argument (in angle brackets)
     * @param arg2 Second argument (in angle brackets)
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processIFDIF(const std::string& arg1, const std::string& arg2, int lineNumber);
    
    /**
     * @brief Process ELSE directive
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processELSE(int lineNumber);
    
    /**
     * @brief Process ENDIF directive
     * @param lineNumber Current line number
     * @return true if directive processed successfully
     */
    bool processENDIF(int lineNumber);
    
    /**
     * @brief Check for unterminated conditionals
     * @return true if all conditionals properly closed
     */
    bool checkUnterminated() const;
    
    /**
     * @brief Clear all conditional state
     */
    void clear();
    
private:
    /**
     * @brief Evaluate expression to integer
     * @param expression Expression string
     * @param symbolTable Symbol table for resolution
     * @param result Output: result value
     * @return true if evaluation succeeded
     */
    bool evaluateExpression(const std::string& expression, const SymbolTable& symbolTable, int64_t& result);
    
    /**
     * @brief Extract argument from angle brackets <arg>
     * @param text Input text with angle brackets
     * @return Extracted argument (empty if no brackets or blank)
     */
    std::string extractArgument(const std::string& text);
    
    /**
     * @brief Push new conditional block
     * @param type Conditional type
     * @param condition Evaluated condition
     * @param lineNumber Line number
     */
    void pushBlock(ConditionalType type, bool condition, int lineNumber);
    
    std::stack<ConditionalBlock> stack_;    ///< Conditional block stack
    int currentPass_;                       ///< Current pass (1 or 2)
    static const int MAX_NESTING = 255;     ///< Maximum nesting depth
};

} // namespace z80
