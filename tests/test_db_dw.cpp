/**
 * @file test_db_dw.cpp
 * @brief Tests for DB and DW directive code generation
 */

#include "assembler/parser.h"
#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace z80;

// Helper function to assemble from memory
bool assembleFromSource(Parser& parser, const std::vector<std::string>& source, const std::string& tempFile) {
    // Write source to temporary file
    std::ofstream out(tempFile);
    if (!out) return false;
    
    for (const auto& line : source) {
        out << line << "\n";
    }
    out.close();
    
    // Assemble the file
    bool result = parser.assemble(tempFile);
    
    // Clean up
    std::remove(tempFile.c_str());
    
    return result;
}

void testSimpleDB() {
    std::cout << "Test: Simple DB directive..." << std::endl;
    
    Parser parser;
    std::vector<std::string> source = {
        "        ORG 100H",
        "DATA1:  DB 42",
        "DATA2:  DB 1,2,3",
        "DATA3:  DB 0FFH"
    };
    
    bool success = assembleFromSource(parser, source, "test_db.tmp");
    assert(success && "DB parsing failed");
    
    const auto& lines = parser.getLines();
    
    // Find DATA1 (42)
    auto data1 = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "DATA1"; });
    assert(data1 != lines.end());
    assert(data1->code.size() == 1);
    assert(data1->code[0] == 42);
    std::cout << "  ✓ DATA1: DB 42 -> 0x" << std::hex << (int)data1->code[0] << std::endl;
    
    // Find DATA2 (1,2,3)
    auto data2 = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "DATA2"; });
    assert(data2 != lines.end());
    assert(data2->code.size() == 3);
    assert(data2->code[0] == 1);
    assert(data2->code[1] == 2);
    assert(data2->code[2] == 3);
    std::cout << "  ✓ DATA2: DB 1,2,3 -> [" << std::dec 
              << (int)data2->code[0] << ", " 
              << (int)data2->code[1] << ", " 
              << (int)data2->code[2] << "]" << std::endl;
    
    // Find DATA3 (0FFH)
    auto data3 = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "DATA3"; });
    assert(data3 != lines.end());
    assert(data3->code.size() == 1);
    assert(data3->code[0] == 0xFF);
    std::cout << "  ✓ DATA3: DB 0FFH -> 0x" << std::hex << (int)data3->code[0] << std::endl;
}

void testSimpleDW() {
    std::cout << "\nTest: Simple DW directive..." << std::endl;
    
    Parser parser;
    std::vector<std::string> source = {
        "        ORG 100H",
        "WORD1:  DW 1234H",
        "WORD2:  DW 0ABCDH,5678H"
    };
    
    bool success = assembleFromSource(parser, source, "test_dw.tmp");
    assert(success && "DW parsing failed");
    
    const auto& lines = parser.getLines();
    
    // Find WORD1 (1234H) - little-endian: 34H, 12H
    auto word1 = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "WORD1"; });
    assert(word1 != lines.end());
    assert(word1->code.size() == 2);
    assert(word1->code[0] == 0x34);  // LSB
    assert(word1->code[1] == 0x12);  // MSB
    std::cout << "  ✓ WORD1: DW 1234H -> [0x" << std::hex 
              << (int)word1->code[0] << ", 0x" 
              << (int)word1->code[1] << "] (little-endian)" << std::endl;
    
    // Find WORD2 (ABCDH, 5678H)
    auto word2 = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "WORD2"; });
    assert(word2 != lines.end());
    assert(word2->code.size() == 4);
    assert(word2->code[0] == 0xCD);  // LSB of ABCDH
    assert(word2->code[1] == 0xAB);  // MSB of ABCDH
    assert(word2->code[2] == 0x78);  // LSB of 5678H
    assert(word2->code[3] == 0x56);  // MSB of 5678H
    std::cout << "  ✓ WORD2: DW 0ABCDH,5678H -> [0x" << std::hex 
              << (int)word2->code[0] << ", 0x" 
              << (int)word2->code[1] << ", 0x"
              << (int)word2->code[2] << ", 0x"
              << (int)word2->code[3] << "]" << std::endl;
}

void testExpressions() {
    std::cout << "\nTest: DB/DW with expressions..." << std::endl;
    
    Parser parser;
    std::vector<std::string> source = {
        "VALUE   EQU 10",
        "        ORG 100H",
        "DATA:   DB VALUE+5",
        "DATA2:  DB VALUE*2",
        "ADDR:   DW 1000H+100H"
    };
    
    bool success = assembleFromSource(parser, source, "test_expr.tmp");
    assert(success && "Expression parsing failed");
    
    const auto& lines = parser.getLines();
    
    // DATA: DB VALUE+5 = 10+5 = 15
    auto data = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "DATA"; });
    assert(data != lines.end());
    assert(data->code.size() == 1);
    assert(data->code[0] == 15);
    std::cout << "  ✓ DATA: DB VALUE+5 -> " << std::dec << (int)data->code[0] << std::endl;
    
    // DATA2: DB VALUE*2 = 10*2 = 20
    auto data2 = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "DATA2"; });
    assert(data2 != lines.end());
    assert(data2->code.size() == 1);
    assert(data2->code[0] == 20);
    std::cout << "  ✓ DATA2: DB VALUE*2 -> " << std::dec << (int)data2->code[0] << std::endl;
    
    // ADDR: DW 1000H+100H = 1100H
    auto addr = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "ADDR"; });
    assert(addr != lines.end());
    assert(addr->code.size() == 2);
    assert(addr->code[0] == 0x00);  // LSB of 1100H
    assert(addr->code[1] == 0x11);  // MSB of 1100H
    std::cout << "  ✓ ADDR: DW 1000H+100H -> [0x" << std::hex 
              << (int)addr->code[0] << ", 0x" 
              << (int)addr->code[1] << "] = 0x1100" << std::endl;
}

void testLocationCounter() {
    std::cout << "\nTest: DB/DW with location counter ($)..." << std::endl;
    
    Parser parser;
    std::vector<std::string> source = {
        "        ORG 100H",
        "START:  DB $",         // Should be 100H (LSB = 0)
        "NEXT:   DW $"          // Should be 101H
    };
    
    bool success = assembleFromSource(parser, source, "test_loc.tmp");
    assert(success && "Location counter parsing failed");
    
    const auto& lines = parser.getLines();
    
    // START: DB $ at 100H
    auto start = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "START"; });
    assert(start != lines.end());
    assert(start->code.size() == 1);
    assert(start->code[0] == 0x00);  // LSB of 100H
    std::cout << "  ✓ START: DB $ at 0x100 -> 0x" << std::hex << (int)start->code[0] << std::endl;
    
    // NEXT: DW $ at 101H
    auto next = std::find_if(lines.begin(), lines.end(), 
        [](const ParsedLine& l) { return l.label == "NEXT"; });
    assert(next != lines.end());
    assert(next->code.size() == 2);
    assert(next->code[0] == 0x01);  // LSB of 101H
    assert(next->code[1] == 0x01);  // MSB of 101H
    std::cout << "  ✓ NEXT: DW $ at 0x101 -> [0x" << std::hex 
              << (int)next->code[0] << ", 0x" 
              << (int)next->code[1] << "]" << std::endl;
}

int main() {
    std::cout << "=== DB/DW Code Generation Tests ===" << std::endl << std::endl;
    
    testSimpleDB();
    testSimpleDW();
    testExpressions();
    testLocationCounter();
    
    std::cout << "\n=== All DB/DW Tests Passed ✓ ===" << std::endl;
    return 0;
}
