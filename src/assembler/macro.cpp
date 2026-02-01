/**
 * @file macro.cpp
 * @brief Implementation of macro processor
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include "macro.h"
#include "common/utils.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace z80 {

MacroProcessor::MacroProcessor()
    : nextUniqueId_(1)
{
}

void MacroProcessor::defineMacro(const MacroDefinition& macro) {
    macros_[macro.name] = macro;
}

bool MacroProcessor::isMacroDefined(const std::string& name) const {
    return macros_.find(name) != macros_.end();
}

const MacroDefinition* MacroProcessor::getMacro(const std::string& name) const {
    auto it = macros_.find(name);
    return (it != macros_.end()) ? &it->second : nullptr;
}

bool MacroProcessor::beginExpansion(const std::string& name, const std::vector<std::string>& arguments) {
    // Check expansion depth
    if (expansionStack_.size() >= MAX_EXPANSION_DEPTH) {
        return false; // Too deep
    }
    
    // Find macro
    const MacroDefinition* macro = getMacro(name);
    if (!macro) {
        return false;
    }
    
    // Check parameter count
    if (arguments.size() != macro->parameters.size()) {
        // M80 allows fewer arguments (rest become empty strings)
        // but not more arguments
        if (arguments.size() > macro->parameters.size()) {
            return false;
        }
    }
    
    // Create expansion state
    MacroExpansion expansion;
    expansion.definition = macro;
    expansion.arguments = arguments;
    
    // Pad arguments with empty strings if needed
    while (expansion.arguments.size() < macro->parameters.size()) {
        expansion.arguments.push_back("");
    }
    
    expansion.currentLine = 0;
    expansion.uniqueId = getNextUniqueId();
    expansion.exitRequested = false;
    expansion.iterationIndex = 0;
    
    expansionStack_.push_back(expansion);
    return true;
}

bool MacroProcessor::beginRepeat(int repeatCount, const std::vector<std::string>& body,
                                 const std::vector<std::string>& localLabels) {
    // Check expansion depth
    if (expansionStack_.size() >= MAX_EXPANSION_DEPTH) {
        return false;
    }
    
    if (repeatCount <= 0) {
        return false;
    }
    
    // Create temporary macro definition for REPT
    MacroDefinition macro;
    macro.type = MacroType::Repeat;
    macro.name = ""; // Anonymous
    macro.body = body;
    macro.localLabels = localLabels;
    macro.repeatCount = repeatCount;
    
    // Store temporarily (use special name)
    std::string tempName = "$$REPT" + std::to_string(getNextUniqueId());
    macro.name = tempName;
    macros_[tempName] = macro;
    
    // Create expansion state
    MacroExpansion expansion;
    expansion.definition = &macros_[tempName];
    expansion.currentLine = 0;
    expansion.uniqueId = getNextUniqueId();
    expansion.exitRequested = false;
    expansion.iterationIndex = 0;
    
    expansionStack_.push_back(expansion);
    return true;
}

bool MacroProcessor::beginIRP(const std::string& iteratorName, const std::vector<std::string>& values,
                              const std::vector<std::string>& body,
                              const std::vector<std::string>& localLabels) {
    // Check expansion depth
    if (expansionStack_.size() >= MAX_EXPANSION_DEPTH) {
        return false;
    }
    
    if (values.empty()) {
        return true; // IRP with empty list is valid, just does nothing
    }
    
    // Create temporary macro definition for IRP
    MacroDefinition macro;
    macro.type = MacroType::IndefiniteRP;
    macro.name = ""; // Anonymous
    macro.body = body;
    macro.localLabels = localLabels;
    macro.iteratorName = iteratorName;
    macro.iteratorValues = values;
    
    // Store temporarily
    std::string tempName = "$$IRP" + std::to_string(getNextUniqueId());
    macro.name = tempName;
    macros_[tempName] = macro;
    
    // Create expansion state
    MacroExpansion expansion;
    expansion.definition = &macros_[tempName];
    expansion.currentLine = 0;
    expansion.uniqueId = getNextUniqueId();
    expansion.exitRequested = false;
    expansion.iterationIndex = 0;
    
    // Set first iterator value as argument
    expansion.arguments.push_back(values[0]);
    
    expansionStack_.push_back(expansion);
    return true;
}

bool MacroProcessor::beginIRPC(const std::string& iteratorName, const std::string& chars,
                               const std::vector<std::string>& body,
                               const std::vector<std::string>& localLabels) {
    // Check expansion depth
    if (expansionStack_.size() >= MAX_EXPANSION_DEPTH) {
        return false;
    }
    
    if (chars.empty()) {
        return true; // IRPC with empty string is valid, just does nothing
    }
    
    // Create temporary macro definition for IRPC
    MacroDefinition macro;
    macro.type = MacroType::IndefiniteRPC;
    macro.name = ""; // Anonymous
    macro.body = body;
    macro.localLabels = localLabels;
    macro.iteratorName = iteratorName;
    
    // Convert chars to individual strings
    for (char ch : chars) {
        macro.iteratorValues.push_back(std::string(1, ch));
    }
    
    // Store temporarily
    std::string tempName = "$$IRPC" + std::to_string(getNextUniqueId());
    macro.name = tempName;
    macros_[tempName] = macro;
    
    // Create expansion state
    MacroExpansion expansion;
    expansion.definition = &macros_[tempName];
    expansion.currentLine = 0;
    expansion.uniqueId = getNextUniqueId();
    expansion.exitRequested = false;
    expansion.iterationIndex = 0;
    
    // Set first character as argument
    expansion.arguments.push_back(macro.iteratorValues[0]);
    
    expansionStack_.push_back(expansion);
    return true;
}

bool MacroProcessor::getNextLine(std::string& expandedLine) {
    if (expansionStack_.empty()) {
        return false;
    }
    
    MacroExpansion& expansion = expansionStack_.back();
    
    // Check if EXITM was called
    if (expansion.exitRequested) {
        expansionStack_.pop_back();
        return getNextLine(expandedLine); // Continue with outer expansion if any
    }
    
    const MacroDefinition* macro = expansion.definition;
    
    // Check if we've finished this iteration
    if (expansion.currentLine >= static_cast<int>(macro->body.size())) {
        // For REPT: check if more iterations needed
        if (macro->type == MacroType::Repeat) {
            expansion.iterationIndex++;
            if (expansion.iterationIndex < static_cast<size_t>(macro->repeatCount)) {
                // Start next iteration
                expansion.currentLine = 0;
                expansion.uniqueId = getNextUniqueId(); // New unique ID for local labels
            } else {
                // All iterations done
                expansionStack_.pop_back();
                return getNextLine(expandedLine); // Continue with outer expansion if any
            }
        }
        // For IRP/IRPC: check if more values to iterate
        else if (macro->type == MacroType::IndefiniteRP || macro->type == MacroType::IndefiniteRPC) {
            expansion.iterationIndex++;
            if (expansion.iterationIndex < macro->iteratorValues.size()) {
                // Start next iteration with next value
                expansion.currentLine = 0;
                expansion.uniqueId = getNextUniqueId(); // New unique ID for local labels
                expansion.arguments[0] = macro->iteratorValues[expansion.iterationIndex];
            } else {
                // All iterations done
                expansionStack_.pop_back();
                return getNextLine(expandedLine); // Continue with outer expansion if any
            }
        }
        // For regular macros: we're done
        else {
            expansionStack_.pop_back();
            return getNextLine(expandedLine); // Continue with outer expansion if any
        }
    }
    
    // Get and expand current line
    const std::string& line = macro->body[expansion.currentLine];
    expandedLine = expandLine(line, expansion);
    
    expansion.currentLine++;
    return true;
}

void MacroProcessor::exitMacro() {
    if (!expansionStack_.empty()) {
        expansionStack_.back().exitRequested = true;
    }
}

void MacroProcessor::clear() {
    macros_.clear();
    expansionStack_.clear();
    nextUniqueId_ = 1;
}

std::string MacroProcessor::expandLine(const std::string& line, const MacroExpansion& expansion) {
    std::string result = substituteParameters(line, expansion);
    
    // After parameter substitution, evaluate %(expression) syntax
    result = evaluateExpressions(result, expansion);
    
    // Substitute local labels
    // We need to replace local label references with unique names
    // This is done by finding labels that match the local label list
    const MacroDefinition* macro = expansion.definition;
    for (const std::string& localLabel : macro->localLabels) {
        std::string uniqueLabel = generateLocalLabel(localLabel, expansion.uniqueId);
        
        // Replace all occurrences of the local label
        // We need to be careful to only replace whole words
        size_t pos = 0;
        while ((pos = result.find(localLabel, pos)) != std::string::npos) {
            // Check if it's a whole word (not part of another identifier)
            bool isWholeWord = true;
            
            if (pos > 0) {
                char before = result[pos - 1];
                if (std::isalnum(before) || before == '_' || before == '?' || before == '@') {
                    isWholeWord = false;
                }
            }
            
            if (isWholeWord && pos + localLabel.length() < result.length()) {
                char after = result[pos + localLabel.length()];
                if (std::isalnum(after) || after == '_' || after == '?' || after == '@') {
                    isWholeWord = false;
                }
            }
            
            if (isWholeWord) {
                result.replace(pos, localLabel.length(), uniqueLabel);
                pos += uniqueLabel.length();
            } else {
                pos++;
            }
        }
    }
    
    return result;
}

std::string MacroProcessor::substituteParameters(const std::string& line, const MacroExpansion& expansion) {
    std::string result;
    result.reserve(line.length() * 2); // Reserve extra space
    
    const MacroDefinition* macro = expansion.definition;
    
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == '&' && i + 1 < line.length()) {
            // Found potential parameter reference
            // Extract parameter name
            size_t start = i + 1;
            size_t end = start;
            
            // Parameter name follows same rules as identifiers
            while (end < line.length() && 
                   (std::isalnum(line[end]) || line[end] == '_' || line[end] == '?' || line[end] == '@')) {
                end++;
            }
            
            if (end > start) {
                std::string paramName = line.substr(start, end - start);
                
                // Find parameter in macro definition
                bool found = false;
                
                // For user-defined macros, check parameter list
                if (macro->type == MacroType::UserDefined) {
                    for (size_t p = 0; p < macro->parameters.size(); p++) {
                        if (macro->parameters[p] == paramName) {
                            // Substitute with actual argument
                            result += expansion.arguments[p];
                            i = end - 1; // Skip to end of parameter name
                            found = true;
                            break;
                        }
                    }
                }
                // For IRP/IRPC, check iterator name
                else if (macro->type == MacroType::IndefiniteRP || macro->type == MacroType::IndefiniteRPC) {
                    if (paramName == macro->iteratorName) {
                        // Substitute with current iterator value
                        result += expansion.arguments[0];
                        i = end - 1;
                        found = true;
                    }
                }
                
                if (!found) {
                    // Not a parameter, keep as is
                    result += '&';
                    result += paramName;
                    i = end - 1;
                }
            } else {
                // Just a bare '&'
                result += '&';
            }
        } else {
            result += line[i];
        }
    }
    
    return result;
}

std::string MacroProcessor::generateLocalLabel(const std::string& /* label */, int uniqueId) {
    // Generate format like "??0001", "??0002", etc.
    std::ostringstream oss;
    oss << "??" << std::setfill('0') << std::setw(4) << uniqueId;
    return oss.str();
}

