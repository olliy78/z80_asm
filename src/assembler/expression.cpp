/**
 * @file expression.cpp
 * @brief Implementation of expression evaluator
 */

#include "expression.h"
#include "common/utils.h"
#include <cctype>

namespace z80 {

ExpressionEvaluator::ExpressionEvaluator(const SymbolTable& symbolTable,
                                         Address locationCounter,
                                         SegmentType currentSegment)
    : symbolTable_(symbolTable)
    , locationCounter_(locationCounter)
    , currentSegment_(currentSegment)
{
}

ExpressionResult ExpressionEvaluator::evaluate(const std::string& expression) {
    if (expression.empty()) {
        ExpressionResult result;
        result.valid = false;
        result.errorMessage = "Empty expression";
        return result;
    }
    
    size_t pos = 0;
    skipWhitespace(expression, pos);
    
    ExpressionResult result = parseOrExpression(expression, pos);
    
    skipWhitespace(expression, pos);
    if (result.valid && pos < expression.length()) {
        result.valid = false;
        result.errorMessage = "Unexpected characters after expression";
    }
    
    return result;
}

void ExpressionEvaluator::skipWhitespace(const std::string& expr, size_t& pos) {
    while (pos < expr.length() && std::isspace(expr[pos])) {
        pos++;
    }
}

bool ExpressionEvaluator::matchKeyword(const std::string& expr, size_t& pos, const std::string& keyword) {
    skipWhitespace(expr, pos);
    
    if (pos + keyword.length() > expr.length()) {
        return false;
    }
    
    for (size_t i = 0; i < keyword.length(); i++) {
        if (std::toupper(expr[pos + i]) != std::toupper(keyword[i])) {
            return false;
        }
    }
    
    // Check that keyword is not part of a longer identifier
    size_t endPos = pos + keyword.length();
    if (endPos < expr.length() && (std::isalnum(expr[endPos]) || expr[endPos] == '_')) {
        return false;
    }
    
    pos = endPos;
    return true;
}

bool ExpressionEvaluator::matchOperator(const std::string& expr, size_t& pos, char op) {
    skipWhitespace(expr, pos);
    if (pos < expr.length() && expr[pos] == op) {
        pos++;
        return true;
    }
    return false;
}

std::string ExpressionEvaluator::parseIdentifier(const std::string& expr, size_t& pos) {
    skipWhitespace(expr, pos);
    
    if (pos >= expr.length() || (!std::isalpha(expr[pos]) && expr[pos] != '_' && expr[pos] != '.' && expr[pos] != '@' && expr[pos] != '?')) {
        return "";
    }
    
    size_t start = pos;
    while (pos < expr.length() && (std::isalnum(expr[pos]) || expr[pos] == '_' || expr[pos] == '.' || expr[pos] == '@' || expr[pos] == '?' || expr[pos] == '\'')) {
        pos++;
    }
    
    return expr.substr(start, pos - start);
}

ExpressionResult ExpressionEvaluator::parseNumber(const std::string& expr, size_t& pos) {
    skipWhitespace(expr, pos);
    
    if (pos >= expr.length() || !std::isdigit(expr[pos])) {
        ExpressionResult result;
        result.valid = false;
        result.errorMessage = "Expected number";
        return result;
    }
    
    size_t start = pos;
    while (pos < expr.length() && (std::isxdigit(expr[pos]) || 
                                   expr[pos] == 'H' || expr[pos] == 'h' ||
                                   expr[pos] == 'O' || expr[pos] == 'o' ||
                                   expr[pos] == 'B' || expr[pos] == 'b')) {
        pos++;
    }
    
    std::string numStr = expr.substr(start, pos - start);
    int base = 10;
    int64_t value = 0;
    
    if (z80::parseNumber(numStr, value, base)) {
        return ExpressionResult(value, ExpressionType::Absolute);
    }
    
    ExpressionResult result;
    result.valid = false;
    result.errorMessage = "Invalid number format: " + numStr;
    return result;
}

ExpressionResult ExpressionEvaluator::parseOrExpression(const std::string& expr, size_t& pos) {
    ExpressionResult left = parseXorExpression(expr, pos);
    if (!left.valid) return left;
    
    while (true) {
        size_t savedPos = pos;
        if (matchKeyword(expr, pos, "OR")) {
            ExpressionResult right = parseXorExpression(expr, pos);
            if (!right.valid) return right;
            
            left.value = left.value | right.value;
            left.type = combineTypes(left.type, right.type, '|');
        } else {
            pos = savedPos;
            break;
        }
    }
    
    return left;
}

ExpressionResult ExpressionEvaluator::parseXorExpression(const std::string& expr, size_t& pos) {
    ExpressionResult left = parseAndExpression(expr, pos);
    if (!left.valid) return left;
    
    while (true) {
        size_t savedPos = pos;
        if (matchKeyword(expr, pos, "XOR")) {
            ExpressionResult right = parseAndExpression(expr, pos);
            if (!right.valid) return right;
            
            left.value = left.value ^ right.value;
            left.type = combineTypes(left.type, right.type, '^');
        } else {
            pos = savedPos;
            break;
        }
    }
    
    return left;
}

ExpressionResult ExpressionEvaluator::parseAndExpression(const std::string& expr, size_t& pos) {
    ExpressionResult left = parseRelationalExpression(expr, pos);
    if (!left.valid) return left;
    
    while (true) {
        size_t savedPos = pos;
        if (matchKeyword(expr, pos, "AND")) {
            ExpressionResult right = parseRelationalExpression(expr, pos);
            if (!right.valid) return right;
            
            left.value = left.value & right.value;
            left.type = combineTypes(left.type, right.type, '&');
        } else {
            pos = savedPos;
            break;
        }
    }
    
    return left;
}

ExpressionResult ExpressionEvaluator::parseRelationalExpression(const std::string& expr, size_t& pos) {
    ExpressionResult left = parseAdditiveExpression(expr, pos);
    if (!left.valid) return left;
    
    size_t savedPos = pos;
    
    if (matchKeyword(expr, pos, "EQ")) {
        ExpressionResult right = parseAdditiveExpression(expr, pos);
        if (!right.valid) return right;
        left.value = (left.value == right.value) ? 1 : 0;
        left.type = ExpressionType::Absolute;
    } else if (matchKeyword(expr, pos, "NE")) {
        ExpressionResult right = parseAdditiveExpression(expr, pos);
        if (!right.valid) return right;
        left.value = (left.value != right.value) ? 1 : 0;
        left.type = ExpressionType::Absolute;
    } else if (matchKeyword(expr, pos, "LE")) {
        ExpressionResult right = parseAdditiveExpression(expr, pos);
        if (!right.valid) return right;
        left.value = (left.value <= right.value) ? 1 : 0;
        left.type = ExpressionType::Absolute;
    } else if (matchKeyword(expr, pos, "LT")) {
        ExpressionResult right = parseAdditiveExpression(expr, pos);
        if (!right.valid) return right;
        left.value = (left.value < right.value) ? 1 : 0;
        left.type = ExpressionType::Absolute;
    } else if (matchKeyword(expr, pos, "GE")) {
        ExpressionResult right = parseAdditiveExpression(expr, pos);
        if (!right.valid) return right;
        left.value = (left.value >= right.value) ? 1 : 0;
        left.type = ExpressionType::Absolute;
    } else if (matchKeyword(expr, pos, "GT")) {
        ExpressionResult right = parseAdditiveExpression(expr, pos);
        if (!right.valid) return right;
        left.value = (left.value > right.value) ? 1 : 0;
        left.type = ExpressionType::Absolute;
    } else {
        pos = savedPos;
    }
    
    return left;
}

ExpressionResult ExpressionEvaluator::parseAdditiveExpression(const std::string& expr, size_t& pos) {
    ExpressionResult left = parseMultiplicativeExpression(expr, pos);
    if (!left.valid) return left;
    
    while (true) {
        skipWhitespace(expr, pos);
        if (pos >= expr.length()) break;
        
        if (matchOperator(expr, pos, '+')) {
            ExpressionResult right = parseMultiplicativeExpression(expr, pos);
            if (!right.valid) return right;
            left.value = left.value + right.value;
            left.type = combineTypes(left.type, right.type, '+');
        } else if (matchOperator(expr, pos, '-')) {
            ExpressionResult right = parseMultiplicativeExpression(expr, pos);
            if (!right.valid) return right;
            left.value = left.value - right.value;
            left.type = combineTypes(left.type, right.type, '-');
        } else {
            break;
        }
    }
    
    return left;
}

ExpressionResult ExpressionEvaluator::parseMultiplicativeExpression(const std::string& expr, size_t& pos) {
    ExpressionResult left = parseUnaryExpression(expr, pos);
    if (!left.valid) return left;
    
    while (true) {
        size_t savedPos = pos;
        
        if (matchOperator(expr, pos, '*')) {
            ExpressionResult right = parseUnaryExpression(expr, pos);
            if (!right.valid) return right;
            left.value = left.value * right.value;
            left.type = combineTypes(left.type, right.type, '*');
        } else if (matchOperator(expr, pos, '/')) {
            ExpressionResult right = parseUnaryExpression(expr, pos);
            if (!right.valid) return right;
            if (right.value == 0) {
                left.valid = false;
                left.errorMessage = "Division by zero";
                return left;
            }
            left.value = left.value / right.value;
            left.type = combineTypes(left.type, right.type, '/');
        } else if (matchKeyword(expr, pos, "MOD")) {
            ExpressionResult right = parseUnaryExpression(expr, pos);
            if (!right.valid) return right;
            if (right.value == 0) {
                left.valid = false;
                left.errorMessage = "Modulo by zero";
                return left;
            }
            left.value = left.value % right.value;
            left.type = combineTypes(left.type, right.type, '%');
        } else if (matchKeyword(expr, pos, "SHL")) {
            ExpressionResult right = parseUnaryExpression(expr, pos);
            if (!right.valid) return right;
            left.value = left.value << right.value;
            left.type = combineTypes(left.type, right.type, '<');
        } else if (matchKeyword(expr, pos, "SHR")) {
            ExpressionResult right = parseUnaryExpression(expr, pos);
            if (!right.valid) return right;
            left.value = left.value >> right.value;
            left.type = combineTypes(left.type, right.type, '>');
        } else {
            pos = savedPos;
            break;
        }
    }
    
    return left;
}

ExpressionResult ExpressionEvaluator::parseUnaryExpression(const std::string& expr, size_t& pos) {
    skipWhitespace(expr, pos);
    
    if (matchOperator(expr, pos, '+')) {
        return parseUnaryExpression(expr, pos);
    } else if (matchOperator(expr, pos, '-')) {
        ExpressionResult result = parseUnaryExpression(expr, pos);
        if (result.valid) {
            result.value = -result.value;
        }
        return result;
    } else {
        size_t savedPos = pos;
        if (matchKeyword(expr, pos, "NOT")) {
            ExpressionResult result = parseUnaryExpression(expr, pos);
            if (result.valid) {
                result.value = ~result.value;
            }
            return result;
        }
        pos = savedPos;
    }
    
    return parsePrimaryExpression(expr, pos);
}

ExpressionResult ExpressionEvaluator::parsePrimaryExpression(const std::string& expr, size_t& pos) {
    skipWhitespace(expr, pos);
    
    if (pos >= expr.length()) {
        ExpressionResult result;
        result.valid = false;
        result.errorMessage = "Unexpected end of expression";
        return result;
    }
    
    // Parentheses
    if (matchOperator(expr, pos, '(')) {
        ExpressionResult result = parseOrExpression(expr, pos);
        if (!result.valid) return result;
        
        if (!matchOperator(expr, pos, ')')) {
            result.valid = false;
            result.errorMessage = "Missing closing parenthesis";
            return result;
        }
        return result;
    }
    
    // Location counter $
    if (matchOperator(expr, pos, '$')) {
        ExpressionType type = (currentSegment_ == SegmentType::ASEG) 
                              ? ExpressionType::Absolute 
                              : ExpressionType::Relocatable;
        return ExpressionResult(locationCounter_, type);
    }
    
    // Character literal 'X'
    if (expr[pos] == '\'') {
        size_t startPos = pos;
        pos++; // Skip opening quote
        
        if (pos >= expr.length()) {
            ExpressionResult result;
            result.valid = false;
            result.errorMessage = "Unterminated character literal";
            return result;
        }
        
        // Get the character value
        int value = static_cast<unsigned char>(expr[pos]);
        pos++; // Move past the character
        
        // Check for closing quote
        if (pos >= expr.length() || expr[pos] != '\'') {
            ExpressionResult result;
            result.valid = false;
            result.errorMessage = "Unterminated character literal";
            return result;
        }
        pos++; // Skip closing quote
        
        return ExpressionResult(value, ExpressionType::Absolute);
    }
    
    // Number
    if (std::isdigit(expr[pos])) {
        return parseNumber(expr, pos);
    }
    
    // Identifier (symbol)
    if (std::isalpha(expr[pos]) || expr[pos] == '_' || expr[pos] == '.' || expr[pos] == '@' || expr[pos] == '?') {
        std::string identifier = parseIdentifier(expr, pos);
        
        if (!symbolTable_.hasSymbol(identifier)) {
            ExpressionResult result;
            result.valid = false;
            result.errorMessage = "Undefined symbol: " + identifier;
            return result;
        }
        
        const Symbol* symbol = symbolTable_.getSymbol(identifier);
        
        // Determine type based on symbol properties
        ExpressionType type;
        if (symbol->isExternal) {
            type = ExpressionType::External;
        } else if (symbol->isRelocatable) {
            type = ExpressionType::Relocatable;
        } else {
            type = ExpressionType::Absolute;
        }
        
        return ExpressionResult(symbol->value, type);
    }
    
    ExpressionResult result;
    result.valid = false;
    result.errorMessage = "Unexpected character: " + std::string(1, expr[pos]);
    return result;
}

ExpressionType ExpressionEvaluator::combineTypes(ExpressionType left, ExpressionType right, char op) {
    // External references propagate
    if (left == ExpressionType::External || right == ExpressionType::External) {
        return ExpressionType::External;
    }
    
    // Addition/subtraction rules for relocatable addresses
    if (op == '+') {
        // Can't add two relocatable addresses
        if (left == ExpressionType::Relocatable && right == ExpressionType::Relocatable) {
            // This is actually an error, but we'll let it slide for now
            return ExpressionType::Relocatable;
        }
        // Relocatable + Absolute = Relocatable
        if (left == ExpressionType::Relocatable || right == ExpressionType::Relocatable) {
            return ExpressionType::Relocatable;
        }
    } else if (op == '-') {
        // Relocatable - Relocatable = Absolute (offset)
        if (left == ExpressionType::Relocatable && right == ExpressionType::Relocatable) {
            return ExpressionType::Absolute;
        }
        // Relocatable - Absolute = Relocatable
        if (left == ExpressionType::Relocatable) {
            return ExpressionType::Relocatable;
        }
    }
    
    // Most operations on absolute values yield absolute
    return ExpressionType::Absolute;
}

} // namespace z80
