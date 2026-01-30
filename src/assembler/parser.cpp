/**
 * @file parser.cpp
 * @brief Minimal parser implementation
 */

#include "parser.h"
#include "expression.h"
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
    , moduleName_("")
    , inPhase_(false)
    , phaseOrigin_(0)
    , phaseOffset_(0)
    , inMacroDefinition_(false)
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
    inMacroDefinition_ = false;
    macroBody_.clear();
    
    // Process source lines (including macro expansion)
    std::vector<std::string> expandedLines;
    expandSourceWithMacros(sourceLines, expandedLines, filename);
    
    for (size_t i = 0; i < expandedLines.size(); ++i) {
        const std::string& line = expandedLines[i];
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
                // Check if first token is a known mnemonic or directive
                std::string upperToken = token.text;
                for (char& c : upperToken) c = std::toupper(c);
                bool isDirective = (upperToken == "ORG" || upperToken == "EQU" || 
                                   upperToken == "DB" || upperToken == "DW" || 
                                   upperToken == "DS" || upperToken == "END" ||
                                   upperToken == "PUBLIC" || upperToken == "EXTRN" ||
                                   upperToken == "ENTRY" || upperToken == "EXT" ||
                                   upperToken == "NAME" || upperToken == "TITLE" ||
                                   upperToken == "PHASE" || upperToken == "DEPHASE" ||
                                   upperToken == ".PHASE" || upperToken == ".DEPHASE" ||
                                   upperToken == "CSEG" || upperToken == "DSEG" || upperToken == "ASEG");
                
                if (instructions_.isMnemonic(token.text) || isDirective) {
                    // It's an instruction or directive, not a label
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
            else if (upperMnemonic == "PUBLIC" || upperMnemonic == "ENTRY") {
                // PUBLIC/ENTRY: Mark symbols as exported
                // Format: PUBLIC symbol1, symbol2, ...
                token = lexer.nextToken();
                while (token.type == TokenType::Identifier) {
                    std::string symbolName = token.text;
                    // Mark symbol as public (create if doesn't exist yet)
                    Symbol* sym = symbolTable_.getSymbol(symbolName);
                    if (sym) {
                        sym->isPublic = true;
                    } else {
                        // Create forward reference
                        Symbol newSym;
                        newSym.type = SymbolType::Label;
                        newSym.value = 0;
                        newSym.defined = false;
                        newSym.isPublic = true;
                        newSym.segment = currentSegment_;
                        newSym.definedLine = lineNum;
                        symbolTable_.addSymbol(symbolName, newSym);
                    }
                    
                    token = lexer.nextToken();
                    if (token.type == TokenType::Comma) {
                        token = lexer.nextToken();  // Skip comma
                    } else {
                        break;
                    }
                }
            }
            else if (upperMnemonic == "EXTRN" || upperMnemonic == "EXT") {
                // EXTRN/EXT: Mark symbols as external (imported)
                // Format: EXTRN symbol1, symbol2, ...
                token = lexer.nextToken();
                while (token.type == TokenType::Identifier) {
                    std::string symbolName = token.text;
                    // Create external symbol
                    Symbol sym;
                    sym.type = SymbolType::Label;
                    sym.value = 0;
                    sym.defined = false;
                    sym.isExternal = true;
                    sym.segment = SegmentType::ASEG;  // External symbols are absolute
                    sym.definedLine = lineNum;
                    symbolTable_.addSymbol(symbolName, sym);
                    
                    token = lexer.nextToken();
                    if (token.type == TokenType::Comma) {
                        token = lexer.nextToken();  // Skip comma
                    } else {
                        break;
                    }
                }
            }
            else {
                // For all other directives and instructions, add label if present
                if (!labelName.empty()) {
                    Symbol sym;
                    sym.type = SymbolType::Label;
                    // Use phased address if in PHASE block
                    sym.value = locationCounter_ + (inPhase_ ? phaseOffset_ : 0);
                    sym.segment = currentSegment_;
                    sym.defined = true;
                    sym.isRelocatable = (currentSegment_ != SegmentType::ASEG);
                    sym.definedLine = lineNum;
                    symbolTable_.addSymbol(labelName, sym);
                }
                
                // Continue processing directive/instruction
                if (upperMnemonic == "DB") {
                    // Parse operands for later use in pass 2
                    parsedLine.operandString = parseOperands(lexer, parsedLine.operands);
                    // Count bytes - each operand becomes a byte
                    locationCounter_ += parsedLine.operands.size();
                }
                else if (upperMnemonic == "DW") {
                    // Parse operands for later use in pass 2
                    parsedLine.operandString = parseOperands(lexer, parsedLine.operands);
                    // Count words - each operand becomes 2 bytes
                    locationCounter_ += parsedLine.operands.size() * 2;
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
                else if (upperMnemonic == "NAME" || upperMnemonic == "TITLE") {
                    // NAME/TITLE: Set module name
                    token = lexer.nextToken();
                    if (token.type == TokenType::Identifier || token.type == TokenType::String) {
                        moduleName_ = token.text;
                    }
                }
                else if (upperMnemonic == ".Z80") {
                    // .Z80: Enable Z80 instruction set (default, ignore)
                }
                else if (upperMnemonic == ".8080") {
                    // .8080: Enable 8080 instruction set (not supported, warn?)
                }
                else if (upperMnemonic == ".LIST" || upperMnemonic == ".XLIST") {
                    // Listing control directives (ignore for now)
                }
                else if (upperMnemonic == ".TFCOND" || upperMnemonic == ".SFCOND" || upperMnemonic == ".LFCOND") {
                    // Conditional listing directives (ignore for now)
                }
                else if (upperMnemonic == ".PHASE" || upperMnemonic == "PHASE") {
                    // .PHASE: Set phase offset for relocatable code
                    // Code is assembled at current location but symbols use phase address
                    token = lexer.nextToken();
                    if (token.type == TokenType::Number || token.type == TokenType::Identifier) {
                        // Evaluate expression for phase address
                        int64_t phaseAddr = 0;
                        if (token.type == TokenType::Number) {
                            phaseAddr = token.numValue;
                        } else {
                            // Try to resolve symbol
                            const Symbol* sym = symbolTable_.getSymbol(token.text);
                            if (sym && sym->defined) {
                                phaseAddr = sym->value;
                            }
                        }
                        phaseOrigin_ = locationCounter_;
                        phaseOffset_ = phaseAddr - locationCounter_;
                        inPhase_ = true;
                    }
                }
                else if (upperMnemonic == ".DEPHASE" || upperMnemonic == "DEPHASE") {
                    // .DEPHASE: End phase block
                    inPhase_ = false;
                    phaseOffset_ = 0;
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
    
    // Re-expand source with macros (we need to do this again for pass2)
    std::vector<std::string> expandedLines;
    macroProcessor_.clear(); // Reset macro state
    expandSourceWithMacros(sourceLines, expandedLines, filename);
    
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
        else if (upperMnemonic == "NAME" || upperMnemonic == "TITLE") {
            // NAME/TITLE handled in pass 1
            continue;
        }
        else if (upperMnemonic == "PUBLIC" || upperMnemonic == "ENTRY" || 
                 upperMnemonic == "EXTRN" || upperMnemonic == "EXT") {
            // Symbol declarations handled in pass 1
            continue;
        }
        else if (upperMnemonic == "PHASE" || upperMnemonic == "DEPHASE" ||
                 upperMnemonic == ".PHASE" || upperMnemonic == ".DEPHASE") {
            // PHASE handled in pass 1
            continue;
        }
        else if (upperMnemonic == ".Z80" || upperMnemonic == ".8080" ||
                 upperMnemonic == ".LIST" || upperMnemonic == ".XLIST" ||
                 upperMnemonic == ".TFCOND" || upperMnemonic == ".SFCOND" || upperMnemonic == ".LFCOND") {
            // Assembler directives - ignore
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
    // DB generates bytes from comma-separated expressions
    // Operands are already parsed in line.operands
    
    ExpressionEvaluator eval(symbolTable_, locationCounter_, currentSegment_);
    
    for (const std::string& operand : line.operands) {
        // Check if operand is a string literal (enclosed in quotes)
        if ((operand.length() >= 2 && operand[0] == '\'' && operand[operand.length()-1] == '\'') ||
            (operand.length() >= 2 && operand[0] == '"' && operand[operand.length()-1] == '"')) {
            // String literal - add each character as a byte
            for (size_t i = 1; i < operand.length() - 1; i++) {
                line.code.push_back(static_cast<Byte>(operand[i]));
            }
            continue;
        }
        
        // Otherwise, evaluate as expression
        ExpressionResult result = eval.evaluate(operand);
        
        if (!result.valid) {
            AssemblyError err;
            err.level = ErrorLevel::Error;
            err.message = "Error evaluating DB expression '" + operand + "': " + result.errorMessage;
            err.filename = filename;
            err.line = line.lineNumber;
            errors_.push_back(err);
            return false;
        }
        
        // Store byte value (truncate to 8 bits)
        line.code.push_back(static_cast<Byte>(result.value & 0xFF));
    }
    
    return true;
}

bool Parser::generateDW(ParsedLine& line, const std::string& filename) {
    // DW generates 16-bit words from comma-separated expressions
    // Z80 uses little-endian (LSB first)
    
    ExpressionEvaluator eval(symbolTable_, locationCounter_, currentSegment_);
    
    for (const std::string& operand : line.operands) {
        ExpressionResult result = eval.evaluate(operand);
        
        if (!result.valid) {
            AssemblyError err;
            err.level = ErrorLevel::Error;
            err.message = "Error evaluating DW expression '" + operand + "': " + result.errorMessage;
            err.filename = filename;
            err.line = line.lineNumber;
            errors_.push_back(err);
            return false;
        }
        
        // Store word value in little-endian (LSB, MSB)
        line.code.push_back(static_cast<Byte>(result.value & 0xFF));        // LSB
        line.code.push_back(static_cast<Byte>((result.value >> 8) & 0xFF)); // MSB
    }
    
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
        
        // Add operand bytes if needed (evaluate expressions)
        if (info->operandBytes > 0) {
            // Find the operand that contains an immediate value or address
            std::string valueOperand;
            
            for (const std::string& op : line.operands) {
                std::string upper = op;
                for (char& c : upper) c = std::toupper(c);
                
                // Skip condition codes
                if (upper == "Z" || upper == "NZ" || upper == "C" || upper == "NC" ||
                    upper == "P" || upper == "M" || upper == "PE" || upper == "PO") {
                    continue;
                }
                
                // Skip register operands
                if (isRegisterOperand(upper)) continue;
                
                // Handle indirect addressing - extract the expression inside ()
                if (!op.empty() && op[0] == '(' && op[op.length()-1] == ')') {
                    // Extract expression between parentheses
                    valueOperand = op.substr(1, op.length() - 2);
                    break;
                }
                
                // Check for indexed addressing (IX+d) or (IY+d)
                if (upper.find("(IX+") == 0 || upper.find("(IX-") == 0 ||
                    upper.find("(IY+") == 0 || upper.find("(IY-") == 0) {
                    // Extract displacement - everything after IX or IY until )
                    size_t startPos = upper.find('+');
                    if (startPos == std::string::npos) startPos = upper.find('-');
                    if (startPos != std::string::npos) {
                        size_t endPos = op.find(')');
                        valueOperand = op.substr(startPos, endPos - startPos);
                        break;
                    }
                }
                
                // Otherwise it's a direct value/address
                valueOperand = op;
                break;
            }
            
            // Evaluate the operand expression
            if (!valueOperand.empty()) {
                ExpressionEvaluator eval(symbolTable_, locationCounter_, currentSegment_);
                ExpressionResult result = eval.evaluate(valueOperand);
                
                if (!result.valid) {
                    AssemblyError err;
                    err.level = ErrorLevel::Error;
                    err.message = "Error evaluating operand '" + valueOperand + "': " + result.errorMessage;
                    err.filename = filename;
                    err.line = line.lineNumber;
                    errors_.push_back(err);
                    return false;
                }
                
                // Store operand bytes
                if (info->operandBytes == 1) {
                    // 8-bit immediate or displacement
                    Byte byteVal = static_cast<Byte>(result.value & 0xFF);
                    
                    // Special handling for JR instruction (relative addressing)
                    if (line.mnemonic == "JR" || line.mnemonic == "jr") {
                        // Calculate relative offset: target - (PC after instruction)
                        // PC after JR instruction = current address + 2
                        int32_t offset = result.value - (line.address + 2);
                        
                        // Check if offset is in range for relative jump (-128 to +127)
                        if (offset < -128 || offset > 127) {
                            AssemblyError err;
                            err.level = ErrorLevel::Error;
                            err.message = "JR offset out of range (" + std::to_string(offset) + 
                                        " bytes, must be -128 to +127)";
                            err.filename = filename;
                            err.line = line.lineNumber;
                            errors_.push_back(err);
                            return false;
                        }
                        
                        byteVal = static_cast<Byte>(offset & 0xFF);
                    }
                    
                    line.code.push_back(byteVal);
                } else if (info->operandBytes == 2) {
                    // 16-bit address in little-endian
                    line.code.push_back(static_cast<Byte>(result.value & 0xFF));
                    line.code.push_back(static_cast<Byte>((result.value >> 8) & 0xFF));
                }
                
                // TODO: Track relocatable references for REL output
                if (result.type != ExpressionType::Absolute) {
                    // This address needs relocation - store for later REL generation
                }
            } else {
                // No operand found but instruction expects bytes - use zeros
                for (int i = 0; i < info->operandBytes; i++) {
                    line.code.push_back(0x00);
                }
            }
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
            } else if (token.type == TokenType::String) {
                // Preserve string with quotes for later detection
                currentOperand += "'" + token.text + "'";
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
    
    // Try to find instruction with this pattern
    auto* inst = instructions_.findInstruction(mnemonic, pattern);
    if (inst) return inst;
    
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

std::string Parser::operandToPattern(const std::string& operand) {
    if (operand.empty()) {
        return "";
    }
    
    // Convert operand to uppercase for comparison
    std::string upper = operand;
    for (char& c : upper) c = std::toupper(c);
    
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
        if (parseNumber(innerValue, value, base) && value >= 0 && value <= 255) {
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
        // Special case: bit numbers 0-7 for BIT/SET/RES instructions
        // Keep them as literal digits
        if (value >= 0 && value <= 7 && operand.length() == 1) {
            return operand;  // Return "0" through "7" literally
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
    // We need to try both patterns (N and NN) depending on context
    // For now, prefer NN (addresses are more common)
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

bool Parser::writeREL(const std::string& filename, const std::string& moduleName) {
    RELWriter writer;
    
    // Derive module name from filename if not provided
    std::string modName = moduleName;
    if (modName.empty()) {
        // Extract basename without extension
        size_t lastSlash = filename.find_last_of("/\\");
        size_t lastDot = filename.find_last_of('.');
        size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
        size_t end = (lastDot == std::string::npos) ? filename.length() : lastDot;
        modName = filename.substr(start, end - start);
    }
    
    // Determine if module is relocatable (has CSEG or DSEG code)
    bool isRelocatable = false;
    for (const auto& line : lines_) {
        if (!line.code.empty()) {
            if (line.segment == SegmentType::CSEG || line.segment == SegmentType::DSEG) {
                isRelocatable = true;
                break;
            }
        }
    }
    
    // Begin module
    writer.beginModule(modName, isRelocatable);
    
    // Calculate segment sizes
    Address csegSize = 0, dsegSize = 0;
    for (const auto& line : lines_) {
        if (!line.code.empty()) {
            if (line.segment == SegmentType::CSEG) {
                Address endAddr = line.address + line.code.size();
                if (endAddr > csegSize) csegSize = endAddr;
            } else if (line.segment == SegmentType::DSEG) {
                Address endAddr = line.address + line.code.size();
                if (endAddr > dsegSize) csegSize = endAddr;
            }
        }
    }
    
    // Write sizes
    if (csegSize > 0) {
        writer.writeProgramSize(csegSize);
    }
    if (dsegSize > 0) {
        writer.writeDataSize(dsegSize);
    }
    
    // Write code/data by segment
    // Group consecutive bytes by segment
    std::vector<Byte> currentData;
    SegmentType currentSegType = SegmentType::CSEG;
    Address currentAddr = 0;
    bool hasData = false;
    
    for (const auto& line : lines_) {
        if (line.code.empty()) continue;
        
        // If segment changed or address is not continuous, flush current data
        if (hasData && (line.segment != currentSegType || line.address != currentAddr)) {
            // Write accumulated data
            if (currentSegType == SegmentType::ASEG) {
                writer.writeAbsoluteData(currentData);
            } else if (currentSegType == SegmentType::CSEG) {
                writer.writeProgramData(currentData);
            } else if (currentSegType == SegmentType::DSEG) {
                writer.writeDataData(currentData);
            }
            currentData.clear();
            hasData = false;
        }
        
        // If starting new segment, set location
        if (!hasData) {
            ItemType locType = ItemType::Absolute;
            if (line.segment == SegmentType::CSEG) {
                locType = ItemType::ProgramRel;
            } else if (line.segment == SegmentType::DSEG) {
                locType = ItemType::DataRel;
            }
            writer.setLocation(line.address, locType);
            currentSegType = line.segment;
            currentAddr = line.address;
        }
        
        // Accumulate bytes
        for (Byte b : line.code) {
            currentData.push_back(b);
        }
        currentAddr += line.code.size();
        hasData = true;
    }
    
    // Flush remaining data
    if (hasData) {
        if (currentSegType == SegmentType::ASEG) {
            writer.writeAbsoluteData(currentData);
        } else if (currentSegType == SegmentType::CSEG) {
            writer.writeProgramData(currentData);
        } else if (currentSegType == SegmentType::DSEG) {
            writer.writeDataData(currentData);
        }
    }
    
    // End module and file
    writer.endModule();
    writer.endFile();
    
    // Write to file
    return writer.writeToFile(filename);
}

bool Parser::expandSourceWithMacros(const std::vector<std::string>& sourceLines,
                                    std::vector<std::string>& expandedLines,
                                    const std::string& filename) {
    bool inMacroDef = false;
    bool inReptDef = false;
    bool inIRPDef = false;
    bool inIRPCDef = false;
    
    MacroDefinition currentMacro;
    std::vector<std::string> currentBody;
    std::vector<std::string> localLabels;
    int repeatCount = 0;
    std::string iteratorName;
    std::vector<std::string> iteratorValues;
    std::string irpcChars;
    
    for (size_t lineIdx = 0; lineIdx < sourceLines.size(); ++lineIdx) {
        const std::string& line = sourceLines[lineIdx];
        
        // If we're expanding macros, get lines from macro processor
        while (macroProcessor_.isExpanding()) {
            std::string expandedLine;
            if (macroProcessor_.getNextLine(expandedLine)) {
                expandedLines.push_back(expandedLine);
            } else {
                break; // Expansion complete
            }
        }
        
        Lexer lexer(line, filename);
        Token token = lexer.nextToken();
        
        // Skip empty lines and comments
        if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) {
            if (!inMacroDef && !inReptDef && !inIRPDef && !inIRPCDef) {
                expandedLines.push_back(line);
            }
            continue;
        }
        
        // Check for label
        std::string labelName;
        if (token.type == TokenType::Identifier) {
            Token next = lexer.peekToken();
            if (next.type == TokenType::Colon) {
                labelName = token.text;
                lexer.nextToken(); // Skip colon
                token = lexer.nextToken();
            } else {
                // Check if first token is a known directive/mnemonic
                std::string upperToken = token.text;
                for (char& c : upperToken) c = std::toupper(c);
                bool isDirective = (upperToken == "ORG" || upperToken == "EQU" || 
                                   upperToken == "DB" || upperToken == "DW" || 
                                   upperToken == "DS" || upperToken == "END" ||
                                   upperToken == "PUBLIC" || upperToken == "EXTRN" ||
                                   upperToken == "ENTRY" || upperToken == "EXT" ||
                                   upperToken == "NAME" || upperToken == "TITLE" ||
                                   upperToken == "CSEG" || upperToken == "DSEG" || 
                                   upperToken == "ASEG" || upperToken == "PHASE" ||
                                   upperToken == "DEPHASE" || upperToken == ".PHASE" ||
                                   upperToken == ".DEPHASE" || upperToken == ".Z80" ||
                                   upperToken == ".8080" || upperToken == ".LIST" ||
                                   upperToken == ".XLIST" || upperToken == ".TFCOND" ||
                                   upperToken == ".SFCOND" || upperToken == ".LFCOND" ||
                                   upperToken == "MACRO" || upperToken == "REPT" ||
                                   upperToken == "IRP" || upperToken == "IRPC" ||
                                   upperToken == "ENDM" || upperToken == "LOCAL" ||
                                   upperToken == "EXITM");
                
                if (!isDirective && next.type == TokenType::Identifier) {
                    // Assume it's a label without colon
                    labelName = token.text;
                    token = lexer.nextToken();
                }
            }
        }
        
        // Check for directives
        if (token.type == TokenType::Identifier) {
            std::string upperMnemonic = token.text;
            for (char& c : upperMnemonic) c = std::toupper(c);
            
            // MACRO definition
            if (upperMnemonic == "MACRO" && !labelName.empty()) {
                inMacroDef = true;
                currentMacro = MacroDefinition();
                currentMacro.type = MacroType::UserDefined;
                currentMacro.name = labelName;
                currentMacro.definitionLine = lineIdx + 1;
                currentMacro.definitionFile = filename;
                currentBody.clear();
                localLabels.clear();
                
                // Parse parameters
                token = lexer.nextToken();
                while (token.type == TokenType::Identifier) {
                    currentMacro.parameters.push_back(token.text);
                    token = lexer.nextToken();
                    if (token.type == TokenType::Comma) {
                        token = lexer.nextToken();
                    }
                }
                continue;
            }
            
            // REPT definition
            if (upperMnemonic == "REPT") {
                inReptDef = true;
                currentBody.clear();
                localLabels.clear();
                
                // Parse repeat count
                token = lexer.nextToken();
                if (token.type == TokenType::Number) {
                    repeatCount = static_cast<int>(token.numValue);
                }
                continue;
            }
            
            // IRP definition
            if (upperMnemonic == "IRP") {
                inIRPDef = true;
                currentBody.clear();
                localLabels.clear();
                iteratorValues.clear();
                
                // Parse iterator name
                token = lexer.nextToken();
                if (token.type == TokenType::Identifier) {
                    iteratorName = token.text;
                }
                
                // Parse argument list in <...>
                token = lexer.nextToken();
                if (token.type == TokenType::LessThan || line.find('<') != std::string::npos) {
                    // Find content between < and >
                    size_t start = line.find('<');
                    size_t end = line.find('>');
                    if (start != std::string::npos && end != std::string::npos && end > start) {
                        std::string argList = line.substr(start + 1, end - start - 1);
                        
                        // Parse comma-separated values
                        std::istringstream iss(argList);
                        std::string value;
                        while (std::getline(iss, value, ',')) {
                            // Trim whitespace
                            size_t first = value.find_first_not_of(" \t");
                            size_t last = value.find_last_not_of(" \t");
                            if (first != std::string::npos) {
                                value = value.substr(first, last - first + 1);
                                if (!value.empty()) {
                                    iteratorValues.push_back(value);
                                }
                            }
                        }
                    }
                }
                continue;
            }
            
            // IRPC definition
            if (upperMnemonic == "IRPC") {
                inIRPCDef = true;
                currentBody.clear();
                localLabels.clear();
                
                // Parse iterator name
                token = lexer.nextToken();
                if (token.type == TokenType::Identifier) {
                    iteratorName = token.text;
                }
                
                // Parse character string
                token = lexer.nextToken();
                if (token.type == TokenType::String || token.type == TokenType::Identifier) {
                    irpcChars = token.text;
                }
                continue;
            }
            
            // LOCAL declaration
            if (upperMnemonic == "LOCAL" && (inMacroDef || inReptDef || inIRPDef || inIRPCDef)) {
                // Parse local labels
                token = lexer.nextToken();
                while (token.type == TokenType::Identifier) {
                    localLabels.push_back(token.text);
                    token = lexer.nextToken();
                    if (token.type == TokenType::Comma) {
                        token = lexer.nextToken();
                    }
                }
                continue;
            }
            
            // EXITM
            if (upperMnemonic == "EXITM") {
                if (macroProcessor_.isExpanding()) {
                    macroProcessor_.exitMacro();
                }
                if (inMacroDef || inReptDef || inIRPDef || inIRPCDef) {
                    currentBody.push_back(line);
                }
                continue;
            }
            
            // ENDM - end macro/rept/irp/irpc
            if (upperMnemonic == "ENDM") {
                if (inMacroDef) {
                    currentMacro.body = currentBody;
                    currentMacro.localLabels = localLabels;
                    macroProcessor_.defineMacro(currentMacro);
                    inMacroDef = false;
                } else if (inReptDef) {
                    macroProcessor_.beginRepeat(repeatCount, currentBody, localLabels);
                    inReptDef = false;
                    
                    // Expand immediately
                    while (macroProcessor_.isExpanding()) {
                        std::string expandedLine;
                        if (macroProcessor_.getNextLine(expandedLine)) {
                            expandedLines.push_back(expandedLine);
                        } else {
                            break;
                        }
                    }
                } else if (inIRPDef) {
                    macroProcessor_.beginIRP(iteratorName, iteratorValues, currentBody, localLabels);
                    inIRPDef = false;
                    
                    // Expand immediately
                    while (macroProcessor_.isExpanding()) {
                        std::string expandedLine;
                        if (macroProcessor_.getNextLine(expandedLine)) {
                            expandedLines.push_back(expandedLine);
                        } else {
                            break;
                        }
                    }
                } else if (inIRPCDef) {
                    macroProcessor_.beginIRPC(iteratorName, irpcChars, currentBody, localLabels);
                    inIRPCDef = false;
                    
                    // Expand immediately
                    while (macroProcessor_.isExpanding()) {
                        std::string expandedLine;
                        if (macroProcessor_.getNextLine(expandedLine)) {
                            expandedLines.push_back(expandedLine);
                        } else {
                            break;
                        }
                    }
                }
                continue;
            }
            
            // Check if it's a macro invocation
            if (!inMacroDef && !inReptDef && !inIRPDef && !inIRPCDef) {
                // First check if it's a known directive or mnemonic
                bool isKnownDirective = (upperMnemonic == "ORG" || upperMnemonic == "EQU" || 
                                        upperMnemonic == "DB" || upperMnemonic == "DW" || 
                                        upperMnemonic == "DS" || upperMnemonic == "END" ||
                                        upperMnemonic == "PUBLIC" || upperMnemonic == "EXTRN" ||
                                        upperMnemonic == "ENTRY" || upperMnemonic == "EXT" ||
                                        upperMnemonic == "NAME" || upperMnemonic == "TITLE" ||
                                        upperMnemonic == "CSEG" || upperMnemonic == "DSEG" || 
                                        upperMnemonic == "ASEG" || upperMnemonic == "PHASE" ||
                                        upperMnemonic == "DEPHASE" || upperMnemonic == ".PHASE" ||
                                        upperMnemonic == ".DEPHASE" || upperMnemonic == ".Z80" ||
                                        upperMnemonic == ".8080" || upperMnemonic == ".LIST" ||
                                        upperMnemonic == ".XLIST" || upperMnemonic == ".TFCOND" ||
                                        upperMnemonic == ".SFCOND" || upperMnemonic == ".LFCOND");
                
                std::string checkName = labelName.empty() ? token.text : labelName;
                if (!isKnownDirective && macroProcessor_.isMacroDefined(checkName)) {
                    std::string macroName = checkName;
                    std::vector<std::string> arguments;
                    
                    // Parse arguments
                    if (labelName.empty()) {
                        token = lexer.nextToken();
                    }
                    while (token.type == TokenType::Identifier || token.type == TokenType::Number || 
                           token.type == TokenType::String) {
                        std::string arg;
                        if (token.type == TokenType::Number) {
                            arg = std::to_string(token.numValue);
                        } else {
                            arg = token.text;
                        }
                        arguments.push_back(arg);
                        
                        token = lexer.nextToken();
                        if (token.type == TokenType::Comma) {
                            token = lexer.nextToken();
                        } else {
                            break;
                        }
                    }
                    
                    // Begin expansion
                    macroProcessor_.beginExpansion(macroName, arguments);
                    
                    // Expand all lines from this macro
                    while (macroProcessor_.isExpanding()) {
                        std::string expandedLine;
                        if (macroProcessor_.getNextLine(expandedLine)) {
                            expandedLines.push_back(expandedLine);
                        } else {
                            break;
                        }
                    }
                    continue;
                }
            }
        }
        
        // If in macro/rept/irp/irpc definition, accumulate body
        if (inMacroDef || inReptDef || inIRPDef || inIRPCDef) {
            currentBody.push_back(line);
        } else {
            // Normal line, pass through
            expandedLines.push_back(line);
        }
    }
    
    return true;
}

} // namespace z80
