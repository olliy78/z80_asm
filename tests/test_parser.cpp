/**
 * @file test_parser.cpp
 * @brief Unit tests for the parser
 */

#include "../src/assembler/parser.h"
#include <iostream>
#include <cassert>
#include <fstream>

using namespace z80;

void testBasicParsing() {
    std::cout << "Test: Basic Parsing..." << std::endl;
    
    // Create a simple test file
    std::ofstream testFile("/tmp/test_parser.asm");
    testFile << "        ORG 0\n";
    testFile << "START:  NOP\n";
    testFile << "        JP START\n";
    testFile << "        END\n";
    testFile.close();
    
    Parser parser;
    bool success = parser.assemble("/tmp/test_parser.asm");
    
    if (!success) {
        std::cout << "  Errors during assembly:" << std::endl;
        for (const auto& error : parser.getErrors()) {
            std::cout << "    Line " << error.line << ": " << error.message << std::endl;
        }
    } else {
        std::cout << "  ✓ Basic parsing succeeded" << std::endl;
    }
    
    const auto& lines = parser.getLines();
    std::cout << "  Parsed " << lines.size() << " lines" << std::endl;
    
    // Check symbol table
    const auto& symbolTable = parser.getSymbolTable();
    if (symbolTable.hasSymbol("START")) {
        const Symbol* sym = symbolTable.getSymbol("START");
        std::cout << "  ✓ Symbol 'START' found at address 0x" << std::hex << sym->value << std::dec << std::endl;
        assert(sym->value == 0);
    } else {
        std::cout << "  ✗ Symbol 'START' not found" << std::endl;
    }
}

void testDirectives() {
    std::cout << "\nTest: Directives (EQU, DB, DW, DS)..." << std::endl;
    
    std::ofstream testFile("/tmp/test_directives.asm");
    testFile << "VALUE   EQU 42\n";
    testFile << "        ORG 100H\n";
    testFile << "DATA1:  DB 1,2,3\n";
    testFile << "DATA2:  DW 0x1234\n";
    testFile << "SPACE:  DS 10\n";
    testFile << "        END\n";
    testFile.close();
    
    Parser parser;
    bool success = parser.assemble("/tmp/test_directives.asm");
    
    if (!success) {
        std::cout << "  Errors during assembly:" << std::endl;
        for (const auto& error : parser.getErrors()) {
            std::cout << "    Line " << error.line << ": " << error.message << std::endl;
        }
    } else {
        std::cout << "  ✓ Directive parsing succeeded" << std::endl;
    }
    
    // Check EQU symbol
    const auto& symbolTable = parser.getSymbolTable();
    if (symbolTable.hasSymbol("VALUE")) {
        const Symbol* sym = symbolTable.getSymbol("VALUE");
        std::cout << "  ✓ EQU symbol 'VALUE' = " << sym->value << std::endl;
        assert(sym->value == 42);
    }
    
    // Check generated code
    const auto& lines = parser.getLines();
    for (const auto& line : lines) {
        if (line.label == "DATA1") {
            std::cout << "  ✓ DATA1 at address 0x" << std::hex << line.address << std::dec;
            std::cout << ", " << line.code.size() << " bytes" << std::endl;
            assert(line.code.size() == 3);
            assert(line.code[0] == 1 && line.code[1] == 2 && line.code[2] == 3);
        } else if (line.label == "DATA2") {
            std::cout << "  ✓ DATA2 at address 0x" << std::hex << line.address << std::dec;
            std::cout << ", " << line.code.size() << " bytes" << std::endl;
            assert(line.code.size() == 2);
            assert(line.code[0] == 0x34 && line.code[1] == 0x12); // Little-endian
        } else if (line.label == "SPACE") {
            std::cout << "  ✓ SPACE at address 0x" << std::hex << line.address << std::dec;
            std::cout << ", " << line.code.size() << " bytes reserved" << std::endl;
            assert(line.code.size() == 10);
        }
    }
}

void testSegments() {
    std::cout << "\nTest: Segments (CSEG, DSEG, ASEG)..." << std::endl;
    
    std::ofstream testFile("/tmp/test_segments.asm");
    testFile << "        CSEG\n";
    testFile << "        ORG 1000H\n";
    testFile << "CODE1:  NOP\n";
    testFile << "        DSEG\n";
    testFile << "        ORG 2000H\n";
    testFile << "DATA1:  DS 1\n";
    testFile << "        ASEG\n";
    testFile << "        ORG 3000H\n";
    testFile << "ABS1:   NOP\n";
    testFile << "        END\n";
    testFile.close();
    
    Parser parser;
    bool success = parser.assemble("/tmp/test_segments.asm");
    
    if (!success) {
        std::cout << "  Errors during assembly:" << std::endl;
        for (const auto& error : parser.getErrors()) {
            std::cout << "    Line " << error.line << ": " << error.message << std::endl;
        }
    } else {
        std::cout << "  ✓ Segment parsing succeeded" << std::endl;
    }
    
    // Check symbols in different segments
    const auto& symbolTable = parser.getSymbolTable();
    
    if (symbolTable.hasSymbol("CODE1")) {
        const Symbol* sym = symbolTable.getSymbol("CODE1");
        std::cout << "  ✓ CODE1 in " << (int)sym->segment << " segment at 0x" << std::hex << sym->value << std::dec << std::endl;
    }
    
    if (symbolTable.hasSymbol("DATA1")) {
        const Symbol* sym = symbolTable.getSymbol("DATA1");
        std::cout << "  ✓ DATA1 in " << (int)sym->segment << " segment at 0x" << std::hex << sym->value << std::dec << std::endl;
    }
    
    if (symbolTable.hasSymbol("ABS1")) {
        const Symbol* sym = symbolTable.getSymbol("ABS1");
        std::cout << "  ✓ ABS1 in " << (int)sym->segment << " segment at 0x" << std::hex << sym->value << std::dec << std::endl;
    }
}

int main() {
    std::cout << "=== Parser Tests ===" << std::endl;
    
    testBasicParsing();
    testDirectives();
    testSegments();
    
    std::cout << "\n=== All Parser Tests Passed ===" << std::endl;
    return 0;
}
