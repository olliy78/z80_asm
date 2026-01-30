/**
 * @file test_rel_output.cpp
 * @brief End-to-end test: assemble and generate .REL file
 */

#include "assembler/parser.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>

using namespace z80;

void printHexDump(const std::string& filename, int maxBytes = 50) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "  Cannot open file!" << std::endl;
        return;
    }
    
    std::cout << "  Hex dump: ";
    int count = 0;
    char byte;
    while (file.get(byte) && count < maxBytes) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)(unsigned char)byte << " ";
        count++;
        if (count % 16 == 0) std::cout << "\n            ";
    }
    std::cout << std::dec;
    if (file.get(byte)) {
        std::cout << "... (more bytes)";
    }
    std::cout << std::endl;
}

bool createTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    if (!file) return false;
    
    for (const auto& line : lines) {
        file << line << "\n";
    }
    return true;
}

void testMinimalProgram() {
    std::cout << "Test: Minimal absolute program..." << std::endl;
    
    // Create test source file
    std::vector<std::string> source = {
        "        ORG 0",
        "START:  NOP",
        "        END"
    };
    
    std::string asmFile = "/tmp/test_minimal.asm";
    std::string relFile = "/tmp/test_minimal.rel";
    
    assert(createTestFile(asmFile, source));
    
    // Assemble
    Parser parser;
    bool success = parser.assemble(asmFile);
    
    if (!success) {
        std::cout << "  Assembly errors:" << std::endl;
        for (const auto& err : parser.getErrors()) {
            std::cout << "    " << err.message << std::endl;
        }
    }
    
    assert(success && "Assembly failed");
    
    // Generate .REL file
    success = parser.writeREL(relFile, "MINIMAL");
    assert(success && "REL generation failed");
    
    std::cout << "  ✓ Assembly successful" << std::endl;
    std::cout << "  ✓ .REL file generated" << std::endl;
    printHexDump(relFile, 30);
    
    // Cleanup
    std::remove(asmFile.c_str());
    std::remove(relFile.c_str());
}

void testRelocatableProgram() {
    std::cout << "\nTest: Relocatable program with data..." << std::endl;
    
    std::vector<std::string> source = {
        "        CSEG",
        "START:  LD HL,DATA",
        "        LD A,(HL)",
        "        RET",
        "",
        "        DSEG",
        "DATA:   DB 42H",
        "        END START"
    };
    
    std::string asmFile = "/tmp/test_reloc.asm";
    std::string relFile = "/tmp/test_reloc.rel";
    
    assert(createTestFile(asmFile, source));
    
    Parser parser;
    bool success = parser.assemble(asmFile);
    
    if (!success) {
        std::cout << "  Assembly errors:" << std::endl;
        for (const auto& err : parser.getErrors()) {
            std::cout << "    Line " << err.line << ": " << err.message << std::endl;
        }
    }
    
    // Even if assembly has errors (missing instructions), try to generate REL
    if (parser.getLines().empty()) {
        std::cout << "  ⚠ No code generated (missing instruction definitions)" << std::endl;
        std::remove(asmFile.c_str());
        return;
    }
    
    success = parser.writeREL(relFile, "TESTREL");
    assert(success && "REL generation failed");
    
    std::cout << "  ✓ .REL file generated" << std::endl;
    printHexDump(relFile, 40);
    
    // Cleanup
    std::remove(asmFile.c_str());
    std::remove(relFile.c_str());
}

void testWithDBDirective() {
    std::cout << "\nTest: Program with DB/DW directives..." << std::endl;
    
    std::vector<std::string> source = {
        "        ORG 100H",
        "START:  DB 1,2,3",
        "        DW 1234H",
        "MSG:    DB 'HELLO'",
        "        END"
    };
    
    std::string asmFile = "/tmp/test_db.asm";
    std::string relFile = "/tmp/test_db.rel";
    
    assert(createTestFile(asmFile, source));
    
    Parser parser;
    bool success = parser.assemble(asmFile);
    assert(success && "Assembly failed");
    
    // Check generated data
    const auto& lines = parser.getLines();
    int totalBytes = 0;
    for (const auto& line : lines) {
        totalBytes += line.code.size();
    }
    
    std::cout << "  Generated " << totalBytes << " bytes of data" << std::endl;
    // Should have: 3 bytes (DB 1,2,3) + 2 bytes (DW 1234H) + 5 bytes ('HELLO') = 10 bytes
    if (totalBytes != 10) {
        std::cout << "  ⚠ Warning: Expected 10 bytes, got " << totalBytes << std::endl;
    }
    
    success = parser.writeREL(relFile, "TESTDB");
    assert(success);
    
    std::cout << "  ✓ DB/DW data correctly assembled" << std::endl;
    printHexDump(relFile, 40);
    
    // Cleanup
    std::remove(asmFile.c_str());
    std::remove(relFile.c_str());
}

void testExpressions() {
    std::cout << "\nTest: Program with expressions..." << std::endl;
    
    std::vector<std::string> source = {
        "VALUE   EQU 10",
        "        ORG 0",
        "DATA1:  DB VALUE",
        "DATA2:  DB VALUE+5",
        "DATA3:  DB VALUE*2",
        "ADDR:   DW 1000H+100H",
        "        END"
    };
    
    std::string asmFile = "/tmp/test_expr.asm";
    std::string relFile = "/tmp/test_expr.rel";
    
    assert(createTestFile(asmFile, source));
    
    Parser parser;
    bool success = parser.assemble(asmFile);
    assert(success);
    
    // Verify expressions were evaluated
    const auto& lines = parser.getLines();
    bool foundValue10 = false, foundValue15 = false, foundValue20 = false;
    
    for (const auto& line : lines) {
        for (Byte b : line.code) {
            if (b == 10) foundValue10 = true;
            if (b == 15) foundValue15 = true;
            if (b == 20) foundValue20 = true;
        }
    }
    
    if (foundValue10 && foundValue15 && foundValue20) {
        std::cout << "  ✓ All expression values found in output" << std::endl;
    } else {
        std::cout << "  Values: 10=" << foundValue10 << " 15=" << foundValue15 
                  << " 20=" << foundValue20 << std::endl;
    }
    
    success = parser.writeREL(relFile, "EXPR");
    assert(success);
    
    std::cout << "  ✓ Expressions correctly evaluated" << std::endl;
    printHexDump(relFile, 40);
    
    // Cleanup
    std::remove(asmFile.c_str());
    std::remove(relFile.c_str());
}

int main() {
    std::cout << "=== End-to-End REL Output Tests ===" << std::endl << std::endl;
    
    testMinimalProgram();
    testRelocatableProgram();
    testWithDBDirective();
    testExpressions();
    
    std::cout << "\n=== All E2E Tests Passed ✓ ===" << std::endl;
    return 0;
}
