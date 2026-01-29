/**
 * @file test_parser_advanced.cpp
 * @brief Advanced parser tests for complex operand patterns
 */

#include "assembler/parser.h"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace z80;

void testComplexOperands() {
    std::cout << "\nTest: Complex Operands..." << std::endl;
    
    std::ofstream testFile("/tmp/test_complex_operands.asm");
    testFile << "        ORG 8000H\n";
    testFile << "START:  LD A,B          ; Register to register\n";
    testFile << "        LD A,42H        ; Immediate 8-bit\n";
    testFile << "        LD HL,1234H     ; Immediate 16-bit\n";
    testFile << "        LD A,(HL)       ; Indirect HL\n";
    testFile << "        LD (HL),A       ; Indirect HL\n";
    testFile << "        LD A,(IX+5)     ; Indexed IX\n";
    testFile << "        LD (IY-3),B     ; Indexed IY\n";
    testFile << "        LD HL,(9000H)   ; Extended indirect\n";
    testFile << "        END\n";
    testFile.close();
    
    Parser parser;
    bool success = parser.assemble("/tmp/test_complex_operands.asm");
    
    if (!success) {
        std::cout << "  Errors during assembly:" << std::endl;
        for (const auto& error : parser.getErrors()) {
            std::cout << "    Line " << error.line << ": " << error.message << std::endl;
        }
    } else {
        std::cout << "  ✓ Complex operand parsing succeeded" << std::endl;
    }
    
    // Check that instructions were parsed
    const auto& lines = parser.getLines();
    std::cout << "  ✓ Parsed " << lines.size() << " lines" << std::endl;
    
    // Verify some operand parsing
    for (const auto& line : lines) {
        if (!line.mnemonic.empty() && line.mnemonic != "ORG" && line.mnemonic != "END") {
            std::cout << "    " << line.mnemonic;
            if (!line.operands.empty()) {
                std::cout << " [";
                for (size_t i = 0; i < line.operands.size(); i++) {
                    if (i > 0) std::cout << ", ";
                    std::cout << line.operands[i];
                }
                std::cout << "]";
            }
            std::cout << " -> " << line.code.size() << " bytes" << std::endl;
        }
    }
}

void testMultipleOperands() {
    std::cout << "\nTest: Multiple Operands..." << std::endl;
    
    std::ofstream testFile("/tmp/test_multi_operands.asm");
    testFile << "        LD A,B\n";
    testFile << "        LD BC,1234H\n";
    testFile << "        ADD A,5\n";
    testFile << "        LD (IX+10),42H\n";
    testFile << "        END\n";
    testFile.close();
    
    Parser parser;
    bool success = parser.assemble("/tmp/test_multi_operands.asm");
    
    if (success) {
        std::cout << "  ✓ Multi-operand parsing succeeded" << std::endl;
        
        const auto& lines = parser.getLines();
        for (const auto& line : lines) {
            if (!line.operands.empty()) {
                std::cout << "    " << line.mnemonic << ": " 
                          << line.operands.size() << " operand(s)";
                if (line.operands.size() > 0) {
                    std::cout << " [" << line.operands[0];
                    if (line.operands.size() > 1) {
                        std::cout << ", " << line.operands[1];
                    }
                    std::cout << "]";
                }
                std::cout << std::endl;
            }
        }
    } else {
        std::cout << "  ✗ Failed:" << std::endl;
        for (const auto& error : parser.getErrors()) {
            std::cout << "    Line " << error.line << ": " << error.message << std::endl;
        }
    }
}

void testRegisterPatterns() {
    std::cout << "\nTest: Register Pattern Recognition..." << std::endl;
    
    std::ofstream testFile("/tmp/test_registers.asm");
    testFile << "        LD A,B\n";
    testFile << "        LD BC,DE\n";
    testFile << "        ADD HL,BC\n";
    testFile << "        INC A\n";
    testFile << "        DEC HL\n";
    testFile << "        PUSH BC\n";
    testFile << "        POP DE\n";
    testFile << "        END\n";
    testFile.close();
    
    Parser parser;
    bool success = parser.assemble("/tmp/test_registers.asm");
    
    if (success) {
        std::cout << "  ✓ Register pattern recognition succeeded" << std::endl;
    } else {
        std::cout << "  ✗ Some register patterns failed:" << std::endl;
        for (const auto& error : parser.getErrors()) {
            std::cout << "    Line " << error.line << ": " << error.message << std::endl;
        }
    }
}

int main() {
    std::cout << "=== Advanced Parser Tests ===" << std::endl;
    
    testComplexOperands();
    testMultipleOperands();
    testRegisterPatterns();
    
    std::cout << "\n=== Advanced Tests Complete ===" << std::endl;
    return 0;
}
