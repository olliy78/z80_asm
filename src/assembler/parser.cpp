/**
 * @file parser.cpp
 * @brief Minimal parser implementation
 */

#include "parser.h"
#include "common/utils.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace z80 {

Parser::Parser() 
    : locationCounter_(0)
    , currentSegment_(SegmentType::CSEG)
    , csegOrigin_(0)
    , dsegOrigin_(0)
    , asegOrigin_(0)
{
}

bool Parser::assemble(const std::string& filename) {
    // Reset state
    lines_.clear();
    errors_.clear();
    symbolTable_ = SymbolTable();
    locationCounter_ = 0;
    
    // Read file
    std::ifstream file(filename);
    if (!file.is_open()) {
        AssemblyError err;
        err.level = ErrorLevel::Error;
        err.message = "Cannot open file";
        err.filename = filename;
        err.line = 0;
        err.column = 0;
        errors_.push_back(err);
        return false;
    }
    
    std::vector<std::string> sourceLines;
    std::string line;
    while (std::getline(file, line)) {
        sourceLines.push_back(line);
    }
    
    // Pass 1: Build symbol table
    if (!pass1(sourceLines, filename)) {
        return false;
    }
    
    // Pass 2: Generate code
    if (!pass2(sourceLines, filename)) {
        return false;
    }
    
    return errors_.empty();
}

