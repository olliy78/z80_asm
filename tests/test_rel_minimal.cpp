/**
 * @file test_rel_minimal.cpp
 * @brief Minimal end-to-end test for REL generation
 */

#include "assembler/parser.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>

using namespace z80;

void printHexDump(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "  Cannot open output file!" << std::endl;
        return;
    }
    
    std::cout << "  REL output: ";
    char byte;
    int count = 0;
    while (file.get(byte) && count < 20) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)(unsigned char)byte << " ";
        count++;
    }
    std::cout << std::dec << std::endl;
}

int main() {
    std::cout << "=== Minimal REL Generation Test ===" << std::endl << std::endl;
    
    // Create test source file
    std::string asmFile = "/tmp/minimal_test.asm";
    std::string relFile = "/tmp/minimal_test.rel";
    
    std::ofstream src(asmFile);
    if (!src) {
        std::cerr << "Failed to create source file" << std::endl;
        return 1;
    }
    
    src << "        ORG 0\n";
    src << "START:  DB 1,2,3\n";
    src << "        END\n";
    src.close();
    
    std::cout << "Created source file: " << asmFile << std::endl;
    
    // Assemble
    Parser parser;
    std::cout << "Assembling..." << std::endl;
    
    bool success = parser.assemble(asmFile);
    
    if (!success || parser.hasErrors()) {
        std::cout << "\nAssembly errors:" << std::endl;
        for (const auto& err : parser.getErrors()) {
            std::cout << "  Line " << err.line << ": " << err.message << std::endl;
        }
        
        if (!success) {
            std::cerr << "\nAssembly FAILED!" << std::endl;
            std::remove(asmFile.c_str());
            return 1;
        }
    }
    
    std::cout << "✓ Assembly successful" << std::endl;
    
    // Check generated code
    const auto& lines = parser.getLines();
    std::cout << "Generated " << lines.size() << " line(s) with code" << std::endl;
    
    int totalBytes = 0;
    for (const auto& line : lines) {
        totalBytes += line.code.size();
        std::cout << "  Line " << line.lineNumber << ": " << line.code.size() 
                  << " byte(s) at address " << std::hex << line.address << std::dec << std::endl;
    }
    std::cout << "Total: " << totalBytes << " bytes" << std::endl;
    
    // Generate .REL file
    std::cout << "\nGenerating .REL file..." << std::endl;
    success = parser.writeREL(relFile, "MINTEST");
    
    if (!success) {
        std::cerr << "REL generation FAILED!" << std::endl;
        std::remove(asmFile.c_str());
        return 1;
    }
    
    std::cout << "✓ .REL file generated: " << relFile << std::endl;
    printHexDump(relFile);
    
    // Cleanup
    std::remove(asmFile.c_str());
    std::remove(relFile.c_str());
    
    std::cout << "\n=== Test Passed ✓ ===" << std::endl;
    return 0;
}