bool MacroProcessor::isLocalLabel(const std::string& label, const MacroExpansion& expansion) {
    const MacroDefinition* macro = expansion.definition;
    return std::find(macro->localLabels.begin(), macro->localLabels.end(), label) 
           != macro->localLabels.end();
}

std::string MacroProcessor::evaluateExpressions(const std::string& line, const MacroExpansion& expansion) {
    std::string result;
    result.reserve(line.length() * 2);
    
    const MacroDefinition* macro = expansion.definition;
    
    for (size_t i = 0; i < line.length(); i++) {
        // Check for %(expression) syntax for expression evaluation
        if (line[i] == '%' && i + 1 < line.length() && line[i+1] == '(') {
            // Find matching closing parenthesis
            size_t start = i + 2;
            size_t end = start;
            int parenDepth = 1;
            
            while (end < line.length() && parenDepth > 0) {
                if (line[end] == '(') {
                    parenDepth++;
                } else if (line[end] == ')') {
                    parenDepth--;
                }
                if (parenDepth > 0) {
                    end++;
                }
            }
            
            if (parenDepth == 0 && end > start) {
                // Extract expression
                std::string expr = line.substr(start, end - start);
                
                // Check if expression is a parameter name (without &), and substitute it
                // M80 treats %(param) the same as %(&param)
                bool isParameter = false;
                for (size_t p = 0; p < macro->parameters.size(); p++) {
                    if (macro->parameters[p] == expr) {
                        // Replace with actual argument value
                        expr = expansion.arguments[p];
                        isParameter = true;
                        break;
                    }
                }
                
                // Check for IRP/IRPC iterator
                if (!isParameter && (macro->type == MacroType::IndefiniteRP || macro->type == MacroType::IndefiniteRPC)) {
                    if (macro->iteratorName == expr) {
                        expr = expansion.arguments[0];
                        isParameter = true;
                    }
                }
                
                // Now evaluate the expression
                int64_t value = 0;
                bool evaluated = false;
                
                // First try to parse as a simple number
                int base;
                if (parseNumber(expr, value, base)) {
                    evaluated = true;
                } else if (expressionEvaluator_) {
                    // Try full expression evaluation (may work in Pass 1/2 with symbols)
                    value = expressionEvaluator_(expr);
                    evaluated = true;
                }
                
                if (evaluated) {
                    // Convert to string (use hex for now, can be improved with RADIX support)
                    std::ostringstream oss;
                    oss << std::hex << std::uppercase << value;
                    result += oss.str();
                } else {
                    // Can't evaluate, output empty (M80 behavior when can't evaluate)
                    result += "";
                }
                
                i = end; // Skip past the closing parenthesis
                continue;
            }
        }
        
        result += line[i];
    }
    
    return result;
}


} // namespace z80
