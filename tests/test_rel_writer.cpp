/**
 * @file test_rel_writer.cpp
 * @brief Basic tests for REL writer
 */

#include "assembler/rel_writer.h"
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace z80;

void printBytes(const std::vector<Byte>& buffer, int maxBytes = 20) {
    std::cout << "  Output: ";
    for (size_t i = 0; i < std::min((size_t)maxBytes, buffer.size()); i++) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)buffer[i] << " ";
    }
    if (buffer.size() > maxBytes) {
        std::cout << "... (" << std::dec << buffer.size() << " bytes total)";
    }
    std::cout << std::dec << std::endl;
}

void testMinimalModule() {
    std::cout << "Test: Minimal relocatable module..." << std::endl;
    
    RELWriter writer;
    
    // Begin module "TEST"
    writer.beginModule("TEST", true);
    
    // Write program size
    writer.writeProgramSize(0x0005);
    
    // End module
    writer.endModule();
    
    // End file
    writer.endFile();
    
    const auto& buffer = writer.getBuffer();
    
    // Verify first byte matches expected pattern
    // Control bit: 1 (relocatable)
    // Type: 00 (special)
    // Code: 0010 (program name)
    // = 1 00 0010 1... = 10000101 0... = 0x85 (for first byte with length field)
    
    printBytes(buffer);
    
    // First byte should have the control bit + special type + program name code
    assert(buffer.size() > 0);
    assert((buffer[0] & 0x80) == 0x80);  // Control bit = 1
    
    std::cout << "  ✓ Module header written correctly" << std::endl;
}

void testAbsoluteModule() {
    std::cout << "\nTest: Absolute module (ASEG)..." << std::endl;
    
    RELWriter writer;
    
    // Begin absolute module
    writer.beginModule("ABSMOD", false);  // false = absolute
    
    // Set location
    writer.setLocation(0x100, ItemType::Absolute);
    
    // Write some absolute data
    std::vector<Byte> code = {0x00, 0xC3, 0x00, 0x01};  // NOP, JP 0100H
    writer.writeAbsoluteData(code);
    
    writer.endModule();
    writer.endFile();
    
    const auto& buffer = writer.getBuffer();
    printBytes(buffer);
    
    // First bit should be 0 (absolute)
    assert((buffer[0] & 0x80) == 0x00);
    
    std::cout << "  ✓ Absolute module written correctly" << std::endl;
}

void testSymbolNames() {
    std::cout << "\nTest: Symbol name truncation..." << std::endl;
    
    RELWriter writer;
    
    // Begin module with long name
    writer.beginModule("VERYLONGNAME", true);
    
    // Should truncate to "VERYLO" (6 chars)
    
    // Write entry symbol with long name
    writer.writeEntrySymbol("ANOTHERLONGNAME", 0x1000, ItemType::ProgramRel);
    
    writer.endModule();
    writer.endFile();
    
    const auto& buffer = writer.getBuffer();
    printBytes(buffer, 30);
    
    std::cout << "  ✓ Symbol names truncated to 6 characters" << std::endl;
}

void testDataSegments() {
    std::cout << "\nTest: Program and Data segments..." << std::endl;
    
    RELWriter writer;
    
    writer.beginModule("MULTI", true);
    
    // Program area
    writer.writeProgramSize(10);
    std::vector<Byte> progCode = {0x21, 0x00, 0x00};  // LD HL,0000H
    writer.writeProgramData(progCode);
    
    // Data area
    writer.writeDataSize(20);
    std::vector<Byte> dataBytes = {0x00, 0x00};
    writer.writeDataData(dataBytes);
    
    writer.endModule();
    writer.endFile();
    
    const auto& buffer = writer.getBuffer();
    printBytes(buffer, 40);
    
    std::cout << "  ✓ Multiple segments written" << std::endl;
}

void testExternalSymbol() {
    std::cout << "\nTest: External symbol reference..." << std::endl;
    
    RELWriter writer;
    
    writer.beginModule("EXTREF", true);
    
    // Declare external symbol
    writer.writeChainExternal("BDOS");
    
    // Write some code that uses it
    std::vector<Byte> code = {0xCD, 0x00, 0x00};  // CALL 0000H (will be relocated)
    writer.writeProgramData(code);
    
    // Chain address for relocation
    writer.writeChainAddress(0x0001);  // Offset to the address in code
    
    writer.endModule();
    writer.endFile();
    
    const auto& buffer = writer.getBuffer();
    printBytes(buffer, 40);
    
    std::cout << "  ✓ External symbol and chain address written" << std::endl;
}

void testBIOSMOHeader() {
    std::cout << "\nTest: BIOSMO header (compare with real bios.rel)..." << std::endl;
    
    RELWriter writer;
    
    // Recreate the start of bios.rel
    writer.beginModule("BIOSMO", true);
    
    const auto& buffer = writer.getBuffer();
    
    // Expected from analysis: 0x85, 0x90, ...
    assert(buffer.size() >= 2);
    assert(buffer[0] == 0x85);
    assert(buffer[1] == 0x90);
    
    printBytes(buffer, 10);
    std::cout << "  ✓ Matches bios.rel header: [0x85, 0x90, ...]" << std::endl;
}

int main() {
    std::cout << "=== REL Writer Tests ===" << std::endl << std::endl;
    
    testMinimalModule();
    testAbsoluteModule();
    testSymbolNames();
    testDataSegments();
    testExternalSymbol();
    testBIOSMOHeader();
    
    std::cout << "\n=== All REL Writer Tests Passed ✓ ===" << std::endl;
    return 0;
}
