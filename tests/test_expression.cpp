/**
 * @file test_expression.cpp
 * @brief Tests for expression evaluator
 */

#include "assembler/expression.h"
#include "assembler/symbol_table.h"
#include <iostream>
#include <cassert>

using namespace z80;

void testSimpleNumbers() {
    std::cout << "Testing simple numbers..." << std::endl;
    
    SymbolTable symbols;
    ExpressionEvaluator eval(symbols, 0x1000, SegmentType::CSEG);
    
    // Decimal
    auto result = eval.evaluate("42");
    assert(result.valid && result.value == 42);
    assert(result.type == ExpressionType::Absolute);
    
    // Hex
    result = eval.evaluate("0FFH");
    assert(result.valid && result.value == 0xFF);
    
    result = eval.evaluate("1234H");
    assert(result.valid && result.value == 0x1234);
    
    std::cout << "  ✓ Simple numbers passed" << std::endl;
}

void testArithmetic() {
    std::cout << "Testing arithmetic..." << std::endl;
    
    SymbolTable symbols;
    ExpressionEvaluator eval(symbols, 0x1000, SegmentType::CSEG);
    
    // Addition
    auto result = eval.evaluate("2 + 3");
    assert(result.valid && result.value == 5);
    
    // Subtraction
    result = eval.evaluate("10 - 3");
    assert(result.valid && result.value == 7);
    
    // Multiplication
    result = eval.evaluate("4 * 5");
    assert(result.valid && result.value == 20);
    
    // Division
    result = eval.evaluate("20 / 4");
    assert(result.valid && result.value == 5);
    
    // Modulo
    result = eval.evaluate("17 MOD 5");
    assert(result.valid && result.value == 2);
    
    std::cout << "  ✓ Arithmetic passed" << std::endl;
}

void testPrecedence() {
    std::cout << "Testing operator precedence..." << std::endl;
    
    SymbolTable symbols;
    ExpressionEvaluator eval(symbols, 0x1000, SegmentType::CSEG);
    
    // Multiplication before addition
    auto result = eval.evaluate("2 + 3 * 4");
    assert(result.valid && result.value == 14);
    
    // Parentheses override precedence
    result = eval.evaluate("(2 + 3) * 4");
    assert(result.valid && result.value == 20);
    
    // Complex expression
    result = eval.evaluate("10 + 20 / 5 - 3");
    assert(result.valid && result.value == 11);
    
    std::cout << "  ✓ Precedence passed" << std::endl;
}

void testBitwise() {
    std::cout << "Testing bitwise operations..." << std::endl;
    
    SymbolTable symbols;
    ExpressionEvaluator eval(symbols, 0x1000, SegmentType::CSEG);
    
    // AND
    auto result = eval.evaluate("0FH AND 03H");
    assert(result.valid && result.value == 0x03);
    
    // OR
    result = eval.evaluate("01H OR 02H");
    assert(result.valid && result.value == 0x03);
    
    // XOR
    result = eval.evaluate("0FH XOR 0AH");
    assert(result.valid && result.value == 0x05);
    
    // NOT
    result = eval.evaluate("NOT 0");
    assert(result.valid && result.value == -1);
    
    // Shift left
    result = eval.evaluate("1 SHL 4");
    assert(result.valid && result.value == 16);
    
    // Shift right
    result = eval.evaluate("16 SHR 2");
    assert(result.valid && result.value == 4);
    
    std::cout << "  ✓ Bitwise operations passed" << std::endl;
}

void testRelational() {
    std::cout << "Testing relational operators..." << std::endl;
    
    SymbolTable symbols;
    ExpressionEvaluator eval(symbols, 0x1000, SegmentType::CSEG);
    
    // Equal
    auto result = eval.evaluate("5 EQ 5");
    assert(result.valid && result.value == 1);
    
    result = eval.evaluate("5 EQ 3");
    assert(result.valid && result.value == 0);
    
    // Not equal
    result = eval.evaluate("5 NE 3");
    assert(result.valid && result.value == 1);
    
    // Less than
    result = eval.evaluate("3 LT 5");
    assert(result.valid && result.value == 1);
    
    result = eval.evaluate("5 LT 3");
    assert(result.valid && result.value == 0);
    
    // Less or equal
    result = eval.evaluate("5 LE 5");
    assert(result.valid && result.value == 1);
    
    // Greater than
    result = eval.evaluate("5 GT 3");
    assert(result.valid && result.value == 1);
    
    // Greater or equal
    result = eval.evaluate("5 GE 5");
    assert(result.valid && result.value == 1);
    
    std::cout << "  ✓ Relational operators passed" << std::endl;
}

