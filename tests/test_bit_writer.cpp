/**
 * @file test_bit_writer.cpp
 * @brief Tests for MSB-first bit writer
 */

#include "assembler/bit_writer.h"
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace z80;

void printBuffer(const std::vector<Byte>& buffer, const std::string& label) {
    std::cout << "  " << label << ": ";
    for (Byte b : buffer) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)b << " ";
    }
    std::cout << std::dec << std::endl;
}

void testSingleBits() {
    std::cout << "Test: Single bits MSB-first..." << std::endl;
    
    BitWriter writer;
    
    // Write bits: 1, 0, 0, 0, 0, 1, 0, 1
    // Expected byte: 10000101 = 0x85
    writer.writeBit(true);   // Bit 7
    writer.writeBit(false);  // Bit 6
    writer.writeBit(false);  // Bit 5
    writer.writeBit(false);  // Bit 4
    writer.writeBit(false);  // Bit 3
    writer.writeBit(true);   // Bit 2
    writer.writeBit(false);  // Bit 1
    writer.writeBit(true);   // Bit 0
    
    const auto& buffer = writer.getBuffer();
    assert(buffer.size() == 1);
    assert(buffer[0] == 0x85);
    
    printBuffer(buffer, "Single bits");
    std::cout << "  ✓ Expected 0x85, got 0x" << std::hex << (int)buffer[0] << std::dec << std::endl;
}

void testMultipleBits() {
    std::cout << "\nTest: writeBits() method..." << std::endl;
    
    BitWriter writer;
    
    // Write 3 bits: value=5 (binary 101)
    // Expected: 1, 0, 1
    writer.writeBits(0x5, 3);
    
    // Write 5 bits: value=10 (binary 01010)
    // Expected: 0, 1, 0, 1, 0
    writer.writeBits(0x0A, 5);
    
    // Total: 10101010 = 0xAA
    const auto& buffer = writer.getBuffer();
    assert(buffer.size() == 1);
    assert(buffer[0] == 0xAA);
    
    printBuffer(buffer, "Multiple bits");
    std::cout << "  ✓ writeBits(5,3) + writeBits(10,5) = 0xAA" << std::endl;
}

void testPartialByte() {
    std::cout << "\nTest: Partial byte with flush..." << std::endl;
    
    BitWriter writer;
    
    // Write 5 bits
    writer.writeBits(0x1F, 5);  // 11111
    
    // Before flush
    assert(writer.getBuffer().size() == 0);
    assert(!writer.isAligned());
    assert(writer.getBitPosition() == 2);
    
    // Flush - should pad with zeros: 11111000 = 0xF8
    writer.flush();
    
    const auto& buffer = writer.getBuffer();
    assert(buffer.size() == 1);
    assert(buffer[0] == 0xF8);
    assert(writer.isAligned());
    
    printBuffer(buffer, "Partial byte");
    std::cout << "  ✓ 5 bits (11111) flushed = 0xF8" << std::endl;
}

void testMultipleBytes() {
    std::cout << "\nTest: Multiple complete bytes..." << std::endl;
    
    BitWriter writer;
    
    // Write three bytes using writeByte()
    writer.writeByte(0x12);
    writer.writeByte(0x34);
    writer.writeByte(0x56);
    
    const auto& buffer = writer.getBuffer();
    assert(buffer.size() == 3);
    assert(buffer[0] == 0x12);
    assert(buffer[1] == 0x34);
    assert(buffer[2] == 0x56);
    
    printBuffer(buffer, "Multiple bytes");
    std::cout << "  ✓ Three bytes written correctly" << std::endl;
}

void testWordWriting() {
    std::cout << "\nTest: 16-bit word writing (MSB-first)..." << std::endl;
    
    BitWriter writer;
    
    // Write word 0x1234 - should write MSB (0x12) then LSB (0x34)
    writer.writeWord(0x1234);
    
    const auto& buffer = writer.getBuffer();
    assert(buffer.size() == 2);
    assert(buffer[0] == 0x12);  // MSB first
    assert(buffer[1] == 0x34);  // LSB second
    
    printBuffer(buffer, "Word (MSB-first)");
    std::cout << "  ✓ Word 0x1234 written as [0x12, 0x34]" << std::endl;
}

