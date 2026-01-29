/**
 * @file errors.h
 * @brief Error handling and reporting for the assembler
 * 
 * Provides error collection, classification, and reporting
 * compatible with M80's error message format.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>
#include <vector>

namespace z80 {

/**
 * @enum ErrorLevel
 * @brief Severity levels for assembly errors
 */
enum class ErrorLevel {
    Warning,
    Error,
    Fatal,
};

/**
 * @struct AssemblyError
 * @brief Represents a single assembly error or warning
 */
struct AssemblyError {
    ErrorLevel level;      ///< Severity level
    std::string message;   ///< Error message text
    std::string filename;  ///< Source filename
    int line;              ///< Line number (1-based)
    int column;            ///< Column number (1-based)
};

/**
 * @class ErrorHandler
 * @brief Collects and reports assembly errors
 * 
 * Manages error and warning messages during assembly.
 * Provides formatted output compatible with M80.
 */
class ErrorHandler {
public:
    /** @brief Construct an error handler */
    ErrorHandler();
    
    /**
     * @brief Add an error or warning
     * @param level Severity level
     * @param message Error message
     * @param filename Source filename
     * @param line Line number
     * @param column Column number
     */
    void addError(ErrorLevel level, const std::string& message, 
                  const std::string& filename, int line, int column);
    
    /** @brief Print all errors to stderr */
    void printErrors() const;
    
    /**
     * @brief Get count of errors (excludes warnings)
     * @return Number of errors
     */
    int getErrorCount() const { return errorCount_; }
    
    /**
     * @brief Get count of warnings
     * @return Number of warnings
     */
    int getWarningCount() const { return warningCount_; }
    
private:
    std::vector<AssemblyError> errors_;  ///< Collection of errors/warnings
    int errorCount_;                     ///< Count of errors
    int warningCount_;                   ///< Count of warnings
};

} // namespace z80
