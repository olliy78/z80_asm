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

//=============================================================================
// CP/M File Format Handling
//=============================================================================

/**
 * @brief Splits a line at carriage return (CR) characters not followed by line feed
 * 
 * CP/M files sometimes use CR alone as line separator instead of CR+LF.
 * This function detects such cases and splits them into separate lines.
 * 
 * @param line Input line that may contain embedded CR characters
 * @return Vector of lines split at standalone CR characters
 */
static std::vector<std::string> splitAtCR(const std::string& line) {
    std::vector<std::string> result;
    std::string current;
    
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == '\r') {
            // Check if next char is LF
            if (i + 1 < line.length() && line[i+1] == '\n') {
                // CR+LF - keep both, it's normal line ending
                current += line[i];
            } else {
                // CR without LF - treat as line separator
                result.push_back(current);
                current.clear();
            }
        } else if (line[i] == '\n') {
            // LF - end current line
            result.push_back(current);
            current.clear();
        } else {
            current += line[i];
        }
    }
    
    // Don't forget the last line if there's content
    if (!current.empty()) {
        result.push_back(current);
    }
    
    return result;
}

/**
 * @brief Removes CP/M control characters from a line
 * 
 * CP/M files may contain control characters (0x00-0x1F and 0x80-0x9F) that need
 * to be filtered out. This function:
 * - Preserves TAB, CR, and LF characters
 * - Removes high control characters (0x80-0x9F) at line start or after CR
 * - Removes other low control characters
 * 
 * @param line Input line with potential control characters
 * @return Cleaned line with control characters removed
 */
static std::string cleanCpmLine(const std::string& line) {
    std::string cleaned;
    cleaned.reserve(line.length());
    
    bool afterCR = false; // Track if we're right after a CR
    
    for (size_t i = 0; i < line.length(); i++) {
        unsigned char c = static_cast<unsigned char>(line[i]);
        
        // Track CR (carriage return)
        if (c == '\r') {
            cleaned += line[i];
            afterCR = true;
            continue;
        }
        
        // Skip control characters at start of line OR after CR (common in CP/M files)
        // This includes 0x8A which appears before some labels
        if ((cleaned.empty() || afterCR) && c >= 0x80 && c <= 0x9F) {
            continue; // Skip high control characters at line start or after CR
        }
        
        afterCR = false; // Reset after first non-CR character
        
        // Skip low control characters (except TAB, CR, LF)
        if (c < 32 && c != '\t' && c != '\r' && c != '\n') {
            continue;
        }
        
        cleaned += line[i];
    }
    
    return cleaned;
}

//=============================================================================
// Parser Constructor and Main Assembly Function
//=============================================================================

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
    // Set up expression evaluator for macro %(expression) syntax
    macroProcessor_.setExpressionEvaluator([this](const std::string& expr) -> int64_t {
        ExpressionEvaluator evaluator(symbolTable_, locationCounter_, currentSegment_);
        ExpressionResult result = evaluator.evaluate(expr);
        return result.valid ? result.value : 0;
    });
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
        // CP/M files use Control-Z (0x1A) as EOF marker
        // Check BEFORE cleaning to catch EOF marker
        size_t ctrlZPos = line.find('\x1A');
        if (ctrlZPos != std::string::npos) {
            // Truncate line at Control-Z
            line = line.substr(0, ctrlZPos);
            // Clean what's before Control-Z
            line = cleanCpmLine(line);
            // Split at CR if needed
            auto splitLines = splitAtCR(line);
            for (const auto& l : splitLines) {
                sourceLines.push_back(l);
            }
            break; // Stop reading file
        }
        
        // Clean CP/M control characters
        line = cleanCpmLine(line);
        
        // Split at CR (carriage return without LF) - common in CP/M files
        auto splitLines = splitAtCR(line);
        for (const auto& l : splitLines) {
            sourceLines.push_back(l);
        }
    }
    
    // Pass 1: Build symbol table
    if (!pass1(sourceLines, filename)) {
        return false;
    }
    
    // Pass 2: Generate code
    if (!pass2(sourceLines, filename)) {
        return false;
    }
    
    // Check if there are any actual errors (not just warnings)
    for (const auto& err : errors_) {
        if (err.level == ErrorLevel::Error) {
            return false;
        }
    }
    
    return true;
}

//=============================================================================
// Pass 1: Symbol Table Building and Address Calculation
//=============================================================================

