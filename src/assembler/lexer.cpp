/**
 * @file lexer.cpp
 * @brief Implementation of the lexical analyzer
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include "lexer.h"
#include "common/utils.h"
#include <cctype>

namespace z80 {

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename), position_(0), 
      line_(1), column_(1), hasPeeked_(false) {
}

Token Lexer::nextToken() {
    if (hasPeeked_) {
        hasPeeked_ = false;
        return peeked_;
    }
    
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TokenType::EndOfFile, "", line_, column_);
    }
    
    char c = peek();
    int startLine = line_;
    int startColumn = column_;
    
    // Comment
    if (c == ';') {
        skipComment();
        return nextToken(); // Skip to next token
    }
    
    // End of line
    if (c == '\n' || c == '\r') {
        advance();
        if (c == '\r' && peek() == '\n') {
            advance();
        }
        return Token(TokenType::EndOfLine, "", startLine, startColumn);
    }
    
    // String
    if (c == '\'' || c == '"') {
        return readString(c);
    }
    
    // Number
    if (std::isdigit(c)) {
        return readNumber();
    }
    
    // Identifier or keyword
    if (std::isalpha(c) || c == '_' || c == '.' || c == '@' || c == '?') {
        return readIdentifier();
    }
    
    // Single character tokens
    advance();
    switch (c) {
        case '+': return Token(TokenType::Plus, "+", startLine, startColumn);
        case '-': return Token(TokenType::Minus, "-", startLine, startColumn);
        case '*': return Token(TokenType::Multiply, "*", startLine, startColumn);
        case '/': return Token(TokenType::Divide, "/", startLine, startColumn);
        case ':': return Token(TokenType::Colon, ":", startLine, startColumn);
        case ',': return Token(TokenType::Comma, ",", startLine, startColumn);
        case '(': return Token(TokenType::LeftParen, "(", startLine, startColumn);
        case ')': return Token(TokenType::RightParen, ")", startLine, startColumn);
        case '[': return Token(TokenType::LeftBracket, "[", startLine, startColumn);
        case ']': return Token(TokenType::RightBracket, "]", startLine, startColumn);
        case '$': return Token(TokenType::Dollar, "$", startLine, startColumn);
    }
    
    // Unknown character
    return Token(TokenType::EndOfFile, std::string(1, c), startLine, startColumn);
}

Token Lexer::peekToken() {
    if (!hasPeeked_) {
        peeked_ = nextToken();
        hasPeeked_ = true;
    }
    return peeked_;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        // Skip spaces, tabs, and other control characters (except newlines)
        // This handles CP/M era files with form feeds and other control codes
        if (c == ' ' || c == '\t' || (c > 0 && c < 32 && c != '\n' && c != '\r')) {
            advance();
        } else {
            break;
        }
    }
}

void Lexer::skipComment() {
    while (!isAtEnd() && peek() != '\n' && peek() != '\r') {
        advance();
    }
}

Token Lexer::readIdentifier() {
    int startLine = line_;
    int startColumn = column_;
    std::string text;
    
    while (!isAtEnd()) {
        char c = peek();
        if (std::isalnum(c) || c == '_' || c == '.' || c == '@' || c == '?' || c == '\'') {
            text += advance();
        } else {
            break;
        }
    }
    
    // Check if it's a directive (starts with .)
    if (!text.empty() && text[0] == '.') {
        return Token(TokenType::Directive, text, startLine, startColumn);
    }
    
    // For now, classify as identifier
    // TODO: Check for mnemonics, registers, keywords
    return Token(TokenType::Identifier, text, startLine, startColumn);
}

Token Lexer::readNumber() {
    int startLine = line_;
    int startColumn = column_;
    std::string text;
    
    while (!isAtEnd()) {
        char c = peek();
        if (std::isalnum(c) || c == 'x' || c == 'X') {
            text += advance();
        } else {
            break;
        }
    }
    
    // Check for hex/binary/octal suffix
    char suffix = peek();
    if (suffix == 'h' || suffix == 'H' || suffix == 'b' || suffix == 'B' ||
        suffix == 'o' || suffix == 'O' || suffix == 'q' || suffix == 'Q' ||
        suffix == 'd' || suffix == 'D') {
        text += advance();
    }
    
    Token token(TokenType::Number, text, startLine, startColumn);
    
    // Parse the number
    int base;
    if (!parseNumber(text, token.numValue, base)) {
        // Error parsing number, leave as 0
        token.numValue = 0;
    }
    
    return token;
}

Token Lexer::readString(char quote) {
    int startLine = line_;
    int startColumn = column_;
    std::string text;
    
    advance(); // Skip opening quote
    
    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\n' || peek() == '\r') {
            // Unterminated string
            break;
        }
        text += advance();
    }
    
    if (!isAtEnd() && peek() == quote) {
        advance(); // Skip closing quote
    }
    
    return Token(TokenType::String, text, startLine, startColumn);
}

char Lexer::peek(int offset) const {
    size_t pos = position_ + offset;
    if (pos >= source_.length()) {
        return '\0';
    }
    return source_[pos];
}

char Lexer::advance() {
    if (isAtEnd()) {
        return '\0';
    }
    
    char c = source_[position_++];
    
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    
    return c;
}

bool Lexer::isAtEnd() const {
    return position_ >= source_.length();
}

} // namespace z80
