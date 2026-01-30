/**
 * @file test_file_io.cpp
 * @brief Quick test for file I/O issues
 */

#include <iostream>
#include <fstream>
#include <vector>

bool createTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Failed to create: " << filename << std::endl;
        return false;
    }
    
    for (const auto& line : lines) {
        file << line << "\n";
    }
    
    std::cout << "Created: " << filename << std::endl;
    return true;
}

bool readTestFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return false;
    }
    
    std::cout << "Successfully opened: " << filename << std::endl;
    std::string line;
    while (std::getline(file, line)) {
        std::cout << "  > " << line << std::endl;
    }
    
    return true;
}

int main() {
    std::cout << "=== File I/O Test ===" << std::endl;
    
    std::vector<std::string> testData = {
        "        ORG 0",
        "START:  NOP",
        "        END"
    };
    
    std::string testFile = "/tmp/io_test.asm";
    
    if (!createTestFile(testFile, testData)) {
        return 1;
    }
    
    if (!readTestFile(testFile)) {
        return 1;
    }
    
    std::cout << "\n=== Test Passed ✓ ===" << std::endl;
    return 0;
}
