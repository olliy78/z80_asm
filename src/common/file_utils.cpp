/**
 * @file file_utils.cpp
 * @brief Implementation of file format utilities
 */

#include "file_utils.h"

namespace z80 {
namespace fileutils {

std::vector<std::string> splitAtCR(const std::string& line) {
    std::vector<std::string> result;
    std::string current;
    
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == '\r') {
            // Check if next char is LF
            if (i + 1 < line.length() && line[i+1] == '\n') {
                // CR+LF - keep both, it's normal line ending
                current += line[i];
            } else {
                // CR without LF - treat as line separator
                result.push_back(current);
                current.clear();
            }
        } else if (line[i] == '\n') {
            // LF - end current line
            result.push_back(current);
            current.clear();
        } else {
            current += line[i];
        }
    }
    
    // Don't forget the last line if there's content
    if (!current.empty()) {
        result.push_back(current);
    }
    
    return result;
}

std::string cleanCpmLine(const std::string& line) {
    std::string cleaned;
    cleaned.reserve(line.length());
    
    bool afterCR = false; // Track if we're right after a CR
    
    for (size_t i = 0; i < line.length(); i++) {
        unsigned char c = static_cast<unsigned char>(line[i]);
        
        // Track CR (carriage return)
        if (c == '\r') {
            cleaned += line[i];
            afterCR = true;
            continue;
        }
        
        // Skip control characters at start of line OR after CR (common in CP/M files)
        // This includes 0x8A which appears before some labels
        if ((cleaned.empty() || afterCR) && c >= 0x80 && c <= 0x9F) {
            continue; // Skip high control characters at line start or after CR
        }
        
        afterCR = false; // Reset after first non-CR character
        
        // Skip low control characters (except TAB, CR, LF)
        if (c < 32 && c != '\t' && c != '\r' && c != '\n') {
            continue;
        }
        
        cleaned += line[i];
    }
    
    return cleaned;
}

} // namespace fileutils
} // namespace z80