bool Parser::pass1(const std::vector<std::string>& sourceLines, const std::string& filename) {
    locationCounter_ = 0;
    currentSegment_ = SegmentType::CSEG;
    inMacroDefinition_ = false;
    macroBody_.clear();
    conditionalProcessor_.clear();
    conditionalProcessor_.setPass(1);
    sourceLocations_.clear();
    
    // Pass 1: Build symbol table and calculate addresses
    
    // Process source lines (including macro expansion)
    std::vector<std::string> expandedLines;
    expandSourceWithMacros(sourceLines, expandedLines, sourceLocations_, filename);
    
    for (size_t i = 0; i < expandedLines.size(); ++i) {
        const std::string& line = expandedLines[i];
        int lineNum = i + 1;
        
        // Get original source location for error reporting
        std::string errorFilename = filename;
        int errorLineNum = lineNum;
        if (i < sourceLocations_.size()) {
            errorFilename = sourceLocations_[i].filename;
            errorLineNum = sourceLocations_[i].lineNumber;
        }
        
        Lexer lexer(line, filename);
        Token token = lexer.nextToken();
        
        // Skip empty lines and comments
        if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) {
            continue;
        }
        
        // Check FIRST if this is a conditional directive (before label processing)
        // Conditional directives never have labels and control assembly flow
        if (token.type == TokenType::Identifier) {
            std::string upperToken = token.text;
            for (char& c : upperToken) c = std::toupper(c);
            
            bool isConditionalDirective = false;
            if (upperToken == "IF" || upperToken == "IFT") {
                std::string expression;
                while (true) {
                    token = lexer.nextToken();
                    if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                    expression += token.text + " ";
                }
                conditionalProcessor_.processIF(expression, symbolTable_, lineNum);
                isConditionalDirective = true;
            }
            else if (upperToken == "IFE" || upperToken == "IFF") {
                std::string expression;
                while (true) {
                    token = lexer.nextToken();
                    if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                    expression += token.text + " ";
                }
                conditionalProcessor_.processIFE(expression, symbolTable_, lineNum);
                isConditionalDirective = true;
            }
            else if (upperToken == "IF1") {
                conditionalProcessor_.processIF1(lineNum);
                isConditionalDirective = true;
            }
            else if (upperToken == "IF2") {
                conditionalProcessor_.processIF2(lineNum);
                isConditionalDirective = true;
            }
            else if (upperToken == "IFDEF") {
                token = lexer.nextToken();
                if (token.type == TokenType::Identifier) {
                    conditionalProcessor_.processIFDEF(token.text, symbolTable_, lineNum);
                }
                isConditionalDirective = true;
            }
            else if (upperToken == "IFNDEF") {
                token = lexer.nextToken();
                if (token.type == TokenType::Identifier) {
                    conditionalProcessor_.processIFNDEF(token.text, symbolTable_, lineNum);
                }
                isConditionalDirective = true;
            }
            else if (upperToken == "IFB") {
                std::string argument;
                while (true) {
                    token = lexer.nextToken();
                    if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                    argument += token.text;
                }
                conditionalProcessor_.processIFB(argument, lineNum);
                isConditionalDirective = true;
            }
            else if (upperToken == "IFNB") {
                std::string argument;
                while (true) {
                    token = lexer.nextToken();
                    if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                    argument += token.text;
                }
                conditionalProcessor_.processIFNB(argument, lineNum);
                isConditionalDirective = true;
            }
            else if (upperToken == "IFIDN") {
                std::string arg1, arg2;
                bool foundComma = false;
                while (true) {
                    token = lexer.nextToken();
                    if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                    if (token.type == TokenType::Comma && !foundComma) {
                        foundComma = true;
                    } else if (foundComma) {
                        arg2 += token.text;
                    } else {
                        arg1 += token.text;
                    }
                }
                conditionalProcessor_.processIFIDN(arg1, arg2, lineNum);
                isConditionalDirective = true;
            }
            else if (upperToken == "IFDIF") {
                std::string arg1, arg2;
                bool foundComma = false;
                while (true) {
                    token = lexer.nextToken();
                    if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                    if (token.type == TokenType::Comma && !foundComma) {
                        foundComma = true;
                    } else if (foundComma) {
                        arg2 += token.text;
                    } else {
                        arg1 += token.text;
                    }
                }
                conditionalProcessor_.processIFDIF(arg1, arg2, lineNum);
                isConditionalDirective = true;
            }
            else if (upperToken == "ELSE") {
                if (!conditionalProcessor_.processELSE(lineNum)) {
                    AssemblyError err;
                    err.level = ErrorLevel::Warning;
                    err.message = "ELSE without IF (ignored)";
                    err.filename = errorFilename;
                    err.line = errorLineNum;
                    err.column = 0;
                    errors_.push_back(err);
                }
                isConditionalDirective = true;
            }
            else if (upperToken == "ENDIF") {
                if (!conditionalProcessor_.processENDIF(lineNum)) {
                    AssemblyError err;
                    err.level = ErrorLevel::Warning;
                    err.message = "ENDIF without IF (ignored)";
                    err.filename = errorFilename;
                    err.line = errorLineNum;
                    err.column = 0;
                    errors_.push_back(err);
                }
                isConditionalDirective = true;
            }
            
            // If it was a conditional directive, skip to next line
            if (isConditionalDirective) {
                continue;
            }
        }
        
        // Check if we should assemble this line (based on conditionals)
        if (!conditionalProcessor_.shouldAssemble()) {
            continue;  // Skip line inside false conditional
        }
        
        ParsedLine parsedLine;
        parsedLine.lineNumber = errorLineNum;  // Use original source line number
        parsedLine.sourceFile = errorFilename;  // Use original source filename
        parsedLine.address = locationCounter_;
        parsedLine.segment = currentSegment_;
        
        // Now do normal label parsing
        std::string labelName;
        if (token.type == TokenType::Identifier) {
            Token next = lexer.peekToken();
            if (next.type == TokenType::Colon) {
                // This is definitely a label (has colon)
                labelName = token.text;
                parsedLine.label = labelName;

                lexer.nextToken();  // Skip colon
                token = lexer.nextToken();
                
                // After label, check if the next token is a conditional directive
                if (token.type == TokenType::Identifier) {
                    std::string upperToken = token.text;
                    for (char& c : upperToken) c = std::toupper(c);
                    
                    bool isConditionalDirective = false;
                    if (upperToken == "IF" || upperToken == "IFT") {
                        std::string expression;
                        while (true) {
                            token = lexer.nextToken();
                            if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                            expression += token.text + " ";
                        }
                        conditionalProcessor_.processIF(expression, symbolTable_, lineNum);
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IFE" || upperToken == "IFF") {
                        std::string expression;
                        while (true) {
                            token = lexer.nextToken();
                            if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                            expression += token.text + " ";
                        }
                        conditionalProcessor_.processIFE(expression, symbolTable_, lineNum);
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IF1") {
                        conditionalProcessor_.processIF1(lineNum);
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IF2") {
                        conditionalProcessor_.processIF2(lineNum);
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IFDEF") {
                        token = lexer.nextToken();
                        if (token.type == TokenType::Identifier) {
                            conditionalProcessor_.processIFDEF(token.text, symbolTable_, lineNum);
                        }
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IFNDEF") {
                        token = lexer.nextToken();
                        if (token.type == TokenType::Identifier) {
                            conditionalProcessor_.processIFNDEF(token.text, symbolTable_, lineNum);
                        }
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IFB") {
                        std::string argument;
                        while (true) {
                            token = lexer.nextToken();
                            if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                            argument += token.text;
                        }
                        conditionalProcessor_.processIFB(argument, lineNum);
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IFNB") {
                        std::string argument;
                        while (true) {
                            token = lexer.nextToken();
                            if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) break;
                            argument += token.text;
                        }
                        conditionalProcessor_.processIFNB(argument, lineNum);
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IFIDN") {
                        std::string arg1, arg2;
                        token = lexer.nextToken();
                        if (token.type == TokenType::Identifier || token.type == TokenType::String) {
                            arg1 = token.text;
                            token = lexer.nextToken();
                            if (token.type == TokenType::Comma) {
                                token = lexer.nextToken();
                                if (token.type == TokenType::Identifier || token.type == TokenType::String) {
                                    arg2 = token.text;
                                }
                            }
                        }
                        conditionalProcessor_.processIFIDN(arg1, arg2, lineNum);
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "IFDIF") {
                        std::string arg1, arg2;
                        token = lexer.nextToken();
                        if (token.type == TokenType::Identifier || token.type == TokenType::String) {
                            arg1 = token.text;
                            token = lexer.nextToken();
                            if (token.type == TokenType::Comma) {
                                token = lexer.nextToken();
                                if (token.type == TokenType::Identifier || token.type == TokenType::String) {
                                    arg2 = token.text;
                                }
                            }
                        }
                        conditionalProcessor_.processIFDIF(arg1, arg2, lineNum);
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "ELSE") {
                        if (!conditionalProcessor_.processELSE(lineNum)) {
                            AssemblyError err;
                            err.level = ErrorLevel::Warning;
                            err.message = "ELSE without IF (ignored)";
                            err.filename = filename;
                            err.line = lineNum;
                            err.column = 0;
                            errors_.push_back(err);
                        }
                        isConditionalDirective = true;
                    }
                    else if (upperToken == "ENDIF") {
                        if (!conditionalProcessor_.processENDIF(lineNum)) {
                            AssemblyError err;
                            err.level = ErrorLevel::Warning;
                            err.message = "ENDIF without IF (ignored)";
                            err.filename = filename;
                            err.line = lineNum;
                            err.column = 0;
                            errors_.push_back(err);
                        }
                        isConditionalDirective = true;
                    }
                    
                    if (isConditionalDirective) {
                        // Add label to symbol table but don't process rest of line
                        if (!labelName.empty()) {
                            Symbol sym;
                            sym.type = SymbolType::Label;
                            sym.value = locationCounter_ + (inPhase_ ? phaseOffset_ : 0);
                            sym.segment = currentSegment_;
                            sym.defined = true;
                            sym.isRelocatable = (currentSegment_ != SegmentType::ASEG);
                            sym.definedLine = lineNum;
                            symbolTable_.addSymbol(labelName, sym);
                        }
                        continue;
                    }
                }
            }
            else if (next.type == TokenType::Identifier) {
                // Could be "LABEL MNEM" or "MNEM OPERAND"
                // First check if NEXT token is a directive - if so, first token is definitely a label
                std::string upperNext = next.text;
                for (char& c : upperNext) c = std::toupper(c);
                bool nextIsDirective = (upperNext == "EQU" || upperNext == "ASET" ||
                                       upperNext == "DB" || upperNext == "DW" || upperNext == "DS");
                
                if (nextIsDirective) {
                    // First token is a label, next is a directive
                    labelName = token.text;
                    parsedLine.label = labelName;
                    token = lexer.nextToken();
                } else {
                    // Check if first token is a known mnemonic or directive
                    std::string upperToken = token.text;
                    for (char& c : upperToken) c = std::toupper(c);
                    bool isDirective = (upperToken == "ORG" || upperToken == "EQU" || upperToken == "ASET" ||
                                       upperToken == "DB" || upperToken == "DW" || 
                                       upperToken == "DS" || upperToken == "END" ||
                                       upperToken == "PUBLIC" || upperToken == "EXTRN" ||
                                       upperToken == "ENTRY" || upperToken == "EXT" ||
                                       upperToken == "NAME" || upperToken == "TITLE" ||
                                       upperToken == "PHASE" || upperToken == "DEPHASE" ||
                                       upperToken == ".PHASE" || upperToken == ".DEPHASE" ||
                                       upperToken == "INCLUDE" || upperToken == "PAGE" || upperToken == ".PAGE" ||
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
            }
            else if (next.type == TokenType::EndOfLine || next.type == TokenType::EndOfFile) {
                // Check if it's a mnemonic without operands (like NOP, RET, etc.)
                std::string upperToken = token.text;
                for (char& c : upperToken) c = std::toupper(c);
                bool isDirective = (upperToken == "END" || upperToken == "ORG" || 
                                   upperToken == "CSEG" || upperToken == "DSEG" || upperToken == "ASEG");
                
                if (instructions_.isMnemonic(token.text) || isDirective) {
                    // It's an instruction/directive without operands, not a label
                    // Don't consume token, continue to mnemonic parsing
                } else {
                    // It's a standalone label
                    labelName = token.text;
                    parsedLine.label = labelName;
                    token = lexer.nextToken();
                }
            }
        }
        
        // Parse mnemonic/directive
        // If we have a label but no mnemonic (standalone label), add it to symbol table
        if (!labelName.empty() && token.type != TokenType::Identifier) {
            Symbol sym;
            sym.type = SymbolType::Label;
            sym.value = locationCounter_ + (inPhase_ ? phaseOffset_ : 0);
            sym.segment = currentSegment_;
            sym.defined = true;
            sym.isRelocatable = (currentSegment_ != SegmentType::ASEG);
            sym.definedLine = lineNum;
            symbolTable_.addSymbol(labelName, sym);

        }
        
        if (token.type == TokenType::Identifier) {
            parsedLine.mnemonic = token.text;
            
            // Check if it's a directive
            std::string upperMnemonic = token.text;
            for (char& c : upperMnemonic) c = std::toupper(c);
            
            // Add label to symbol table FIRST (before processing directive/instruction)
            // BUT NOT for EQU/ASET - they define the label themselves
            if (!labelName.empty() && upperMnemonic != "EQU" && upperMnemonic != "ASET") {
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
                if (!labelName.empty()) {
                    // Parse expression
                    std::string expression;
                    while (token.type != TokenType::EndOfLine && token.type != TokenType::EndOfFile) {
                        if (token.type == TokenType::Comment) break;
                        expression += token.text + " ";
                        token = lexer.nextToken();
                    }
                    
                    // Evaluate expression
                    ExpressionEvaluator evaluator(symbolTable_, locationCounter_, currentSegment_);
                    ExpressionResult result = evaluator.evaluate(expression);
                    
                    if (result.valid) {
                        Symbol sym;
                        sym.type = SymbolType::Equ;
                        sym.value = result.value;
                        sym.defined = true;
                        sym.segment = currentSegment_;
                        sym.definedLine = lineNum;
                        symbolTable_.addSymbol(labelName, sym);
                    }
                }
            }
            else if (upperMnemonic == "ASET") {
                // ASET: assignable variable (like EQU but can be redefined)
                token = lexer.nextToken();
                if (!labelName.empty()) {
                    // Parse expression
                    std::string expression;
                    while (token.type != TokenType::EndOfLine && token.type != TokenType::EndOfFile) {
                        if (token.type == TokenType::Comment) break;
                        expression += token.text + " ";
                        token = lexer.nextToken();
                    }
                    
                    // Evaluate expression
                    ExpressionEvaluator evaluator(symbolTable_, locationCounter_, currentSegment_);
                    ExpressionResult result = evaluator.evaluate(expression);
                    
                    if (result.valid) {
                        Symbol sym;
                        sym.type = SymbolType::Equ;
                        sym.value = result.value;
                        sym.defined = true;
                        sym.segment = currentSegment_;
                        sym.definedLine = lineNum;
                        // ASET allows redefinition, so we update if exists
                        Symbol* existingSym = symbolTable_.getSymbol(labelName);
                        if (existingSym) {
                            existingSym->value = result.value;
                            existingSym->defined = true;
                        } else {
                            symbolTable_.addSymbol(labelName, sym);
                        }
                    }
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
                // Continue processing directive/instruction (label already added above)
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
                else if (upperMnemonic == ".RADIX") {
                    // .RADIX: Set default number base (ignore for now, always use decimal/hex notation)
                    // Skip the radix value
                    token = lexer.nextToken();
                }
                else if (upperMnemonic == "PAGE" || upperMnemonic == ".PAGE") {
                    // Page control directive (ignore for now)
                    // Skip optional page length parameter
                    token = lexer.nextToken();
                    if (token.type == TokenType::Number) {
                        // Page length specified, just ignore it
                    }
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

//=============================================================================
// Pass 2: Machine Code Generation
//=============================================================================

bool Parser::pass2(const std::vector<std::string>& sourceLines, const std::string& filename) {
    // Pass 2: Generate actual machine code for each line
    locationCounter_ = 0;
    currentSegment_ = SegmentType::CSEG;
    conditionalProcessor_.clear();
    conditionalProcessor_.setPass(2);
    
    // Re-expand source with macros (we need to do this again for pass2)
    std::vector<std::string> expandedLines;
    macroProcessor_.clear(); // Reset macro state
    expandSourceWithMacros(sourceLines, expandedLines, sourceLocations_, filename);
    
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
        else if (upperMnemonic == "ASET") {
            // ASET handled in pass 1
            continue;
        }
        else if (upperMnemonic == "PAGE" || upperMnemonic == ".PAGE") {
            // PAGE directive (listing control, ignore in pass 2)
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
        else if (upperMnemonic == "IF" || upperMnemonic == "IFT" || upperMnemonic == "IFE" || upperMnemonic == "IFF" ||
                 upperMnemonic == "IF1" || upperMnemonic == "IF2" || upperMnemonic == "IFDEF" || upperMnemonic == "IFNDEF" ||
                 upperMnemonic == "IFB" || upperMnemonic == "IFNB" || upperMnemonic == "IFIDN" || upperMnemonic == "IFDIF" ||
                 upperMnemonic == "ELSE" || upperMnemonic == "ENDIF") {
            // Conditional directives handled in pass 1
            continue;
        }
        else if (upperMnemonic == "END") {
            // Stop processing
            break;
        }
        else {
            // It's an instruction - generate code
            if (!generateInstruction(line, line.sourceFile)) {
                return false;
            }
        }
    }
    
    return true;
}

//=============================================================================
// Code Generation: DB, DW, and Instructions
//=============================================================================

/**
 * @brief Generates byte data for DB (Define Byte) directive
 * 
 * Processes comma-separated operands which can be:
 * - String literals: 'text' or "text" - each character becomes a byte
 * - Numeric expressions: evaluated and stored as 8-bit values
 * 
 * @param line Parsed line containing DB directive and operands
 * @param filename Source filename (unused but kept for consistency)
 * @return true if successful, false on evaluation errors
 */
bool Parser::generateDB(ParsedLine& line, const std::string& /* filename */) {
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
            err.filename = line.sourceFile;
            err.line = line.lineNumber;
            errors_.push_back(err);
            return false;
        }
        
        // Store byte value (truncate to 8 bits)
        line.code.push_back(static_cast<Byte>(result.value & 0xFF));
    }
    
    return true;
}

/**
 * @brief Generates word data for DW (Define Word) directive
 * 
 * Processes comma-separated numeric expressions and stores them as
 * 16-bit words in little-endian format (LSB first, as used by Z80).
 * 
 * @param line Parsed line containing DW directive and operands
 * @param filename Source filename (unused but kept for consistency)
 * @return true if successful, false on evaluation errors
 */
bool Parser::generateDW(ParsedLine& line, const std::string& /* filename */) {
    // DW generates 16-bit words from comma-separated expressions
    // Z80 uses little-endian (LSB first)
    
    ExpressionEvaluator eval(symbolTable_, locationCounter_, currentSegment_);
    
    for (const std::string& operand : line.operands) {
        ExpressionResult result = eval.evaluate(operand);
        
        if (!result.valid) {
            AssemblyError err;
            err.level = ErrorLevel::Error;
            err.message = "Error evaluating DW expression '" + operand + "': " + result.errorMessage;
            err.filename = line.sourceFile;
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

/**
 * @brief Generates machine code for a Z80 instruction
 * 
 * This function:
 * 1. Finds the matching instruction variant based on operand patterns
 * 2. Emits the instruction's opcode bytes
 * 3. Evaluates and emits operand bytes (immediate values, addresses, displacements)
 * 4. Handles special cases like JR relative addressing
 * 
 * @param line Parsed line containing instruction mnemonic and operands
 * @param filename Source filename (unused but kept for consistency)
 * @return true if instruction found and generated, false if unknown instruction
 */
bool Parser::generateInstruction(ParsedLine& line, const std::string& /* filename */) {
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
                
                // Skip register operands (including indirect register addressing)
                if (isRegisterOperand(upper)) continue;
                
                // Check for indirect register addressing: (HL), (BC), (DE), (SP), (IX), (IY)
                if (upper == "(HL)" || upper == "(BC)" || upper == "(DE)" || upper == "(SP)" ||
                    upper == "(IX)" || upper == "(IY)") {
                    continue; // Skip these - they're register operands, not expressions
                }
                
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
                    err.filename = line.sourceFile;
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
                            err.filename = line.sourceFile;
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
    err.filename = line.sourceFile;
    err.line = line.lineNumber;
    err.column = 0;
    errors_.push_back(err);
    
    return false;
}

//=============================================================================
// Operand Parsing and Pattern Matching
//=============================================================================

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
std::string Parser::parseOperands(Lexer& lexer, std::vector<std::string>& operands) {
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
const InstructionInfo* Parser::findInstructionVariant(const std::string& mnemonic,
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
std::string Parser::operandToPattern(const std::string& operand, const std::string& mnemonic) {
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

/**
 * @brief Checks if an operand string is a Z80 register name
 * 
 * @param operand Operand string to check (case-insensitive)
 * @return true if operand is a valid Z80 register (A, B, C, D, E, H, L, AF, BC, DE, HL, SP, IX, IY, I, R)
 */
bool Parser::isRegisterOperand(const std::string& operand) {
    std::string upper = operand;
    for (char& c : upper) c = std::toupper(c);
    return instructions_.isRegister(upper);
}

//=============================================================================
// Public Accessors
//=============================================================================

const std::vector<AssemblyError>& Parser::getErrors() const {
    return errors_;
}

const SymbolTable& Parser::getSymbolTable() const {
    return symbolTable_;
}

//=============================================================================
// REL File Generation
//=============================================================================

/**
 * @brief Writes assembled code to Intel/Microsoft REL format
 * 
 * @deprecated Use RELWriter::writeFromParser() instead for better separation of concerns
 * 
 * This method delegates to RELWriter::writeFromParser(). It exists for backward
 * compatibility but should not be used in new code.
 * 
 * @param filename Output .REL filename
 * @param moduleName Module name (derived from filename if empty)
 * @return true on success, false on error
 */
bool Parser::writeREL(const std::string& filename, const std::string& moduleName) {
    RELWriter writer;
    return writer.writeFromParser(*this, filename, moduleName);
}

//=============================================================================
// Macro Expansion and Source Processing
//=============================================================================

/**
 * @brief Adds a line to expanded output with source location tracking
 * 
 * Helper function to maintain parallel arrays of expanded lines and their
 * original source locations for accurate error reporting.
 * 
 * @param expandedLines Output vector of expanded source lines
 * @param sourceLocations Output vector of source locations (parallel to expandedLines)
 * @param line The expanded line to add
 * @param filename Original filename where the line came from
 * @param lineNumber Original line number in the source file
 */
void Parser::addExpandedLine(std::vector<std::string>& expandedLines,
                             std::vector<SourceLocation>& sourceLocations,
                             const std::string& line,
                             const std::string& filename,
                             int lineNumber) {
    expandedLines.push_back(line);
    SourceLocation loc;
    loc.filename = filename;
    loc.lineNumber = lineNumber;
    sourceLocations.push_back(loc);
}

/**
 * @brief Expands macros and INCLUDEs in source code (multi-pass)
 * 
 * Performs up to 10 iterations of macro expansion to handle nested macro calls.
 * Each iteration may expand macros that were generated by the previous iteration.
 * Preserves source location tracking through all expansion passes.
 * 
 * Process:
 * 1. First pass: Expand all INCLUDEs and macros, creating initial source locations
 * 2. Subsequent passes: Re-expand macros from previous expansion, preserving locations
 * 3. Stops when no more expansions occur or max iterations reached
 * 
 * @param sourceLines Input source code lines
 * @param expandedLines Output fully expanded source lines
 * @param sourceLocations Output source locations for each expanded line
 * @param filename Name of main source file
 * @return true on success, false on error
 */
bool Parser::expandSourceWithMacros(const std::vector<std::string>& sourceLines,
                                    std::vector<std::string>& expandedLines,
                                    std::vector<SourceLocation>& sourceLocations,
                                    const std::string& filename) {
    // Do multiple passes to handle nested macro calls
    // Pass 1: Expand all macros
    std::vector<std::string> tempExpanded;
    std::vector<SourceLocation> tempLocations;
    
    // First expansion: no input locations, create new ones
    std::vector<SourceLocation> emptyLocations;
    bool expandedAnything = false;
    expandSourceWithMacrosImpl(sourceLines, tempExpanded, tempLocations, emptyLocations, filename, expandedAnything);
    
    // Pass 2+: Keep expanding until no more macros are found (up to max iterations)
    // Use existing locations from previous iteration
    const int MAX_ITERATIONS = 10;
    for (int iteration = 0; iteration < MAX_ITERATIONS && expandedAnything; ++iteration) {
        std::vector<std::string> nextExpanded;
        std::vector<SourceLocation> nextLocations;
        expandedAnything = false;
        
        // Pass existing locations so they are preserved
        expandSourceWithMacrosImpl(tempExpanded, nextExpanded, nextLocations, tempLocations, filename, expandedAnything);
        
        tempExpanded = std::move(nextExpanded);
        tempLocations = std::move(nextLocations);
    }
    
    expandedLines = std::move(tempExpanded);
    sourceLocations = std::move(tempLocations);
    
    return true;
}

/**
 * @brief Implementation of single-pass macro expansion
 * 
 * Processes source lines and expands:
 * - INCLUDE directives: Reads and inserts external files
 * - MACRO definitions: Stores macro definitions for later use
 * - Macro invocations: Expands macro calls with parameter substitution
 * - REPT blocks: Repeats code sections N times
 * - IRP blocks: Iterates over parameter lists
 * - IRPC blocks: Iterates over characters in a string
 * 
 * Maintains source location tracking by:
 * - Using inputLocations when available (for subsequent passes)
 * - Creating new locations for first pass
 * - Preserving original file:line for all expanded content
 * 
 * @param sourceLines Input source lines to process
 * @param expandedLines Output expanded lines
 * @param sourceLocations Output source locations (parallel to expandedLines)
 * @param inputLocations Input source locations from previous pass (empty for first pass)
 * @param filename Main source filename for new locations
 * @param expandedAnything Output flag: set to true if any expansion occurred
 * @return true on success, false on error
 */
bool Parser::expandSourceWithMacrosImpl(const std::vector<std::string>& sourceLines,
                                        std::vector<std::string>& expandedLines,
                                        std::vector<SourceLocation>& sourceLocations,
                                        const std::vector<SourceLocation>& inputLocations,
                                        const std::string& filename,
                                        bool& expandedAnything) {
    // Note: State variables for macro/rept/irp/irpc definitions removed
    // MacroProcessor now handles parsing internally
    
    for (size_t lineIdx = 0; lineIdx < sourceLines.size(); ++lineIdx) {
        const std::string& line = sourceLines[lineIdx];
        
        // Get source location for this line from input if available
        std::string currentFilename = filename;
        int currentLineNum = lineIdx + 1;
        if (lineIdx < inputLocations.size()) {
            currentFilename = inputLocations[lineIdx].filename;
            currentLineNum = inputLocations[lineIdx].lineNumber;
        }
        
        // If we're expanding macros, get lines from macro processor
        while (macroProcessor_.isExpanding()) {
            std::string expandedLine;
            if (macroProcessor_.getNextLine(expandedLine)) {
                addExpandedLine(expandedLines, sourceLocations, expandedLine, filename, lineIdx + 1);
            } else {
                break; // Expansion complete
            }
        }
        
        Lexer lexer(line, filename);
        Token token = lexer.nextToken();
        
        // Skip empty lines and comments
        if (token.type == TokenType::EndOfLine || token.type == TokenType::EndOfFile) {
            addExpandedLine(expandedLines, sourceLocations, line, currentFilename, currentLineNum);
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
                bool isDirective = (upperToken == "ORG" || upperToken == "EQU" || upperToken == "ASET" ||
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
                                   upperToken == ".RADIX" ||
                                   upperToken == "INCLUDE" ||
                                   upperToken == "MACRO" || upperToken == "REPT" ||
                                   upperToken == "IRP" || upperToken == "IRPC" ||
                                   upperToken == "ENDM" || upperToken == "LOCAL" ||
                                   upperToken == "EXITM");
                
                // Check if it's a macro name (don't treat as label)
                bool isMacro = macroProcessor_.isMacroDefined(token.text);
                
                if (!isDirective && !isMacro && next.type == TokenType::Identifier) {
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
            
            // INCLUDE directive - process included file
            if (upperMnemonic == "INCLUDE") {
                token = lexer.nextToken();
                if (token.type == TokenType::Identifier || token.type == TokenType::String) {
                    std::string includeFile = token.text;
                    
                    // M80 compatibility: Add .mac extension if no extension is present
                    if (includeFile.find('.') == std::string::npos) {
                        // Convert to lowercase for M80 compatibility
                        std::string lowerFile = includeFile;
                        for (char& c : lowerFile) c = std::tolower(c);
                        includeFile = lowerFile + ".mac";
                    }
                    
                    // Build include file path (relative to current file's directory)
                    std::string includePath;
                    size_t lastSlash = filename.find_last_of("/\\");
                    if (lastSlash != std::string::npos) {
                        includePath = filename.substr(0, lastSlash + 1) + includeFile;
                    } else {
                        includePath = includeFile;
                    }
                    
                    // Read included file
                    std::ifstream includeStream(includePath);
                    if (!includeStream.is_open()) {
                        // Try without path modification
                        includeStream.open(includeFile);
                        if (!includeStream.is_open()) {
                            // Error: cannot open include file
                            expandedLines.push_back("; ERROR: Cannot open include file: " + includeFile);
                            continue;
                        }
                    }
                    
                    // Read all lines from included file
                    std::vector<std::string> includeLines;
                    std::string includeLine;
                    while (std::getline(includeStream, includeLine)) {
                        // CP/M files use Control-Z (0x1A) as EOF marker
                        // Check BEFORE cleaning
                        size_t ctrlZPos = includeLine.find('\x1A');
                        if (ctrlZPos != std::string::npos) {
                            // Truncate line at Control-Z
                            includeLine = includeLine.substr(0, ctrlZPos);
                            // Clean what's before Control-Z
                            includeLine = cleanCpmLine(includeLine);
                            // Split at CR if needed
                            auto splitLines = splitAtCR(includeLine);
                            for (const auto& l : splitLines) {
                                includeLines.push_back(l);
                            }
                            break; // Stop reading file
                        }
                        
                        // Clean CP/M control characters
                        includeLine = cleanCpmLine(includeLine);
                        
                        // Split at CR
                        auto splitLines = splitAtCR(includeLine);
                        for (const auto& l : splitLines) {
                            includeLines.push_back(l);
                        }
                    }
                    includeStream.close();
                    
                    // Recursively expand included file (for nested includes and macros)
                    std::vector<std::string> expandedInclude;
                    std::vector<SourceLocation> includeLocations;
                    expandSourceWithMacros(includeLines, expandedInclude, includeLocations, includePath);
                    
                    // Add expanded lines to output WITH their source locations
                    // If there's a label, prepend it to the first non-empty, non-comment line
                    bool labelAttached = false;
                    for (size_t i = 0; i < expandedInclude.size(); ++i) {
                        std::string outputLine = expandedInclude[i];
                        
                        // Attach label to first meaningful line (not empty, not just comment)
                        if (!labelAttached && !labelName.empty()) {
                            std::string trimmed = outputLine;
                            // Trim leading whitespace
                            size_t firstNonSpace = trimmed.find_first_not_of(" \t");
                            if (firstNonSpace != std::string::npos) {
                                trimmed = trimmed.substr(firstNonSpace);
                            }
                            // Check if line is not empty and not just a comment
                            if (!trimmed.empty() && trimmed[0] != ';') {
                                outputLine = labelName + ": " + outputLine;
                                labelAttached = true;
                            }
                        }
                        
                        expandedLines.push_back(outputLine);
                        if (i < includeLocations.size()) {
                            sourceLocations.push_back(includeLocations[i]);
                        } else {
                            // Fallback if locations are missing
                            addExpandedLine(expandedLines, sourceLocations, "", includePath, i + 1);
                            expandedLines.pop_back(); // Remove the empty line we just added
                        }
                    }
                    
                    // If no meaningful line was found to attach the label to, add it as a separate line
                    if (!labelAttached && !labelName.empty()) {
                        expandedLines.push_back(labelName + ":");
                        SourceLocation loc;
                        loc.filename = currentFilename;
                        loc.lineNumber = currentLineNum;
                        sourceLocations.push_back(loc);
                    }
                    
                    continue;
                }
            }
            
            // MACRO definition
            if (upperMnemonic == "MACRO" && !labelName.empty()) {
                // Use MacroProcessor to parse the macro definition
                MacroDefinition macro;
                size_t endIndex;
                if (macroProcessor_.parseMacroDefinition(sourceLines, lineIdx, labelName, macro, endIndex)) {
                    macro.definitionLine = lineIdx + 1;
                    macro.definitionFile = currentFilename;
                    macroProcessor_.defineMacro(macro);
                    // Skip to end of macro definition
                    lineIdx = endIndex - 1; // -1 because loop will increment
                }
                continue;
            }
            
            // REPT definition
            if (upperMnemonic == "REPT") {
                // Parse repeat count
                int repeatCount = 0;
                token = lexer.nextToken();
                if (token.type == TokenType::Number) {
                    repeatCount = static_cast<int>(token.numValue);
                }
                
                // Use MacroProcessor to parse the REPT block
                std::vector<std::string> body;
                std::vector<std::string> localLabels;
                size_t endIndex;
                if (macroProcessor_.parseReptBlock(sourceLines, lineIdx, repeatCount, body, localLabels, endIndex)) {
                    // Begin expansion and emit expanded lines
                    macroProcessor_.beginRepeat(repeatCount, body, localLabels);
                    expandedAnything = true;
                    
                    // Expand immediately
                    while (macroProcessor_.isExpanding()) {
                        std::string expandedLine;
                        if (macroProcessor_.getNextLine(expandedLine)) {
                            addExpandedLine(expandedLines, sourceLocations, expandedLine, currentFilename, currentLineNum);
                        } else {
                            break;
                        }
                    }
                    
                    // Skip to end of REPT block
                    lineIdx = endIndex - 1; // -1 because loop will increment
                }
                continue;
            }
            
            // IRP definition
            if (upperMnemonic == "IRP") {
                // Parse iterator name
                std::string iteratorName;
                token = lexer.nextToken();
                if (token.type == TokenType::Identifier) {
                    iteratorName = token.text;
                }
                
                // Parse argument list in <...>
                std::vector<std::string> iteratorValues;
                token = lexer.nextToken();
                if (token.type == TokenType::LessThan || line.find('<') != std::string::npos) {
                    size_t start = line.find('<');
                    size_t end = line.find('>');
                    if (start != std::string::npos && end != std::string::npos && end > start) {
                        std::string argList = line.substr(start + 1, end - start - 1);
                        
                        // Parse comma-separated values
                        std::istringstream iss(argList);
                        std::string value;
                        while (std::getline(iss, value, ',')) {
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
                
                // Use MacroProcessor to parse the IRP block
                std::vector<std::string> body;
                std::vector<std::string> localLabels;
                size_t endIndex;
                if (macroProcessor_.parseIRPBlock(sourceLines, lineIdx, iteratorName, iteratorValues, body, localLabels, endIndex)) {
                    // Begin expansion and emit expanded lines
                    macroProcessor_.beginIRP(iteratorName, iteratorValues, body, localLabels);
                    expandedAnything = true;
                    
                    // Expand immediately
                    while (macroProcessor_.isExpanding()) {
                        std::string expandedLine;
                        if (macroProcessor_.getNextLine(expandedLine)) {
                            addExpandedLine(expandedLines, sourceLocations, expandedLine, currentFilename, currentLineNum);
                        } else {
                            break;
                        }
                    }
                    
                    // Skip to end of IRP block
                    lineIdx = endIndex - 1;
                }
                continue;
            }
            
            // IRPC definition
            if (upperMnemonic == "IRPC") {
                // Parse iterator name
                std::string iteratorName;
                token = lexer.nextToken();
                if (token.type == TokenType::Identifier) {
                    iteratorName = token.text;
                }
                
                // Parse character string
                std::string irpcChars;
                token = lexer.nextToken();
                if (token.type == TokenType::String || token.type == TokenType::Identifier) {
                    irpcChars = token.text;
                }
                
                // Use MacroProcessor to parse the IRPC block
                std::vector<std::string> body;
                std::vector<std::string> localLabels;
                size_t endIndex;
                if (macroProcessor_.parseIRPCBlock(sourceLines, lineIdx, iteratorName, irpcChars, body, localLabels, endIndex)) {
                    // Begin expansion and emit expanded lines
                    macroProcessor_.beginIRPC(iteratorName, irpcChars, body, localLabels);
                    expandedAnything = true;
                    
                    // Expand immediately
                    while (macroProcessor_.isExpanding()) {
                        std::string expandedLine;
                        if (macroProcessor_.getNextLine(expandedLine)) {
                            addExpandedLine(expandedLines, sourceLocations, expandedLine, currentFilename, currentLineNum);
                        } else {
                            break;
                        }
                    }
                    
                    // Skip to end of IRPC block
                    lineIdx = endIndex - 1;
                }
                continue;
            }
            
            // EXITM - early exit from macro expansion
            if (upperMnemonic == "EXITM") {
                if (macroProcessor_.isExpanding()) {
                    macroProcessor_.exitMacro();
                }
                // Note: EXITM is now handled during expansion, not during definition parsing
                continue;
            }
            
            // Check if it's a macro invocation
            // (First check if it's a known directive or mnemonic)
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
                
                // Check if current token (mnemonic/directive position) is a macro
                // NOT the label - labels can't be macros
                std::string checkName = token.text;
                if (!isKnownDirective && macroProcessor_.isMacroDefined(checkName)) {
                    std::string macroName = checkName;
                    std::vector<std::string> arguments;
                    
                    // Parse arguments - always advance to next token after macro name
                    token = lexer.nextToken();
                    
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
                    
                    // If there's a label before the macro, add it as a separate line
                    // so it gets defined in the symbol table
                    if (!labelName.empty()) {
                        expandedLines.push_back(labelName + ":");
                        SourceLocation loc;
                        loc.filename = currentFilename;
                        loc.lineNumber = currentLineNum;
                        sourceLocations.push_back(loc);
                    }
                    
                    // Begin expansion
                    macroProcessor_.beginExpansion(macroName, arguments);
                    expandedAnything = true;  // Mark that we expanded a macro
                    
                    // Expand all lines from this macro
                    while (macroProcessor_.isExpanding()) {
                        std::string expandedLine;
                        if (macroProcessor_.getNextLine(expandedLine)) {
                            expandedLines.push_back(expandedLine);
                            // Track source location for macro-expanded lines
                            SourceLocation loc;
                            loc.filename = currentFilename;
                            loc.lineNumber = currentLineNum;  // Point back to macro invocation line
                            sourceLocations.push_back(loc);
                        } else {
                            break;
                        }
                    }
                    continue;
                }
            }
        
        // Normal line - pass through with source location
        // (Macro/REPT/IRP/IRPC definitions are now handled above and skip to endIndex)
        addExpandedLine(expandedLines, sourceLocations, line, currentFilename, currentLineNum);
    }
    
    return true;
}

} // namespace z80
