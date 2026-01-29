#pragma once

#include "common/types.h"
#include <string>
#include <vector>

namespace z80 {

class Lexer {
public:
    Lexer(const std::string& source, const std::string& filename = "");
    
    Token nextToken();
    Token peekToken();
    
    int getCurrentLine() const { return line_; }
    int getCurrentColumn() const { return column_; }
    
private:
    void skipWhitespace();
    void skipComment();
    Token readIdentifier();
    Token readNumber();
    Token readString(char quote);
    
    char peek(int offset = 0) const;
    char advance();
    bool isAtEnd() const;
    
    std::string source_;
    std::string filename_;
    size_t position_;
    int line_;
    int column_;
    Token peeked_;
    bool hasPeeked_;
};

} // namespace z80