bool Parser::pass1(const std::vector<std::string>& sourceLines, const std::string& filename) {
    locationCounter_ = 0;
    currentSegment_ = SegmentType::CSEG;
    
    for (size_t i = 0; i < sourceLines.size(); ++i) {
        const std::string& line = sourceLines[i];
        int lineNum = i + 1;
        
        Lexer lexer(line, filename);
        Token token = lexer.nextToken();
        
        // Skip empty lines and comments
        if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) {
            continue;
        }
        
        ParsedLine parsedLine;
        parsedLine.lineNumber = lineNum;
        parsedLine.address = locationCounter_;
        parsedLine.segment = currentSegment_;
        
        // Check for label
        std::string labelName;
        bool hasColon = false;
        if (token.type == TokenType::Identifier) {
            Token next = lexer.peekToken();
            if (next.type == TokenType::Colon) {
                // This is definitely a label (has colon)
                labelName = token.text;
                parsedLine.label = labelName;
                hasColon = true;
                lexer.nextToken();  // Skip colon
                token = lexer.nextToken();
            }
            else if (next.type == TokenType::Identifier) {
                // Could be "LABEL MNEM" or "MNEM OPERAND"
                // Check if first token is a known mnemonic
                if (instructions_.isMnemonic(token.text)) {
                    // It's an instruction, not a label
                    // Don't consume token, continue to mnemonic parsing
                } else {
                    // Assume it's a label followed by mnemonic
                    labelName = token.text;
                    parsedLine.label = labelName;
                    token = lexer.nextToken();
                }
            }
            else if (next.type == TokenType::EndOfLine || next.type == TokenType::EndOfFile) {
                // Just a standalone label
                labelName = token.text;
                parsedLine.label = labelName;
                token = lexer.nextToken();
            }
        }
        
        // Parse mnemonic/directive
        if (token.type == TokenType::Identifier) {
            parsedLine.mnemonic = token.text;
            
            // Check if it's a directive
            std::string upperMnemonic = token.text;
            for (char& c : upperMnemonic) c = std::toupper(c);
            
            if (upperMnemonic == "ORG") {
                token = lexer.nextToken();
                if (token.type == TokenType::Number) {
                    locationCounter_ = static_cast<Address>(token.numValue);
                    parsedLine.address = locationCounter_;
                }
            }
            else if (upperMnemonic == "EQU") {
                // EQU: symbol definition only, no label
                token = lexer.nextToken();
                if (token.type == TokenType::Number && !labelName.empty()) {
                    Symbol sym;
                    sym.type = SymbolType::Equ;
                    sym.value = token.numValue;
                    sym.defined = true;
                    sym.segment = currentSegment_;
                    sym.definedLine = lineNum;
                    symbolTable_.addSymbol(labelName, sym);
                }
            }
            else {
                // For all other directives and instructions, add label if present
                if (!labelName.empty()) {
                    Symbol sym;
                    sym.type = SymbolType::Label;
                    sym.value = locationCounter_;
                    sym.segment = currentSegment_;
                    sym.defined = true;
                    sym.isRelocatable = (currentSegment_ != SegmentType::ASEG);
                    sym.definedLine = lineNum;
                    symbolTable_.addSymbol(labelName, sym);
                }
                
                // Continue processing directive/instruction
                if (upperMnemonic == "DB") {
                    // Count bytes
                    int count = 0;
                    token = lexer.nextToken();
                    while (token.type != TokenType::EndOfLine && token.type != TokenType::EndOfFile) {
                        if (token.type == TokenType::Number) {
                            count++;
                        } else if (token.type == TokenType::String) {
                            count += token.text.length();
                        }
                        token = lexer.nextToken();
                        if (token.type == TokenType::Comma) {
                            token = lexer.nextToken();
                        }
                    }
                    locationCounter_ += count;
                }
                else if (upperMnemonic == "DW") {
                    // Count words
                    int count = 0;
                    token = lexer.nextToken();
                    while (token.type != TokenType::EndOfLine && token.type != TokenType::EndOfFile) {
                        if (token.type == TokenType::Number || token.type == TokenType::Identifier) {
                            count++;
                        }
                        token = lexer.nextToken();
                        if (token.type == TokenType::Comma) {
                            token = lexer.nextToken();
                        }
                    }
                    locationCounter_ += count * 2;
                }
                else if (upperMnemonic == "DS") {
                    // Reserve space
                    token = lexer.nextToken();
                    if (token.type == TokenType::Number) {
                        locationCounter_ += static_cast<Address>(token.numValue);
                    }
                }
                else if (upperMnemonic == "CSEG") {
                    currentSegment_ = SegmentType::CSEG;
                    locationCounter_ = csegOrigin_;
                }
                else if (upperMnemonic == "DSEG") {
                    currentSegment_ = SegmentType::DSEG;
                    locationCounter_ = dsegOrigin_;
                }
                else if (upperMnemonic == "ASEG") {
                    currentSegment_ = SegmentType::ASEG;
                    locationCounter_ = asegOrigin_;
                }
                else if (upperMnemonic == "END") {
                    // Stop parsing
                    break;
                }
                else {
                    // It's an instruction - parse operands
                    parsedLine.operandString = parseOperands(lexer, parsedLine.operands);
                    
                    // Find matching instruction variant
                    auto info = findInstructionVariant(parsedLine.mnemonic, parsedLine.operands);
                    if (info) {
                        locationCounter_ += info->opcodes.size() + info->operandBytes;
                    } else {
                        // Unknown instruction - might be acceptable in pass2
                        // For now, just estimate 3 bytes
                        locationCounter_ += 3;
                    }
                }
            }
        }
        
        lines_.push_back(parsedLine);
    }
    
    return true;
}

bool Parser::pass2(const std::vector<std::string>& sourceLines, const std::string& filename) {
    // Pass 2: Generate actual machine code for each line
    locationCounter_ = 0;
    currentSegment_ = SegmentType::CSEG;
    
    for (auto& line : lines_) {
        // Skip lines without mnemonics (just labels or comments)
        if (line.mnemonic.empty()) {
            continue;
        }
        
        std::string upperMnemonic = line.mnemonic;
        for (char& c : upperMnemonic) c = std::toupper(c);
        
        // Handle directives
        if (upperMnemonic == "ORG") {
            // ORG handled in pass 1
            continue;
        }
        else if (upperMnemonic == "EQU") {
            // EQU handled in pass 1
            continue;
        }
        else if (upperMnemonic == "DB") {
            // Generate DB bytes
            if (!generateDB(line, filename)) {
                return false;
            }
        }
        else if (upperMnemonic == "DW") {
            // Generate DW words
            if (!generateDW(line, filename)) {
                return false;
            }
        }
        else if (upperMnemonic == "DS") {
            // DS reserves space - no code generated
            continue;
        }
        else if (upperMnemonic == "CSEG" || upperMnemonic == "DSEG" || upperMnemonic == "ASEG") {
            // Segment switches handled in pass 1
            continue;
        }
        else if (upperMnemonic == "END") {
            // Stop processing
            break;
        }
        else {
            // It's an instruction - generate code
            if (!generateInstruction(line, filename)) {
                return false;
            }
        }
    }
    
    return true;
}

