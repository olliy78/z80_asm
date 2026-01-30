/**
 * @file test_assembler_architecture.cpp
 * @brief Test for refactored architecture: Parser -> Assembler -> OutputWriter
 */

#include "assembler/assembler.h"
#include "assembler/output_writer.h"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace z80;

void createTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    for (const auto& line : lines) {
        file << line << "\n";
    }
}

void testNewArchitecture() {
    std::cout << "=== Testing Refactored Architecture ===" << std::endl << std::endl;
    
    // Create test source
    std::string sourceFile = "/tmp/arch_test.asm";
    createTestFile(sourceFile, {
        "        ORG 0",
        "START:  DB 1,2,3",
        "        DW 1234H",
        "        END"
    });
    
    std::cout << "1. Assembler phase:" << std::endl;
    Assembler assembler;
    auto module = assembler.assemble(sourceFile);
    
    if (!module) {
        std::cerr << "   Assembly FAILED!" << std::endl;
        for (const auto& err : assembler.getErrors()) {
            std::cerr << "   Error: " << err.message << std::endl;
        }
        assert(false);
    }
    
    std::cout << "   ✓ Assembly successful" << std::endl;
    std::cout << "   Module: " << module->moduleName << std::endl;
    std::cout << "   Lines with code: " << module->lines.size() << std::endl;
    std::cout << "   Relocatable: " << (module->isRelocatable ? "yes" : "no") << std::endl;
    std::cout << "   CSEG size: " << module->csegSize << std::endl;
    std::cout << "   DSEG size: " << module->dsegSize << std::endl;
    
    // Test REL output
    std::cout << "\n2. REL output phase:" << std::endl;
    std::string relFile = "/tmp/arch_test.rel";
    RELOutputWriter relWriter;
    
    bool success = relWriter.write(*module, relFile);
    assert(success);
    
    std::cout << "   ✓ REL file written: " << relFile << std::endl;
    
    // Verify file exists and has content
    std::ifstream checkFile(relFile, std::ios::binary);
    assert(checkFile.good());
    checkFile.seekg(0, std::ios::end);
    size_t fileSize = checkFile.tellg();
    std::cout << "   File size: " << fileSize << " bytes" << std::endl;
    assert(fileSize > 0);
    
    // Test Listing output (placeholder)
    std::cout << "\n3. Listing output phase:" << std::endl;
    std::string lstFile = "/tmp/arch_test.lst";
    ListingOutputWriter lstWriter;
    
    success = lstWriter.write(*module, lstFile);
    assert(success);
    std::cout << "   ✓ Listing file written: " << lstFile << std::endl;
    
    // Test HEX output (placeholder)
    std::cout << "\n4. HEX output phase:" << std::endl;
    std::string hexFile = "/tmp/arch_test.hex";
    HEXOutputWriter hexWriter;
    
    success = hexWriter.write(*module, hexFile);
    assert(success);
    std::cout << "   ✓ HEX file written: " << hexFile << std::endl;
    
    // Cleanup
    std::remove(sourceFile.c_str());
    std::remove(relFile.c_str());
    std::remove(lstFile.c_str());
    std::remove(hexFile.c_str());
    
    std::cout << "\n=== Architecture Test PASSED ✓ ===" << std::endl;
}

int main() {
    try {
        testNewArchitecture();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
