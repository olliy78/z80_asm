#pragma once

#include <string>
#include <vector>

namespace z80 {

enum class ErrorLevel {
    Warning,
    Error,
    Fatal,
};

struct AssemblyError {
    ErrorLevel level;
    std::string message;
    std::string filename;
    int line;
    int column;
};

class ErrorHandler {
public:
    ErrorHandler();
    
    void addError(ErrorLevel level, const std::string& message, 
                  const std::string& filename, int line, int column);
    
    void printErrors() const;
    
    int getErrorCount() const { return errorCount_; }
    int getWarningCount() const { return warningCount_; }
    
private:
    std::vector<AssemblyError> errors_;
    int errorCount_;
    int warningCount_;
};

} // namespace z80
