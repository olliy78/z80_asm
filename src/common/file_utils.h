/**
 * @file file_utils.h
 * @brief File format utilities for handling CP/M and other legacy formats
 * 
 * Provides utilities for reading and processing files from legacy systems
 * like CP/M, which use different line endings and control character conventions.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>
#include <vector>

namespace z80 {
namespace fileutils {

/**
 * @brief Splits a line at carriage return (CR) characters not followed by line feed
 * 
 * CP/M files sometimes use CR alone as line separator instead of CR+LF.
 * This function detects such cases and splits them into separate lines.
 * 
 * @param line Input line that may contain embedded CR characters
 * @return Vector of lines split at standalone CR characters
 */
std::vector<std::string> splitAtCR(const std::string& line);

/**
 * @brief Removes CP/M control characters from a line
 * 
 * CP/M files may contain control characters (0x00-0x1F and 0x80-0x9F) that need
 * to be filtered out. This function:
 * - Preserves TAB, CR, and LF characters
 * - Removes high control characters (0x80-0x9F) at line start or after CR
 * - Removes other low control characters
 * 
 * @param line Input line with potential control characters
 * @return Cleaned line with control characters removed
 */
std::string cleanCpmLine(const std::string& line);

} // namespace fileutils
} // namespace z80
