/**
 * @file expression.h
 * @brief Expression evaluator for Z80 assembly
 * 
 * Evaluates arithmetic and logical expressions following M80 operator precedence.
 * Supports:
 * - Integer constants (decimal, hex, octal, binary)
 * - Symbols (labels, EQU constants)
 * - Location counter ($)
 * - Arithmetic operators (+, -, *, /, MOD, SHL, SHR)
 * - Logical operators (NOT, AND, OR, XOR)
 * - Relational operators (EQ, NE, LT, LE, GT, GE)
 * - Parentheses for grouping
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include "common/types.h"
#include "symbol_table.h"
#include <string>

namespace z80 {

/**
 * @enum ExpressionType
 * @brief Type of expression result
 */
enum class ExpressionType {
    Absolute,      ///< Absolute value (no relocation needed)
    Relocatable,   ///< Relocatable address (needs relocation)
    External       ///< External symbol reference
};

/**
 * @struct ExpressionResult
 * @brief Result of expression evaluation
 */
struct ExpressionResult {
    int64_t value;              ///< Computed value
    ExpressionType type;        ///< Type of result
    bool valid;                 ///< True if evaluation succeeded
    std::string errorMessage;   ///< Error message if invalid
    
    ExpressionResult() : value(0), type(ExpressionType::Absolute), valid(false) {}
    ExpressionResult(int64_t val, ExpressionType typ = ExpressionType::Absolute) 
        : value(val), type(typ), valid(true) {}
};

/**
 * @class ExpressionEvaluator
 * @brief Evaluates assembly expressions with M80 operator precedence
 * 
 * Operator precedence (highest to lowest):
 * 1. Parentheses ()
 * 2. Unary operators: +, -, NOT
 * 3. Multiplication/Division: *, /, MOD, SHL, SHR
 * 4. Addition/Subtraction: +, -
 * 5. Relational: EQ, NE, LT, LE, GT, GE
 * 6. Bitwise AND
 * 7. Bitwise XOR
 * 8. Bitwise OR
 */
class ExpressionEvaluator {
public:
    /**
     * @brief Construct evaluator with symbol table and context
     * @param symbolTable Symbol table for label/constant lookup
     * @param locationCounter Current assembly address ($)
     * @param currentSegment Current segment type
     */
    ExpressionEvaluator(const SymbolTable& symbolTable,
                       Address locationCounter = 0,
                       SegmentType currentSegment = SegmentType::CSEG);
    
    /**
     * @brief Evaluate an expression string
     * @param expression Expression to evaluate
     * @return Result with value and type
     */
    ExpressionResult evaluate(const std::string& expression);
    
    /**
     * @brief Set current location counter ($)
     * @param address Current address
     */
    void setLocationCounter(Address address) { locationCounter_ = address; }
    
    /**
     * @brief Set current segment
     * @param segment Current segment type
     */
    void setSegment(SegmentType segment) { currentSegment_ = segment; }

private:
    // Expression parsing methods (recursive descent)
    ExpressionResult parseOrExpression(const std::string& expr, size_t& pos);
    ExpressionResult parseXorExpression(const std::string& expr, size_t& pos);
    ExpressionResult parseAndExpression(const std::string& expr, size_t& pos);
    ExpressionResult parseRelationalExpression(const std::string& expr, size_t& pos);
    ExpressionResult parseAdditiveExpression(const std::string& expr, size_t& pos);
    ExpressionResult parseMultiplicativeExpression(const std::string& expr, size_t& pos);
    ExpressionResult parseUnaryExpression(const std::string& expr, size_t& pos);
    ExpressionResult parsePrimaryExpression(const std::string& expr, size_t& pos);
    
    // Helper methods
    void skipWhitespace(const std::string& expr, size_t& pos);
    bool matchKeyword(const std::string& expr, size_t& pos, const std::string& keyword);
    bool matchOperator(const std::string& expr, size_t& pos, char op);
    std::string parseIdentifier(const std::string& expr, size_t& pos);
    ExpressionResult parseNumber(const std::string& expr, size_t& pos);
    
    // Type combination for arithmetic
    ExpressionType combineTypes(ExpressionType left, ExpressionType right, char op);
    
    const SymbolTable& symbolTable_;
    Address locationCounter_;
    SegmentType currentSegment_;
};

} // namespace z80
