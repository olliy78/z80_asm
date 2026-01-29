#include "utils.h"
#include <algorithm>
#include <cctype>
#include <fstream>

namespace z80 {

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();
    
    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    
    return str.substr(start, end - start);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    tokens.push_back(str.substr(start));
    return tokens;
}

bool parseNumber(const std::string& str, int64_t& value, int& base) {
    if (str.empty()) {
        return false;
    }
    
    std::string numStr = str;
    base = 10;
    
    // Determine base and strip prefix/suffix
    if (numStr.length() >= 2) {
        // Hex: 0x1234, 1234h, 0FFh
        if (numStr[0] == '0' && (numStr[1] == 'x' || numStr[1] == 'X')) {
            base = 16;
            numStr = numStr.substr(2);
        } else if (numStr.back() == 'h' || numStr.back() == 'H') {
            base = 16;
            numStr.pop_back();
            // Add leading zero if starts with A-F
            if (std::isalpha(static_cast<unsigned char>(numStr[0]))) {
                numStr = "0" + numStr;
            }
        }
        // Binary: 0b1010, 1010b
        else if (numStr[0] == '0' && (numStr[1] == 'b' || numStr[1] == 'B')) {
            base = 2;
            numStr = numStr.substr(2);
        } else if (numStr.back() == 'b' || numStr.back() == 'B') {
            base = 2;
            numStr.pop_back();
        }
        // Octal: 0o777, 777o, 777q
        else if (numStr[0] == '0' && (numStr[1] == 'o' || numStr[1] == 'O')) {
            base = 8;
            numStr = numStr.substr(2);
        } else if (numStr.back() == 'o' || numStr.back() == 'O' ||
                   numStr.back() == 'q' || numStr.back() == 'Q') {
            base = 8;
            numStr.pop_back();
        }
        // Decimal: 1234, 1234d
        else if (numStr.back() == 'd' || numStr.back() == 'D') {
            base = 10;
            numStr.pop_back();
        }
    }
    
    // Parse the number
    try {
        value = std::stoll(numStr, nullptr, base);
        return true;
    } catch (...) {
        return false;
    }
}

bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string getBaseName(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    size_t lastDot = path.find_last_of('.');
    
    size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
    size_t end = (lastDot == std::string::npos || lastDot < start) 
                 ? path.length() : lastDot;
    
    return path.substr(start, end - start);
}

std::string getExtension(const std::string& path) {
    size_t lastDot = path.find_last_of('.');
    size_t lastSlash = path.find_last_of("/\\");
    
    if (lastDot == std::string::npos || 
        (lastSlash != std::string::npos && lastDot < lastSlash)) {
        return "";
    }
    
    return path.substr(lastDot);
}

std::string changeExtension(const std::string& path, const std::string& newExt) {
    size_t lastDot = path.find_last_of('.');
    size_t lastSlash = path.find_last_of("/\\");
    
    if (lastDot == std::string::npos || 
        (lastSlash != std::string::npos && lastDot < lastSlash)) {
        return path + newExt;
    }
    
    return path.substr(0, lastDot) + newExt;
}

} // namespace z80
