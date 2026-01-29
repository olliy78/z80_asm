/**
 * @file utils.h
 * @brief Utility functions for the Z80 assembler
 * 
 * This file contains various utility functions for string manipulation,
 * number parsing, and file operations used throughout the assembler.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>
#include <vector>

namespace z80 {

/**
 * @defgroup StringUtils String Utilities
 * @brief String manipulation functions
 * @{
 */

/**
 * @brief Convert string to uppercase
 * @param str Input string
 * @return Uppercase version of the string
 */
std::string toUpper(const std::string& str);

/**
 * @brief Convert string to lowercase
 * @param str Input string
 * @return Lowercase version of the string
 */
std::string toLower(const std::string& str);

/**
 * @brief Trim whitespace from both ends of string
 * @param str Input string
 * @return Trimmed string
 */
std::string trim(const std::string& str);

/**
 * @brief Split string by delimiter
 * @param str Input string
 * @param delimiter Character to split on
 * @return Vector of substrings
 */
std::vector<std::string> split(const std::string& str, char delimiter);

/** @} */

/**
 * @defgroup NumberUtils Number Parsing
 * @brief Number parsing utilities
 * @{
 */

/**
 * @brief Parse a number string in various formats
 * 
 * Supports the following formats:
 * - Hexadecimal: 0x1234, 1234h, 0ABh
 * - Binary: 0b1010, 1010b
 * - Octal: 0o777, 777o, 777q
 * - Decimal: 1234, 1234d
 * 
 * @param str String to parse
 * @param[out] value Parsed numeric value
 * @param[out] base Base used for parsing (2, 8, 10, or 16)
 * @return true if parsing succeeded, false otherwise
 */
bool parseNumber(const std::string& str, int64_t& value, int& base);

/** @} */

/**
 * @defgroup FileUtils File Utilities
 * @brief File path manipulation functions
 * @{
 */

/**
 * @brief Check if a file exists
 * @param path Path to check
 * @return true if file exists and is accessible
 */
bool fileExists(const std::string& path);

/**
 * @brief Get base name of file without extension
 * @param path File path
 * @return Base name without path and extension
 */
std::string getBaseName(const std::string& path);

/**
 * @brief Get file extension including the dot
 * @param path File path
 * @return Extension (e.g., ".mac") or empty string
 */
std::string getExtension(const std::string& path);

/**
 * @brief Change file extension
 * @param path Original file path
 * @param newExt New extension (should include the dot)
 * @return Path with new extension
 */
std::string changeExtension(const std::string& path, const std::string& newExt);

/** @} */

} // namespace z80