void testSymbols() {
    std::cout << "Testing symbol lookup..." << std::endl;
    
    SymbolTable symbols;
    
    Symbol start;
    start.value = 0x1000;
    start.isRelocatable = false;
    start.isExternal = false;
    start.segment = SegmentType::CSEG;
    symbols.addSymbol("START", start);
    
    Symbol offset;
    offset.value = 0x10;
    offset.isRelocatable = true;
    offset.isExternal = false;
    offset.segment = SegmentType::CSEG;
    symbols.addSymbol("OFFSET", offset);
    
    ExpressionEvaluator eval(symbols, 0x2000, SegmentType::CSEG);
    
    // Absolute symbol
    auto result = eval.evaluate("START");
    assert(result.valid && result.value == 0x1000);
    assert(result.type == ExpressionType::Absolute);
    
    // Relocatable symbol
    result = eval.evaluate("OFFSET");
    assert(result.valid && result.value == 0x10);
    assert(result.type == ExpressionType::Relocatable);
    
    // Symbol in expression
    result = eval.evaluate("START + 5");
    assert(result.valid && result.value == 0x1005);
    
    // Undefined symbol
    result = eval.evaluate("UNDEFINED");
    assert(!result.valid);
    assert(result.errorMessage.find("Undefined") != std::string::npos);
    
    std::cout << "  ✓ Symbol lookup passed" << std::endl;
}

void testLocationCounter() {
    std::cout << "Testing location counter ($)..." << std::endl;
    
    SymbolTable symbols;
    
    // In CSEG (relocatable)
    ExpressionEvaluator eval1(symbols, 0x1234, SegmentType::CSEG);
    auto result = eval1.evaluate("$");
    assert(result.valid && result.value == 0x1234);
    assert(result.type == ExpressionType::Relocatable);
    
    // In ASEG (absolute)
    ExpressionEvaluator eval2(symbols, 0x5678, SegmentType::ASEG);
    result = eval2.evaluate("$");
    assert(result.valid && result.value == 0x5678);
    assert(result.type == ExpressionType::Absolute);
    
    // In expression
    result = eval1.evaluate("$ + 10");
    assert(result.valid && result.value == 0x123E);
    assert(result.type == ExpressionType::Relocatable);
    
    std::cout << "  ✓ Location counter passed" << std::endl;
}

void testTypeCombination() {
    std::cout << "Testing type combination..." << std::endl;
    
    SymbolTable symbols;
    
    Symbol absSymbol;
    absSymbol.value = 100;
    absSymbol.isRelocatable = false;
    absSymbol.isExternal = false;
    absSymbol.segment = SegmentType::ASEG;
    symbols.addSymbol("ABS", absSymbol);
    
    Symbol relSymbol;
    relSymbol.value = 200;
    relSymbol.isRelocatable = true;
    relSymbol.isExternal = false;
    relSymbol.segment = SegmentType::CSEG;
    symbols.addSymbol("REL", relSymbol);
    
    ExpressionEvaluator eval(symbols, 0x1000, SegmentType::CSEG);
    
    // Absolute + Absolute = Absolute
    auto result = eval.evaluate("ABS + 10");
    assert(result.valid && result.type == ExpressionType::Absolute);
    
    // Relocatable + Absolute = Relocatable
    result = eval.evaluate("REL + 10");
    assert(result.valid && result.type == ExpressionType::Relocatable);
    
    // Relocatable - Absolute = Relocatable
    result = eval.evaluate("REL - 10");
    assert(result.valid && result.type == ExpressionType::Relocatable);
    
    // Relocatable - Relocatable = Absolute (offset)
    result = eval.evaluate("REL - REL");
    assert(result.valid && result.type == ExpressionType::Absolute);
    assert(result.value == 0);
    
    std::cout << "  ✓ Type combination passed" << std::endl;
}

void testUnaryOperators() {
    std::cout << "Testing unary operators..." << std::endl;
    
    SymbolTable symbols;
    ExpressionEvaluator eval(symbols, 0x1000, SegmentType::CSEG);
    
    // Unary plus
    auto result = eval.evaluate("+42");
    assert(result.valid && result.value == 42);
    
    // Unary minus
    result = eval.evaluate("-42");
    assert(result.valid && result.value == -42);
    
    // Double negative
    result = eval.evaluate("--42");
    assert(result.valid && result.value == 42);
    
    // NOT
    result = eval.evaluate("NOT 0FFH");
    assert(result.valid && result.value == (int64_t)~0xFF);
    
    std::cout << "  ✓ Unary operators passed" << std::endl;
}

void testErrors() {
    std::cout << "Testing error conditions..." << std::endl;
    
    SymbolTable symbols;
    ExpressionEvaluator eval(symbols, 0x1000, SegmentType::CSEG);
    
    // Empty expression
    auto result = eval.evaluate("");
    assert(!result.valid);
    
    // Division by zero
    result = eval.evaluate("10 / 0");
    assert(!result.valid);
    assert(result.errorMessage.find("zero") != std::string::npos);
    
    // Modulo by zero
    result = eval.evaluate("10 MOD 0");
    assert(!result.valid);
    
    // Missing closing parenthesis
    result = eval.evaluate("(10 + 5");
    assert(!result.valid);
    
    // Extra characters after expression
    result = eval.evaluate("42 xyz");
    assert(!result.valid);
    
    std::cout << "  ✓ Error handling passed" << std::endl;
}

int main() {
    std::cout << "Running expression evaluator tests..." << std::endl << std::endl;
    
    testSimpleNumbers();
    testArithmetic();
    testPrecedence();
    testBitwise();
    testRelational();
    testSymbols();
    testLocationCounter();
    testTypeCombination();
    testUnaryOperators();
    testErrors();
    
    std::cout << std::endl << "All expression evaluator tests passed! ✓" << std::endl;
    return 0;
}
