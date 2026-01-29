#pragma once

#include <string>
#include <vector>

namespace z80 {

// String utilities
std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);
std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);

// Number parsing
bool parseNumber(const std::string& str, int64_t& value, int& base);

// File utilities
bool fileExists(const std::string& path);
std::string getBaseName(const std::string& path);
std::string getExtension(const std::string& path);
std::string changeExtension(const std::string& path, const std::string& newExt);

} // namespace z80