void testMixedOperations() {
    std::cout << "\nTest: Mixed bit/byte operations..." << std::endl;
    
    BitWriter writer;
    
    // Write 4 bits: 1010
    writer.writeBits(0x0A, 4);
    
    // Write 4 bits: 0101
    writer.writeBits(0x05, 4);
    // Now we have: 10100101 = 0xA5
    
    // Write a complete byte
    writer.writeByte(0x3C);
    
    // Write 4 more bits: 1111
    writer.writeBits(0x0F, 4);
    
    const auto& buffer = writer.getBuffer();
    assert(buffer.size() == 2);
    assert(buffer[0] == 0xA5);
    assert(buffer[1] == 0x3C);
    
    // Flush to get the final partial byte
    writer.flush();
    assert(buffer.size() == 3);
    assert(buffer[2] == 0xF0);  // 1111 0000
    
    printBuffer(buffer, "Mixed operations");
    std::cout << "  ✓ Mixed bit/byte operations work correctly" << std::endl;
}

void testRELExample() {
    std::cout << "\nTest: Real .REL example (bios.rel header)..." << std::endl;
    
    BitWriter writer;
    
    // Simulate writing the start of a .REL file:
    // Control bit: 1 (relocatable)
    writer.writeBit(true);
    
    // Type: 00 (special link item)
    writer.writeBits(0, 2);
    
    // Code: 0010 (program name)
    writer.writeBits(2, 4);
    
    // Length: 110 (6 characters)
    writer.writeBits(6, 3);
    // So far: 1 00 0010 110 = 10000101 10 = 0x85 followed by bits 1,0
    
    // Name "BIOSMO" (6 chars, 8 bits each)
    writer.writeByte('B');
    writer.writeByte('I');
    writer.writeByte('O');
    writer.writeByte('S');
    writer.writeByte('M');
    writer.writeByte('O');
    
    const auto& buffer = writer.getBuffer();
    
    // First byte should be 0x85 (verified from bios.rel)
    assert(buffer.size() >= 1);
    assert(buffer[0] == 0x85);
    
    // Second byte should contain the remaining 2 bits (10) + first 6 bits of 'B'
    // 'B' = 0x42 = 01000010
    // So: 10 + 010000 = 10010000 = 0x90
    assert(buffer[1] == 0x90);
    
    printBuffer(buffer, "REL header");
    std::cout << "  ✓ Matches bios.rel header bytes [0x85, 0x90, ...]" << std::endl;
}

void testBitPosition() {
    std::cout << "\nTest: Bit position tracking..." << std::endl;
    
    BitWriter writer;
    
    // Start at bit 7 (MSB)
    assert(writer.getBitPosition() == 7);
    assert(writer.isAligned());
    
    // Write 1 bit
    writer.writeBit(true);
    assert(writer.getBitPosition() == 6);
    assert(!writer.isAligned());
    
    // Write 6 more bits
    writer.writeBits(0, 6);
    assert(writer.getBitPosition() == 0);
    
    // Write 1 more bit - should complete the byte and reset to 7
    writer.writeBit(false);
    assert(writer.getBitPosition() == 7);
    assert(writer.isAligned());
    
    std::cout << "  ✓ Bit position tracking correct" << std::endl;
}

void testTotalBits() {
    std::cout << "\nTest: Total bits counting..." << std::endl;
    
    BitWriter writer;
    
    assert(writer.getTotalBits() == 0);
    
    // Write 3 bits
    writer.writeBits(0, 3);
    assert(writer.getTotalBits() == 3);
    
    // Write 5 more bits (completes first byte)
    writer.writeBits(0, 5);
    assert(writer.getTotalBits() == 8);
    
    // Write 2 more bits
    writer.writeBits(0, 2);
    assert(writer.getTotalBits() == 10);
    
    std::cout << "  ✓ Total bits counting correct" << std::endl;
}

int main() {
    std::cout << "=== BitWriter Tests (MSB-First) ===" << std::endl << std::endl;
    
    testSingleBits();
    testMultipleBits();
    testPartialByte();
    testMultipleBytes();
    testWordWriting();
    testMixedOperations();
    testRELExample();
    testBitPosition();
    testTotalBits();
    
    std::cout << "\n=== All BitWriter Tests Passed ✓ ===" << std::endl;
    return 0;
}
