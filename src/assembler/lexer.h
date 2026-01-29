/**
 * @file lexer.h
 * @brief Lexical analyzer for Z80 assembly source code
 * 
 * The lexer performs tokenization of assembly source code, breaking it
 * into tokens such as identifiers, numbers, operators, and directives.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include "common/types.h"
#include <string>
#include <vector>

namespace z80 {

/**
 * @class Lexer
 * @brief Lexical analyzer for Z80 assembly language
 * 
 * The Lexer class tokenizes Z80 assembly source code. It recognizes:
 * - Identifiers and labels
 * - Numbers in various formats (hex, binary, octal, decimal)
 * - String literals
 * - Operators and delimiters
 * - Directives (starting with .)
 * - Comments (starting with ;)
 * 
 * Example usage:
 * @code
 * Lexer lexer(sourceCode, "file.mac");
 * Token token = lexer.nextToken();
 * while (token.type != TokenType::EndOfFile) {
 *     // Process token
 *     token = lexer.nextToken();
 * }
 * @endcode
 */
class Lexer {
public:
    /**
     * @brief Construct a new Lexer object
     * @param source Source code to tokenize
     * @param filename Name of the source file (for error reporting)
     */
    Lexer(const std::string& source, const std::string& filename = "");
    
    /**
     * @brief Get the next token from the input
     * @return Next token, or EndOfFile token if at end
     */
    Token nextToken();
    
    /**
     * @brief Peek at the next token without consuming it
     * @return Next token (can be called multiple times)
     */
    Token peekToken();
    
    /**
     * @brief Get current line number
     * @return Current line number (1-based)
     */
    int getCurrentLine() const { return line_; }
    
    /**
     * @brief Get current column number
     * @return Current column number (1-based)
     */
    int getCurrentColumn() const { return column_; }
    
private:
    /** @brief Skip whitespace characters (space, tab) */
    void skipWhitespace();
    
    /** @brief Skip comment until end of line */
    void skipComment();
    
    /** @brief Read an identifier or keyword */
    Token readIdentifier();
    
    /** @brief Read a numeric literal */
    Token readNumber();
    
    /**
     * @brief Read a string literal
     * @param quote Quote character (' or ")
     */
    Token readString(char quote);
    
    /**
     * @brief Peek at character at offset from current position
     * @param offset Offset from current position (default 0)
     * @return Character at offset, or '\0' if past end
     */
    char peek(int offset = 0) const;
    
    /**
     * @brief Advance to next character and return current
     * @return Current character before advancing
     */
    char advance();
    
    /**
     * @brief Check if at end of input
     * @return true if at end of input
     */
    bool isAtEnd() const;
    
    std::string source_;     ///< Source code being tokenized
    std::string filename_;   ///< Filename for error reporting
    size_t position_;        ///< Current position in source
    int line_;               ///< Current line number (1-based)
    int column_;             ///< Current column number (1-based)
    Token peeked_;           ///< Cached peeked token
    bool hasPeeked_;         ///< True if peeked token is valid
};

} // namespace z80
