/**
 * @file test_instructions.cpp
 * @brief Unit tests for Z80 instruction table
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include "../src/assembler/z80_instructions.h"
#include <iostream>
#include <cassert>

using namespace z80;

void testBasicInstructions() {
    Z80Instructions instructions;
    
    // Test LD A,B
    auto ldAB = instructions.findInstruction("LD", "A,B");
    assert(ldAB != nullptr);
    assert(ldAB->opcodes.size() == 1);
    assert(ldAB->opcodes[0] == 0x78);
    std::cout << "LD A,B: 0x" << std::hex << (int)ldAB->opcodes[0] << std::dec << " ✓\n";
    
    // Test LD A,n
    auto ldAn = instructions.findInstruction("LD", "A,N");
    assert(ldAn != nullptr);
    assert(ldAn->opcodes[0] == 0x3E);
    assert(ldAn->operandBytes == 1);
    std::cout << "LD A,N: 0x" << std::hex << (int)ldAn->opcodes[0] << std::dec << " ✓\n";
    
    // Test LD HL,nn
    auto ldHLnn = instructions.findInstruction("LD", "HL,NN");
    assert(ldHLnn != nullptr);
    assert(ldHLnn->opcodes[0] == 0x21);
    assert(ldHLnn->operandBytes == 2);
    std::cout << "LD HL,NN: 0x" << std::hex << (int)ldHLnn->opcodes[0] << std::dec << " ✓\n";
    
    std::cout << "testBasicInstructions passed\n";
}

void testArithmeticInstructions() {
    Z80Instructions instructions;
    
    // Test ADD A,B
    auto addAB = instructions.findInstruction("ADD", "A,B");
    assert(addAB != nullptr);
    assert(addAB->opcodes[0] == 0x80);
    std::cout << "ADD A,B: 0x" << std::hex << (int)addAB->opcodes[0] << std::dec << " ✓\n";
    
    // Test SUB C
    auto subC = instructions.findInstruction("SUB", "A,C");
    assert(subC != nullptr);
    assert(subC->opcodes[0] == 0x91);
    std::cout << "SUB A,C: 0x" << std::hex << (int)subC->opcodes[0] << std::dec << " ✓\n";
    
    // Test INC A
    auto incA = instructions.findInstruction("INC", "A");
    assert(incA != nullptr);
    assert(incA->opcodes[0] == 0x3C);
    std::cout << "INC A: 0x" << std::hex << (int)incA->opcodes[0] << std::dec << " ✓\n";
    
    // Test DEC HL
    auto decHL = instructions.findInstruction("DEC", "HL");
    assert(decHL != nullptr);
    assert(decHL->opcodes[0] == 0x2B);
    std::cout << "DEC HL: 0x" << std::hex << (int)decHL->opcodes[0] << std::dec << " ✓\n";
    
    std::cout << "testArithmeticInstructions passed\n";
}

void testJumpInstructions() {
    Z80Instructions instructions;
    
    // Test JP nn
    auto jpnn = instructions.findInstruction("JP", "NN");
    assert(jpnn != nullptr);
    assert(jpnn->opcodes[0] == 0xC3);
    assert(jpnn->operandBytes == 2);
    std::cout << "JP NN: 0x" << std::hex << (int)jpnn->opcodes[0] << std::dec << " ✓\n";
    
    // Test JR e
    auto jre = instructions.findInstruction("JR", "E");
    assert(jre != nullptr);
    assert(jre->opcodes[0] == 0x18);
    assert(jre->operandBytes == 1);
    std::cout << "JR E: 0x" << std::hex << (int)jre->opcodes[0] << std::dec << " ✓\n";
    
    // Test CALL nn
    auto callnn = instructions.findInstruction("CALL", "NN");
    assert(callnn != nullptr);
    assert(callnn->opcodes[0] == 0xCD);
    std::cout << "CALL NN: 0x" << std::hex << (int)callnn->opcodes[0] << std::dec << " ✓\n";
    
    // Test RET
    auto ret = instructions.findInstruction("RET", "");
    assert(ret != nullptr);
    assert(ret->opcodes[0] == 0xC9);
    std::cout << "RET: 0x" << std::hex << (int)ret->opcodes[0] << std::dec << " ✓\n";
    
    std::cout << "testJumpInstructions passed\n";
}

void testStackInstructions() {
    Z80Instructions instructions;
    
    // Test PUSH BC
    auto pushBC = instructions.findInstruction("PUSH", "BC");
    assert(pushBC != nullptr);
    assert(pushBC->opcodes[0] == 0xC5);
    std::cout << "PUSH BC: 0x" << std::hex << (int)pushBC->opcodes[0] << std::dec << " ✓\n";
    
    // Test POP AF
    auto popAF = instructions.findInstruction("POP", "AF");
    assert(popAF != nullptr);
    assert(popAF->opcodes[0] == 0xF1);
    std::cout << "POP AF: 0x" << std::hex << (int)popAF->opcodes[0] << std::dec << " ✓\n";
    
    std::cout << "testStackInstructions passed\n";
}

void testMiscInstructions() {
    Z80Instructions instructions;
    
    // Test NOP
    auto nop = instructions.findInstruction("NOP", "");
    assert(nop != nullptr);
    assert(nop->opcodes[0] == 0x00);
    std::cout << "NOP: 0x" << std::hex << (int)nop->opcodes[0] << std::dec << " ✓\n";
    
    // Test HALT
    auto halt = instructions.findInstruction("HALT", "");
    assert(halt != nullptr);
    assert(halt->opcodes[0] == 0x76);
    std::cout << "HALT: 0x" << std::hex << (int)halt->opcodes[0] << std::dec << " ✓\n";
    
    // Test DI
    auto di = instructions.findInstruction("DI", "");
    assert(di != nullptr);
    assert(di->opcodes[0] == 0xF3);
    std::cout << "DI: 0x" << std::hex << (int)di->opcodes[0] << std::dec << " ✓\n";
    
    // Test RST 0
    auto rst0 = instructions.findInstruction("RST", "0");
    assert(rst0 != nullptr);
    assert(rst0->opcodes[0] == 0xC7);
    std::cout << "RST 0: 0x" << std::hex << (int)rst0->opcodes[0] << std::dec << " ✓\n";
    
    // Test RST 38h
    auto rst38 = instructions.findInstruction("RST", "56");
    assert(rst38 != nullptr);
    assert(rst38->opcodes[0] == 0xFF);
    std::cout << "RST 56: 0x" << std::hex << (int)rst38->opcodes[0] << std::dec << " ✓\n";
    
    std::cout << "testMiscInstructions passed\n";
}

void testRegisterCheck() {
    Z80Instructions instructions;
    
    assert(instructions.isRegister("A"));
    assert(instructions.isRegister("B"));
    assert(instructions.isRegister("HL"));
    assert(instructions.isRegister("SP"));
    assert(instructions.isRegister("IX"));
    assert(!instructions.isRegister("XYZ"));
    
    std::cout << "testRegisterCheck passed\n";
}

void testMnemonicCheck() {
    Z80Instructions instructions;
    
    assert(instructions.isMnemonic("LD"));
    assert(instructions.isMnemonic("ADD"));
    assert(instructions.isMnemonic("JP"));
    assert(instructions.isMnemonic("CALL"));
    assert(!instructions.isMnemonic("INVALID"));
    
    std::cout << "testMnemonicCheck passed\n";
}

int main() {
    std::cout << "Running Z80 Instruction Table tests...\n\n";
    
    testBasicInstructions();
    testArithmeticInstructions();
    testJumpInstructions();
    testStackInstructions();
    testMiscInstructions();
    testRegisterCheck();
    testMnemonicCheck();
    
    std::cout << "\n✓ All instruction tests passed!\n";
    return 0;
}
