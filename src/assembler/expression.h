#pragma once

#include <string>

namespace z80 {

class Expression {
public:
    Expression();
    
    // TODO: Expression evaluation
    int64_t evaluate(const std::string& expr);
};

} // namespace z80
