/**
 * @file test_real_program.cpp
 * @brief Test assembling real Z80 programs
 */

#include "assembler/assembler.h"
#include "assembler/output_writer.h"
#include <iostream>
#include <iomanip>

using namespace z80;

void printModuleInfo(const AssembledModule& module) {
    std::cout << "\n=== Module: " << module.moduleName << " ===" << std::endl;
    std::cout << "Relocatable: " << (module.isRelocatable ? "yes" : "no") << std::endl;
    std::cout << "CSEG size:   " << module.csegSize << " bytes" << std::endl;
    std::cout << "DSEG size:   " << module.dsegSize << " bytes" << std::endl;
    
    std::cout << "\nGenerated code:" << std::endl;
    int totalBytes = 0;
    for (const auto& line : module.lines) {
        if (line.code.empty()) continue;
        
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') 
                  << line.address << ": ";
        
        for (size_t i = 0; i < line.code.size() && i < 8; i++) {
            std::cout << std::setw(2) << (int)line.code[i] << " ";
        }
        if (line.code.size() > 8) {
            std::cout << "... (" << std::dec << line.code.size() << " bytes)";
        }
        std::cout << std::endl;
        totalBytes += line.code.size();
    }
    std::cout << std::dec << "Total: " << totalBytes << " bytes" << std::endl;
}

int main() {
    std::cout << "=== Testing Real Z80 Programs ===" << std::endl;
    
    // Test 1: Hello World
    std::cout << "\n--- Test 1: Hello World CP/M Program ---" << std::endl;
    
    Assembler assembler;
    auto module = assembler.assemble("../examples/hello.asm");
    
    if (!module) {
        std::cout << "\n❌ Assembly FAILED!" << std::endl;
        std::cout << "\nErrors:" << std::endl;
        for (const auto& err : assembler.getErrors()) {
            std::cout << "  Line " << err.line << ": " << err.message << std::endl;
        }
        
        std::cout << "\nThis is expected - missing instructions will be shown above." << std::endl;
        std::cout << "These need to be added to z80_instructions.cpp" << std::endl;
        return 1;
    }
    
    printModuleInfo(*module);
    
    // Generate outputs
    std::cout << "\n--- Generating Output Files ---" << std::endl;
    
    RELOutputWriter relWriter;
    if (relWriter.write(*module, "/tmp/hello.rel")) {
        std::cout << "✓ REL:     /tmp/hello.rel" << std::endl;
    }
    
    ListingOutputWriter lstWriter;
    if (lstWriter.write(*module, "/tmp/hello.lst")) {
        std::cout << "✓ Listing: /tmp/hello.lst" << std::endl;
    }
    
    HEXOutputWriter hexWriter;
    if (hexWriter.write(*module, "/tmp/hello.hex")) {
        std::cout << "✓ HEX:     /tmp/hello.hex" << std::endl;
    }
    
    std::cout << "\n=== Assembly Successful! ✓ ===" << std::endl;
    
    // Test 2: More instructions
    std::cout << "\n--- Test 2: More Z80 Instructions ---" << std::endl;
    
    auto module2 = assembler.assemble("../examples/test_more.asm");
    
    if (!module2) {
        std::cout << "\n❌ Assembly FAILED!" << std::endl;
        std::cout << "\nErrors:" << std::endl;
        for (const auto& err : assembler.getErrors()) {
            std::cout << "  Line " << err.line << ": " << err.message << std::endl;
        }
        std::cout << "\nSome instructions are not yet implemented." << std::endl;
        return 1;
    }
    
    printModuleInfo(*module2);
    std::cout << "\n=== All Tests Passed! ✓ ===" << std::endl;
    
    // Test 3: Conditional jumps
    std::cout << "\n--- Test 3: Conditional Jumps ---" << std::endl;
    
    auto module3 = assembler.assemble("../examples/test_cond.asm");
    
    if (!module3) {
        std::cout << "\n❌ Assembly FAILED!" << std::endl;
        std::cout << "\nErrors:" << std::endl;
        for (const auto& err : assembler.getErrors()) {
            std::cout << "  Line " << err.line << ": " << err.message << std::endl;
        }
        std::cout << "\nConditional jumps not yet implemented." << std::endl;
        return 1;
    }
    
    printModuleInfo(*module3);
    std::cout << "\n=== All Tests Passed! ✓ ===" << std::endl;
    
    return 0;
}
