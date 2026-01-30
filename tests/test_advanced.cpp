/**
 * @file test_advanced.cpp
 * @brief Test advanced Z80 instructions
 */

#include "assembler/assembler.h"
#include "assembler/output_writer.h"
#include <iostream>
#include <iomanip>

using namespace z80;

void printCode(const AssembledModule& module) {
    std::cout << "\n=== Generated Code ===" << std::endl;
    int totalBytes = 0;
    for (const auto& line : module.lines) {
        if (line.code.empty()) continue;
        
        std::cout << std::hex << std::setw(4) << std::setfill('0') 
                  << line.address << ": ";
        
        for (size_t i = 0; i < line.code.size(); i++) {
            std::cout << std::setw(2) << (int)line.code[i] << " ";
            totalBytes++;
        }
        std::cout << std::endl;
    }
    std::cout << "\nTotal: " << std::dec << totalBytes << " bytes" << std::endl;
}

int main() {
    std::cout << "=== Test Advanced Z80 Instructions ===" << std::endl;
    
    Assembler assembler;
    auto module = assembler.assemble("../examples/test_advanced.asm");
    
    if (!module) {
        std::cout << "\n❌ Assembly FAILED!" << std::endl;
        std::cout << "\nErrors:" << std::endl;
        for (const auto& error : assembler.getErrors()) {
            std::cout << "  Line " << error.line << ": " << error.message << std::endl;
        }
        return 1;
    }
    
    std::cout << "✓ Assembly successful!" << std::endl;
    printCode(*module);
    
    // Generate REL file
    RELOutputWriter relWriter;
    if (relWriter.write(*module, "/tmp/test_advanced.rel")) {
        std::cout << "\n✓ REL file: /tmp/test_advanced.rel" << std::endl;
    }
    
    return 0;
}
