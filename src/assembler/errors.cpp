#include "errors.h"
#include <iostream>

namespace z80 {

ErrorHandler::ErrorHandler() : errorCount_(0), warningCount_(0) {
}

void ErrorHandler::addError(ErrorLevel level, const std::string& message,
                           const std::string& filename, int line, int column) {
    AssemblyError error;
    error.level = level;
    error.message = message;
    error.filename = filename;
    error.line = line;
    error.column = column;
    
    errors_.push_back(error);
    
    if (level == ErrorLevel::Error || level == ErrorLevel::Fatal) {
        ++errorCount_;
    } else if (level == ErrorLevel::Warning) {
        ++warningCount_;
    }
}

void ErrorHandler::printErrors() const {
    for (const auto& error : errors_) {
        const char* levelStr = "Unknown";
        switch (error.level) {
            case ErrorLevel::Warning: levelStr = "Warning"; break;
            case ErrorLevel::Error: levelStr = "Error"; break;
            case ErrorLevel::Fatal: levelStr = "Fatal"; break;
        }
        
        std::cerr << error.filename << "(" << error.line << "," << error.column 
                  << "): " << levelStr << ": " << error.message << "\n";
    }
}

} // namespace z80
