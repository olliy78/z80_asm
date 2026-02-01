/**
 * @file conditional.cpp
 * @brief Implementation of conditional assembly processor
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include "conditional.h"
#include "symbol_table.h"
#include "expression.h"
#include <algorithm>
#include <cctype>
#include <iostream>

namespace z80 {

ConditionalProcessor::ConditionalProcessor()
    : currentPass_(1)
{
}

void ConditionalProcessor::setPass(int pass) {
    currentPass_ = pass;
}

bool ConditionalProcessor::shouldAssemble() const {
    if (stack_.empty()) {
        return true;
    }
    
    // Walk through stack - if any parent is not assembling, we don't assemble
    std::stack<ConditionalBlock> temp = stack_;
    while (!temp.empty()) {
        const ConditionalBlock& block = temp.top();
        if (!block.assembling) {
            return false;
        }
        temp.pop();
    }
    
    return true;
}

bool ConditionalProcessor::processIF(const std::string& expression, const SymbolTable& symbolTable, int lineNumber) {
    int64_t result = 0;
    bool evaluated = evaluateExpression(expression, symbolTable, result);
    
    // IF is true if expression != 0
    bool condition = evaluated && (result != 0);
    pushBlock(ConditionalType::IF, condition, lineNumber);
    return true;
}

bool ConditionalProcessor::processIFE(const std::string& expression, const SymbolTable& symbolTable, int lineNumber) {
    int64_t result = 0;
    bool evaluated = evaluateExpression(expression, symbolTable, result);
    
    // IFE is true if expression == 0
    bool condition = evaluated && (result == 0);
    pushBlock(ConditionalType::IFE, condition, lineNumber);
    return true;
}

bool ConditionalProcessor::processIF1(int lineNumber) {
    bool condition = (currentPass_ == 1);
    pushBlock(ConditionalType::IF1, condition, lineNumber);
    return true;
}

bool ConditionalProcessor::processIF2(int lineNumber) {
    bool condition = (currentPass_ == 2);
    pushBlock(ConditionalType::IF2, condition, lineNumber);
    return true;
}

bool ConditionalProcessor::processIFDEF(const std::string& symbolName, const SymbolTable& symbolTable, int lineNumber) {
    // Symbol is defined if it exists in symbol table or is external
    const Symbol* sym = symbolTable.getSymbol(symbolName);
    bool condition = (sym != nullptr);
    
    pushBlock(ConditionalType::IFDEF, condition, lineNumber);
    return true;
}

bool ConditionalProcessor::processIFNDEF(const std::string& symbolName, const SymbolTable& symbolTable, int lineNumber) {
    // Symbol is not defined if it doesn't exist
    const Symbol* sym = symbolTable.getSymbol(symbolName);
    bool condition = (sym == nullptr);
    
    pushBlock(ConditionalType::IFNDEF, condition, lineNumber);
    return true;
}

bool ConditionalProcessor::processIFB(const std::string& argument, int lineNumber) {
    std::string arg = extractArgument(argument);
    
    // Blank means empty or only whitespace
    bool isBlank = true;
    for (char c : arg) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            isBlank = false;
            break;
        }
    }
    
    pushBlock(ConditionalType::IFB, isBlank, lineNumber);
    return true;
}

bool ConditionalProcessor::processIFNB(const std::string& argument, int lineNumber) {
    std::string arg = extractArgument(argument);
    
    // Not blank means contains non-whitespace
    bool isBlank = true;
    for (char c : arg) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            isBlank = false;
            break;
        }
    }
    
    pushBlock(ConditionalType::IFNB, !isBlank, lineNumber);
    return true;
}

bool ConditionalProcessor::processIFIDN(const std::string& arg1, const std::string& arg2, int lineNumber) {
    std::string a1 = extractArgument(arg1);
    std::string a2 = extractArgument(arg2);
    
    // Case-sensitive comparison
    bool identical = (a1 == a2);
    
    pushBlock(ConditionalType::IFIDN, identical, lineNumber);
    return true;
}

bool ConditionalProcessor::processIFDIF(const std::string& arg1, const std::string& arg2, int lineNumber) {
    std::string a1 = extractArgument(arg1);
    std::string a2 = extractArgument(arg2);
    
    // Case-sensitive comparison
    bool different = (a1 != a2);
    
    pushBlock(ConditionalType::IFDIF, different, lineNumber);
    return true;
}

bool ConditionalProcessor::processELSE(int lineNumber) {
    if (stack_.empty()) {
        return false; // ELSE without IF
    }
    
    ConditionalBlock& block = stack_.top();
    
    if (block.elseEncountered) {
        return false; // Multiple ELSE for same IF
    }
    
    block.elseEncountered = true;
    
    // Flip condition and update assembling state
    block.condition = !block.condition;
    
    // Update assembling flag: true if condition is true AND all parents are assembling
    bool parentAssembling = true;
    if (stack_.size() > 1) {
        std::stack<ConditionalBlock> temp = stack_;
        temp.pop(); // Skip current block
        while (!temp.empty()) {
            if (!temp.top().assembling) {
                parentAssembling = false;
                break;
            }
            temp.pop();
        }
    }
    
    block.assembling = parentAssembling && block.condition;
    
    return true;
}

bool ConditionalProcessor::processENDIF(int lineNumber) {
    if (stack_.empty()) {
        return false; // ENDIF without IF
    }
    
    stack_.pop();
    return true;
}

bool ConditionalProcessor::checkUnterminated() const {
    return stack_.empty();
}

void ConditionalProcessor::clear() {
    while (!stack_.empty()) {
        stack_.pop();
    }
}

bool ConditionalProcessor::evaluateExpression(const std::string& expression, 
                                              const SymbolTable& symbolTable, 
                                              int64_t& result) {
    // Use expression evaluator
    ExpressionEvaluator evaluator(symbolTable, 0, SegmentType::ASEG);
    ExpressionResult exprResult = evaluator.evaluate(expression);
    
    if (exprResult.valid) {
        result = exprResult.value;
        return true;
    }
    
    return false;
}

std::string ConditionalProcessor::extractArgument(const std::string& text) {
    // Find angle brackets
    size_t start = text.find('<');
    size_t end = text.find('>');
    
    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return ""; // No brackets or invalid
    }
    
    return text.substr(start + 1, end - start - 1);
}

void ConditionalProcessor::pushBlock(ConditionalType type, bool condition, int lineNumber) {
    // Check nesting depth
    if (static_cast<int>(stack_.size()) >= MAX_NESTING) {
        return; // Too deep
    }
    
    ConditionalBlock block;
    block.type = type;
    block.condition = condition;
    block.elseEncountered = false;
    block.startLine = lineNumber;
    
    // Determine if we should assemble: condition is true AND all parents are assembling
    bool parentAssembling = true;
    if (!stack_.empty()) {
        // Check if parent chain allows assembling
        std::stack<ConditionalBlock> temp = stack_;
        while (!temp.empty()) {
            if (!temp.top().assembling) {
                parentAssembling = false;
                break;
            }
            temp.pop();
        }
    }
    
    block.assembling = parentAssembling && condition;
    
    stack_.push(block);
}

} // namespace z80