bool Parser::generateDB(ParsedLine& line, const std::string& filename) {
    // Parse and generate bytes for DB directive
    // For now, we need to re-parse the source line to get operands
    // TODO: Store operands in ParsedLine during pass 1
    
    // Simple stub for now - just mark as processed
    return true;
}

bool Parser::generateDW(ParsedLine& line, const std::string& filename) {
    // Parse and generate words for DW directive
    // TODO: Implement word generation
    return true;
}

bool Parser::generateInstruction(ParsedLine& line, const std::string& filename) {
    // Find instruction variant based on parsed operands
    auto info = findInstructionVariant(line.mnemonic, line.operands);
    if (info) {
        // Copy opcodes to code
        for (Byte b : info->opcodes) {
            line.code.push_back(b);
        }
        
        // TODO: Add operand bytes if needed (need to evaluate expressions)
        for (int i = 0; i < info->operandBytes; i++) {
            line.code.push_back(0x00);  // Placeholder
        }
        
        return true;
    }
    
    // Instruction not found - this is an error
    AssemblyError err;
    err.level = ErrorLevel::Error;
    err.message = "Unknown instruction: " + line.mnemonic + " " + line.operandString;
    err.filename = filename;
    err.line = line.lineNumber;
    err.column = 0;
    errors_.push_back(err);
    
    return false;
}

std::string Parser::parseOperands(Lexer& lexer, std::vector<std::string>& operands) {
    std::string currentOperand;
    int parenDepth = 0;
    
    Token token = lexer.nextToken();
    
    while (token.type != TokenType::EndOfLine && token.type != TokenType::EndOfFile) {
        if (token.type == TokenType::Comma && parenDepth == 0) {
            // End of current operand
            if (!currentOperand.empty()) {
                operands.push_back(currentOperand);
                currentOperand.clear();
            }
        } else {
            // Part of current operand
            if (token.type == TokenType::LeftParen || token.type == TokenType::LeftBracket) {
                parenDepth++;
                currentOperand += token.text;
            } else if (token.type == TokenType::RightParen || token.type == TokenType::RightBracket) {
                parenDepth--;
                currentOperand += token.text;
            } else {
                currentOperand += token.text;
            }
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

const InstructionInfo* Parser::findInstructionVariant(const std::string& mnemonic,
                                                       const std::vector<std::string>& operands) {
    // Build pattern from operands
    std::string pattern;
    for (size_t i = 0; i < operands.size(); i++) {
        if (i > 0) pattern += ",";
        pattern += operandToPattern(operands[i]);
    }
    
    return instructions_.findInstruction(mnemonic, pattern);
}

std::string Parser::operandToPattern(const std::string& operand) {
    if (operand.empty()) {
        return "";
    }
    
    // Convert operand to uppercase for comparison
    std::string upper = operand;
    for (char& c : upper) c = std::toupper(c);
    
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
    if (upper.length() > 2 && upper[0] == '(' && upper[upper.length()-1] == ')') {
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
        // Determine size based on value
        if (value >= -128 && value <= 255) {
            // Could be 8-bit, but context matters
            // For now, return NN and let instruction matching decide
            // A better approach would be to try both N and NN patterns
            return "NN";  // Conservative: assume 16-bit
        }
        return "NN";
    }
    
    // Unknown - likely a symbol or forward reference
    // Assume 16-bit address
    return "NN";
}

bool Parser::isRegisterOperand(const std::string& operand) {
    std::string upper = operand;
    for (char& c : upper) c = std::toupper(c);
    return instructions_.isRegister(upper);
}

const std::vector<AssemblyError>& Parser::getErrors() const {
    return errors_;
}

const SymbolTable& Parser::getSymbolTable() const {
    return symbolTable_;
}

} // namespace z80
