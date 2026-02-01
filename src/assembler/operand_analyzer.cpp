/**
 * @file operand_analyzer.cpp
 * @brief Implementation of operand analysis and pattern matching
 */

#include "operand_analyzer.h"
#include "common/utils.h"

namespace z80 {

OperandAnalyzer::OperandAnalyzer(const Z80Instructions& instructions,
                                 const SymbolTable& symbolTable)
    : instructions_(instructions)
    , symbolTable_(symbolTable) {
}

std::string OperandAnalyzer::parseOperands(Lexer& lexer, std::vector<std::string>& operands) {
    std::string currentOperand;
    int parenDepth = 0;
    TokenType lastTokenType = TokenType::EndOfFile;
    
    Token token = lexer.nextToken();
    
    while (token.type != TokenType::EndOfLine && token.type != TokenType::EndOfFile) {
        if (token.type == TokenType::Comma && parenDepth == 0) {
            // End of current operand
            if (!currentOperand.empty()) {
                operands.push_back(currentOperand);
                currentOperand.clear();
            }
            lastTokenType = TokenType::Comma;
        } else {
            // Determine if we need space before this token
            bool needSpace = false;
            if (!currentOperand.empty() && lastTokenType != TokenType::LeftParen && 
                lastTokenType != TokenType::LeftBracket && token.type != TokenType::RightParen && 
                token.type != TokenType::RightBracket && token.type != TokenType::Comma) {
                // Add space between alphanumeric tokens (identifiers, numbers, and strings)
                if ((lastTokenType == TokenType::Identifier || lastTokenType == TokenType::Number) &&
                    (token.type == TokenType::Identifier || token.type == TokenType::Number || token.type == TokenType::String)) {
                    needSpace = true;
                }
                // Also add space before string if last was not comma or paren
                else if (token.type == TokenType::String && lastTokenType != TokenType::Comma) {
                    needSpace = true;
                }
            }
            
            if (needSpace) {
                currentOperand += " ";
            }
            
            // Part of current operand
            if (token.type == TokenType::LeftParen || token.type == TokenType::LeftBracket) {
                parenDepth++;
                currentOperand += token.text;
            } else if (token.type == TokenType::RightParen || token.type == TokenType::RightBracket) {
                parenDepth--;
                currentOperand += token.text;
            } else if (token.type == TokenType::String) {
                // Preserve string with quotes for later detection
                currentOperand += "'" + token.text + "'";
            } else {
                currentOperand += token.text;
            }
            
            lastTokenType = token.type;
        }
        
        token = lexer.nextToken();
    }
    
    // Add last operand
    if (!currentOperand.empty()) {
        operands.push_back(currentOperand);
    }
    
    // Reconstruct full operand string from parsed operands
    std::string fullOperandString;
    for (size_t i = 0; i < operands.size(); i++) {
        if (i > 0) fullOperandString += ",";
        fullOperandString += operands[i];
    }
    
    return fullOperandString;
}

const InstructionInfo* OperandAnalyzer::findInstructionVariant(const std::string& mnemonic,
                                                                 const std::vector<std::string>& operands) {
    // Build pattern from operands
    std::string pattern;
    for (size_t i = 0; i < operands.size(); i++) {
        if (i > 0) pattern += ",";
        pattern += operandToPattern(operands[i], mnemonic);
    }
    
    // Try to find instruction with this pattern
    auto* inst = instructions_.findInstruction(mnemonic, pattern);
    if (inst) return inst;
    
    // If not found and pattern contains NN (16-bit), try N (8-bit) instead
    // This handles cases like LD A,symbol where symbol value fits in 8 bits
    if (pattern.find(",NN") != std::string::npos || pattern == "NN") {
        std::string pattern8 = pattern;
        // Replace NN with N
        size_t pos = 0;
        while ((pos = pattern8.find("NN", pos)) != std::string::npos) {
            pattern8.replace(pos, 2, "N");
            pos += 1;
        }
        inst = instructions_.findInstruction(mnemonic, pattern8);
        if (inst) return inst;
    }
    
    // If not found and pattern contains N (8-bit), try NN (16-bit) instead
    // This handles cases like CALL 5 where 5 is 8-bit but needs 16-bit address
    if (pattern.find(",N") != std::string::npos || pattern == "N") {
        std::string pattern16 = pattern;
        // Replace all standalone N with NN (but not in (NN) or other contexts)
        size_t pos = 0;
        while ((pos = pattern16.find(",N", pos)) != std::string::npos) {
            if (pos + 2 >= pattern16.length() || pattern16[pos + 2] != 'N') {
                pattern16.insert(pos + 1, "N");
                pos += 3;
            } else {
                pos += 2;
            }
        }
        // Check if pattern starts with N
        if (pattern16.length() >= 1 && pattern16[0] == 'N' && 
            (pattern16.length() == 1 || pattern16[1] != 'N')) {
            pattern16 = "N" + pattern16;
        }
        
        inst = instructions_.findInstruction(mnemonic, pattern16);
        if (inst) return inst;
    }
    
    return nullptr;
}

std::string OperandAnalyzer::operandToPattern(const std::string& operand, const std::string& mnemonic) {
    if (operand.empty()) {
        return "";
    }
    
    // Convert operand to uppercase for comparison
    std::string upper = operand;
    for (char& c : upper) c = std::toupper(c);
    
    // Convert mnemonic to uppercase for comparison
    std::string upperMnem = mnemonic;
    for (char& c : upperMnem) c = std::toupper(c);
    
    // Check for condition codes BEFORE registers (Z, C could be confused with registers)
    // Condition codes: Z, NZ, C, NC, P, M, PE, PO
    if (upper == "Z" || upper == "NZ" || upper == "C" || upper == "NC" ||
        upper == "P" || upper == "M" || upper == "PE" || upper == "PO") {
        return upper;
    }
    
    // Check if it's a register (single or pair)
    if (isRegisterOperand(upper)) {
        return upper;
    }
    
    // Check for indirect register addressing: (HL), (BC), (DE), (SP)
    if (upper == "(HL)" || upper == "(BC)" || upper == "(DE)" || upper == "(SP)") {
        return upper;
    }
    
    // Check for indexed addressing: (IX+d), (IX-d), (IY+d), (IY-d)
    // Also handle variations like (IX), (IY)
    if (upper.find("(IX") == 0) {
        if (upper == "(IX)") return "(IX)";
        return "(IX+D)";  // Covers both +d and -d
    }
    if (upper.find("(IY") == 0) {
        if (upper == "(IY)") return "(IY)";
        return "(IY+D)";
    }
    
    // Check for indirect addressing: (nn) or (expression)
    // But distinguish between 8-bit port addresses (N) and 16-bit addresses (NN)
    if (upper.length() > 2 && upper[0] == '(' && upper[upper.length()-1] == ')') {
        // Extract the value inside parentheses
        std::string innerValue = operand.substr(1, operand.length() - 2);
        
        // Try to parse as number to determine if it's 8-bit or 16-bit
        int base = 10;
        int64_t value = 0;
        bool isNumber = parseNumber(innerValue, value, base);
        
        // If not a direct number, try to resolve as symbol
        if (!isNumber && symbolTable_.hasSymbol(innerValue)) {
            const Symbol* sym = symbolTable_.getSymbol(innerValue);
            if (sym && sym->defined) {
                value = sym->value;
                isNumber = true;
            }
        }
        
        // Check if it's 8-bit port address - ONLY for IN/OUT instructions
        if (isNumber && value >= 0 && value <= 255 && 
            (upperMnem == "IN" || upperMnem == "OUT")) {
            // 8-bit port address - used for IN/OUT instructions
            return "(N)";
        }
        
        // Otherwise it's a 16-bit memory address
        return "(NN)";
    }
    
    // Check for immediate 8-bit vs 16-bit values
    // This is a simplified heuristic - real implementation needs expression evaluation
    
    // If operand contains arithmetic or is a symbol, assume 16-bit for now
    // Proper implementation would evaluate expressions in context
    if (upper.find('+') != std::string::npos || 
        upper.find('-') != std::string::npos ||
        upper.find('*') != std::string::npos) {
        // Expression - assume 16-bit
        return "NN";
    }
    
    // Try to parse as number
    int base = 10;
    int64_t value = 0;
    if (parseNumber(operand, value, base)) {
        // Special case: bit numbers 0-7 for BIT/SET/RES instructions only
        // Keep them as literal digits for these instructions
        if (value >= 0 && value <= 7 && operand.length() == 1 &&
            (upperMnem == "BIT" || upperMnem == "SET" || upperMnem == "RES")) {
            return operand;  // Return "0" through "7" literally for bit instructions
        }
        
        // Determine size based on value
        if (value >= -128 && value <= 255) {
            // 8-bit value - return N for immediate 8-bit
            return "N";
        }
        // 16-bit value
        return "NN";
    }
    
    // Unknown - likely a symbol or forward reference
    // For BIT/SET/RES instructions, try to evaluate symbol to check if it's a bit number
    if (upperMnem == "BIT" || upperMnem == "SET" || upperMnem == "RES") {
        // Try to resolve symbol
        if (symbolTable_.hasSymbol(operand)) {
            const Symbol* sym = symbolTable_.getSymbol(operand);
            if (sym && sym->defined) {
                value = sym->value;
                if (value >= 0 && value <= 7) {
                    // It's a bit number - return it as string
                    return std::to_string(value);
                }
            }
        }
    }
    
    // We need to try both patterns (N and NN) depending on context
    // For now, prefer NN (addresses are more common)
    return "NN";
}

bool OperandAnalyzer::isRegisterOperand(const std::string& operand) {
    std::string upper = operand;
    for (char& c : upper) c = std::toupper(c);
    return instructions_.isRegister(upper);
}

} // namespace z80
