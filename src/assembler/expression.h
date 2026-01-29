/**
 * @file expression.h
 * @brief Expression evaluator for assembly-time arithmetic
 * 
 * Evaluates expressions in assembly source code, including:
 * - Arithmetic operations (+, -, *, /, MOD)
 * - Bitwise operations (AND, OR, XOR, NOT, SHL, SHR)
 * - Relational operations (EQ, NE, LT, LE, GT, GE)
 * - Special symbols ($ for location counter)
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>

namespace z80 {

/**
 * @class Expression
 * @brief Evaluates assembly-time expressions
 * 
 * Supports M80-compatible expression syntax with proper operator
 * precedence and support for forward references.
 * 
 * @todo Implement expression parser and evaluator
 */
class Expression {
public:
    /** @brief Construct an expression evaluator */
    Expression();
    
    /**
     * @brief Evaluate an expression string
     * @param expr Expression to evaluate
     * @return Evaluated numeric value
     * @todo Implement evaluation logic
     */
    int64_t evaluate(const std::string& expr);
};

} // namespace z80
