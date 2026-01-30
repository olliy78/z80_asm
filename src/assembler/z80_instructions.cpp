/**
 * @file z80_instructions.cpp
 * @brief Implementation of Z80 instruction set
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include "z80_instructions.h"
#include "common/utils.h"
#include <algorithm>

namespace z80 {

Z80Instructions::Z80Instructions() {
    initializeInstructions();
}

void Z80Instructions::initializeInstructions() {
    // Clear any existing instructions
    instructions_.clear();
    
    // Build the complete instruction table
    addLoad8BitInstructions();
    addLoad16BitInstructions();
    addArithmeticInstructions();
    addLogicalInstructions();
    addRotateShiftInstructions();
    addBitInstructions();
    addJumpCallReturnInstructions();
    addIOInstructions();
    addMiscInstructions();
}

const InstructionInfo* Z80Instructions::findInstruction(const std::string& mnemonic,
                                                        const std::string& operands) const {
    std::string upperMnemonic = toUpper(mnemonic);
    std::string upperOperands = toUpper(operands);
    
    for (const auto& inst : instructions_) {
        if (inst.mnemonic == upperMnemonic && inst.operandPattern == upperOperands) {
            return &inst;
        }
    }
    
    return nullptr;
}

bool Z80Instructions::isRegister(const std::string& name) const {
    std::string upper = toUpper(name);
    return (upper == "A" || upper == "B" || upper == "C" || upper == "D" ||
            upper == "E" || upper == "H" || upper == "L" ||
            upper == "AF" || upper == "BC" || upper == "DE" || upper == "HL" ||
            upper == "SP" || upper == "IX" || upper == "IY" ||
            upper == "IXH" || upper == "IXL" || upper == "IYH" || upper == "IYL" ||
            upper == "I" || upper == "R");
}

bool Z80Instructions::isMnemonic(const std::string& name) const {
    std::string upper = toUpper(name);
    
    // Check if any instruction starts with this mnemonic
    for (const auto& inst : instructions_) {
        if (inst.mnemonic == upper) {
            return true;
        }
    }
    
    return false;
}

void Z80Instructions::addLoad8BitInstructions() {
    // LD r,r' (r = A,B,C,D,E,H,L; r' = A,B,C,D,E,H,L)
    const char* regs = "BCDEHL_A";  // Z80 register encoding order (bit position 6 is (HL))
    Byte baseOpcode = 0x40;
    
    for (int dst = 0; dst < 8; dst++) {
        for (int src = 0; src < 8; src++) {
            if (dst == 6 && src == 6) continue; // Skip LD (HL),(HL) - that's HALT
            if (dst == 6 || src == 6) continue; // Skip (HL) for now - handle separately
            
            InstructionInfo info;
            info.mnemonic = "LD";
            info.operandPattern = std::string(1, regs[dst]) + "," + std::string(1, regs[src]);
            info.mode = AddressingMode::Register;
            info.opcodes = {static_cast<Byte>(baseOpcode + (dst << 3) + src)};
            info.operandBytes = 0;
            info.cycles = 4;
            instructions_.push_back(info);
        }
    }
    
    // LD r,n (r = B,C,D,E,H,L,A)
    baseOpcode = 0x06;
    for (int reg = 0; reg < 8; reg++) {
        if (reg == 6) continue; // Skip (HL) position
        InstructionInfo info;
        info.mnemonic = "LD";
        info.operandPattern = std::string(1, regs[reg]) + ",N";
        info.mode = AddressingMode::Immediate;
        info.opcodes = {static_cast<Byte>(baseOpcode + (reg << 3))};
        info.operandBytes = 1;
        info.cycles = 7;
        instructions_.push_back(info);
    }
    
    // LD r,(HL) and LD (HL),r
    baseOpcode = 0x46;
    for (int reg = 0; reg < 8; reg++) {
        if (reg == 6) continue; // Skip (HL),(HL)
        
        // LD r,(HL)
        InstructionInfo info1;
        info1.mnemonic = "LD";
        info1.operandPattern = std::string(1, regs[reg]) + ",(HL)";
        info1.mode = AddressingMode::RegisterIndirect;
        info1.opcodes = {static_cast<Byte>(baseOpcode + (reg << 3))};
        info1.operandBytes = 0;
        info1.cycles = 7;
        instructions_.push_back(info1);
        
        // LD (HL),r
        InstructionInfo info2;
        info2.mnemonic = "LD";
        info2.operandPattern = "(HL)," + std::string(1, regs[reg]);
        info2.mode = AddressingMode::RegisterIndirect;
        info2.opcodes = {static_cast<Byte>(0x70 + reg)};
        info2.operandBytes = 0;
        info2.cycles = 7;
        instructions_.push_back(info2);
    }
    
    // LD (HL),n
    InstructionInfo ldHLn;
    ldHLn.mnemonic = "LD";
    ldHLn.operandPattern = "(HL),N";
    ldHLn.mode = AddressingMode::Immediate;
    ldHLn.opcodes = {0x36};
    ldHLn.operandBytes = 1;
    ldHLn.cycles = 10;
    instructions_.push_back(ldHLn);
    
    // LD A,(BC) and LD A,(DE)
    InstructionInfo ldABC;
    ldABC.mnemonic = "LD";
    ldABC.operandPattern = "A,(BC)";
    ldABC.mode = AddressingMode::RegisterIndirect;
    ldABC.opcodes = {0x0A};
    ldABC.operandBytes = 0;
    ldABC.cycles = 7;
    instructions_.push_back(ldABC);
    
    InstructionInfo ldADE;
    ldADE.mnemonic = "LD";
    ldADE.operandPattern = "A,(DE)";
    ldADE.mode = AddressingMode::RegisterIndirect;
    ldADE.opcodes = {0x1A};
    ldADE.operandBytes = 0;
    ldADE.cycles = 7;
    instructions_.push_back(ldADE);
    
    // LD (BC),A and LD (DE),A
    InstructionInfo ldBCA;
    ldBCA.mnemonic = "LD";
    ldBCA.operandPattern = "(BC),A";
    ldBCA.mode = AddressingMode::RegisterIndirect;
    ldBCA.opcodes = {0x02};
    ldBCA.operandBytes = 0;
    ldBCA.cycles = 7;
    instructions_.push_back(ldBCA);
    
    InstructionInfo ldDEA;
    ldDEA.mnemonic = "LD";
    ldDEA.operandPattern = "(DE),A";
    ldDEA.mode = AddressingMode::RegisterIndirect;
    ldDEA.opcodes = {0x12};
    ldDEA.operandBytes = 0;
    ldDEA.cycles = 7;
    instructions_.push_back(ldDEA);
    
    // LD A,(nn) and LD (nn),A
    InstructionInfo ldAnn;
    ldAnn.mnemonic = "LD";
    ldAnn.operandPattern = "A,(NN)";
    ldAnn.mode = AddressingMode::Direct;
    ldAnn.opcodes = {0x3A};
    ldAnn.operandBytes = 2;
    ldAnn.cycles = 13;
    instructions_.push_back(ldAnn);
    
    InstructionInfo ldnnA;
    ldnnA.mnemonic = "LD";
    ldnnA.operandPattern = "(NN),A";
    ldnnA.mode = AddressingMode::Direct;
    ldnnA.opcodes = {0x32};
    ldnnA.operandBytes = 2;
    ldnnA.cycles = 13;
    instructions_.push_back(ldnnA);
    
    // LD A,I and LD A,R (Z80 specific)
    InstructionInfo ldAI;
    ldAI.mnemonic = "LD";
    ldAI.operandPattern = "A,I";
    ldAI.mode = AddressingMode::Register;
    ldAI.opcodes = {0xED, 0x57};
    ldAI.operandBytes = 0;
    ldAI.cycles = 9;
    instructions_.push_back(ldAI);
    
    InstructionInfo ldAR;
    ldAR.mnemonic = "LD";
    ldAR.operandPattern = "A,R";
    ldAR.mode = AddressingMode::Register;
    ldAR.opcodes = {0xED, 0x5F};
    ldAR.operandBytes = 0;
    ldAR.cycles = 9;
    instructions_.push_back(ldAR);
    
    // LD I,A and LD R,A (Z80 specific)
    InstructionInfo ldIA;
    ldIA.mnemonic = "LD";
    ldIA.operandPattern = "I,A";
    ldIA.mode = AddressingMode::Register;
    ldIA.opcodes = {0xED, 0x47};
    ldIA.operandBytes = 0;
    ldIA.cycles = 9;
    instructions_.push_back(ldIA);
    
    InstructionInfo ldRA;
    ldRA.mnemonic = "LD";
    ldRA.operandPattern = "R,A";
    ldRA.mode = AddressingMode::Register;
    ldRA.opcodes = {0xED, 0x4F};
    ldRA.operandBytes = 0;
    ldRA.cycles = 9;
    instructions_.push_back(ldRA);
}

void Z80Instructions::addLoad16BitInstructions() {
    // LD dd,nn (dd = BC,DE,HL,SP)
    const char* regPairs[] = {"BC", "DE", "HL", "SP"};
    Byte baseOpcode = 0x01;
    
    for (int rp = 0; rp < 4; rp++) {
        InstructionInfo info;
        info.mnemonic = "LD";
        info.operandPattern = std::string(regPairs[rp]) + ",NN";
        info.mode = AddressingMode::ImmediateExtended;
        info.opcodes = {static_cast<Byte>(baseOpcode + (rp << 4))};
        info.operandBytes = 2;
        info.cycles = 10;
        instructions_.push_back(info);
    }
    
    // LD HL,(nn)
    InstructionInfo ldHLnn;
    ldHLnn.mnemonic = "LD";
    ldHLnn.operandPattern = "HL,(NN)";
    ldHLnn.mode = AddressingMode::Direct;
    ldHLnn.opcodes = {0x2A};
    ldHLnn.operandBytes = 2;
    ldHLnn.cycles = 16;
    instructions_.push_back(ldHLnn);
    
    // LD (nn),HL
    InstructionInfo ldnnHL;
    ldnnHL.mnemonic = "LD";
    ldnnHL.operandPattern = "(NN),HL";
    ldnnHL.mode = AddressingMode::Direct;
    ldnnHL.opcodes = {0x22};
    ldnnHL.operandBytes = 2;
    ldnnHL.cycles = 16;
    instructions_.push_back(ldnnHL);
    
    // LD SP,HL
    InstructionInfo ldSPHL;
    ldSPHL.mnemonic = "LD";
    ldSPHL.operandPattern = "SP,HL";
    ldSPHL.mode = AddressingMode::Register;
    ldSPHL.opcodes = {0xF9};
    ldSPHL.operandBytes = 0;
    ldSPHL.cycles = 6;
    instructions_.push_back(ldSPHL);
    
    // PUSH and POP
    baseOpcode = 0xC5;
    const char* pushPopPairs[] = {"BC", "DE", "HL", "AF"};
    
    for (int rp = 0; rp < 4; rp++) {
        // PUSH
        InstructionInfo push;
        push.mnemonic = "PUSH";
        push.operandPattern = pushPopPairs[rp];
        push.mode = AddressingMode::Register;
        push.opcodes = {static_cast<Byte>(baseOpcode + (rp << 4))};
        push.operandBytes = 0;
        push.cycles = 11;
        instructions_.push_back(push);
        
        // POP
        InstructionInfo pop;
        pop.mnemonic = "POP";
        pop.operandPattern = pushPopPairs[rp];
        pop.mode = AddressingMode::Register;
        pop.opcodes = {static_cast<Byte>(0xC1 + (rp << 4))};
        pop.operandBytes = 0;
        pop.cycles = 10;
        instructions_.push_back(pop);
    }
}

void Z80Instructions::addArithmeticInstructions() {
    // ADD, ADC, SUB, SBC, AND, XOR, OR, CP with register
    const char* mnemonics[] = {"ADD", "ADC", "SUB", "SBC", "AND", "XOR", "OR", "CP"};
    const Byte opcodes[] = {0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8};
    const char* regs = "BCDEHL_A";  // Z80 encoding order
    
    for (int op = 0; op < 8; op++) {
        for (int reg = 0; reg < 8; reg++) {
            if (reg == 6) continue; // (HL) handled separately
            
            // Explicit A,r form
            InstructionInfo info;
            info.mnemonic = mnemonics[op];
            info.operandPattern = "A," + std::string(1, regs[reg]);
            info.mode = AddressingMode::Register;
            info.opcodes = {static_cast<Byte>(opcodes[op] + reg)};
            info.operandBytes = 0;
            info.cycles = 4;
            instructions_.push_back(info);
            
            // Implicit A (just "r" form) - same opcode
            if (op >= 2) { // SUB, SBC, AND, XOR, OR, CP support implicit A
                InstructionInfo infoImplicit;
                infoImplicit.mnemonic = mnemonics[op];
                infoImplicit.operandPattern = std::string(1, regs[reg]);
                infoImplicit.mode = AddressingMode::Register;
                infoImplicit.opcodes = {static_cast<Byte>(opcodes[op] + reg)};
                infoImplicit.operandBytes = 0;
                infoImplicit.cycles = 4;
                instructions_.push_back(infoImplicit);
            }
        }
        
        // With (HL) - explicit A
        InstructionInfo infoHL;
        infoHL.mnemonic = mnemonics[op];
        infoHL.operandPattern = "A,(HL)";
        infoHL.mode = AddressingMode::RegisterIndirect;
        infoHL.opcodes = {static_cast<Byte>(opcodes[op] + 6)};
        infoHL.operandBytes = 0;
        infoHL.cycles = 7;
        instructions_.push_back(infoHL);
        
        // With (HL) - implicit A
        if (op >= 2) {
            InstructionInfo infoHLImplicit;
            infoHLImplicit.mnemonic = mnemonics[op];
            infoHLImplicit.operandPattern = "(HL)";
            infoHLImplicit.mode = AddressingMode::RegisterIndirect;
            infoHLImplicit.opcodes = {static_cast<Byte>(opcodes[op] + 6)};
            infoHLImplicit.operandBytes = 0;
            infoHLImplicit.cycles = 7;
            instructions_.push_back(infoHLImplicit);
        }
        
        // With immediate - explicit A
        InstructionInfo infoImm;
        infoImm.mnemonic = mnemonics[op];
        infoImm.operandPattern = "A,N";
        infoImm.mode = AddressingMode::Immediate;
        infoImm.opcodes = {static_cast<Byte>(opcodes[op] + 0x46)};
        infoImm.operandBytes = 1;
        infoImm.cycles = 7;
        instructions_.push_back(infoImm);
        
        // With immediate - implicit A
        if (op >= 2) {
            InstructionInfo infoImmImplicit;
            infoImmImplicit.mnemonic = mnemonics[op];
            infoImmImplicit.operandPattern = "N";
            infoImmImplicit.mode = AddressingMode::Immediate;
            infoImmImplicit.opcodes = {static_cast<Byte>(opcodes[op] + 0x46)};
            infoImmImplicit.operandBytes = 1;
            infoImmImplicit.cycles = 7;
            instructions_.push_back(infoImmImplicit);
        }
    }
    
    // INC and DEC register
    const char* regs8 = "BCDEHLFA"; // F position is (HL)
    Byte incBase = 0x04;
    Byte decBase = 0x05;
    
    for (int reg = 0; reg < 8; reg++) {
        if (reg == 6) { // (HL)
            InstructionInfo incHL;
            incHL.mnemonic = "INC";
            incHL.operandPattern = "(HL)";
            incHL.mode = AddressingMode::RegisterIndirect;
            incHL.opcodes = {0x34};
            incHL.operandBytes = 0;
            incHL.cycles = 11;
            instructions_.push_back(incHL);
            
            InstructionInfo decHL;
            decHL.mnemonic = "DEC";
            decHL.operandPattern = "(HL)";
            decHL.mode = AddressingMode::RegisterIndirect;
            decHL.opcodes = {0x35};
            decHL.operandBytes = 0;
            decHL.cycles = 11;
            instructions_.push_back(decHL);
        } else {
            // INC r
            InstructionInfo inc;
            inc.mnemonic = "INC";
            inc.operandPattern = std::string(1, regs8[reg]);
            inc.mode = AddressingMode::Register;
            inc.opcodes = {static_cast<Byte>(incBase + (reg << 3))};
            inc.operandBytes = 0;
            inc.cycles = 4;
            instructions_.push_back(inc);
            
            // DEC r
            InstructionInfo dec;
            dec.mnemonic = "DEC";
            dec.operandPattern = std::string(1, regs8[reg]);
            dec.mode = AddressingMode::Register;
            dec.opcodes = {static_cast<Byte>(decBase + (reg << 3))};
            dec.operandBytes = 0;
            dec.cycles = 4;
            instructions_.push_back(dec);
        }
    }
    
    // INC and DEC 16-bit
    const char* regPairs[] = {"BC", "DE", "HL", "SP"};
    for (int rp = 0; rp < 4; rp++) {
        // INC rr
        InstructionInfo inc16;
        inc16.mnemonic = "INC";
        inc16.operandPattern = regPairs[rp];
        inc16.mode = AddressingMode::Register;
        inc16.opcodes = {static_cast<Byte>(0x03 + (rp << 4))};
        inc16.operandBytes = 0;
        inc16.cycles = 6;
        instructions_.push_back(inc16);
        
        // DEC rr
        InstructionInfo dec16;
        dec16.mnemonic = "DEC";
        dec16.operandPattern = regPairs[rp];
        dec16.mode = AddressingMode::Register;
        dec16.opcodes = {static_cast<Byte>(0x0B + (rp << 4))};
        dec16.operandBytes = 0;
        dec16.cycles = 6;
        instructions_.push_back(dec16);
    }
    
    // ADD HL,rr
    for (int rp = 0; rp < 4; rp++) {
        InstructionInfo addHL;
        addHL.mnemonic = "ADD";
        addHL.operandPattern = "HL," + std::string(regPairs[rp]);
        addHL.mode = AddressingMode::Register;
        addHL.opcodes = {static_cast<Byte>(0x09 + (rp << 4))};
        addHL.operandBytes = 0;
        addHL.cycles = 11;
        instructions_.push_back(addHL);
    }
}

void Z80Instructions::addLogicalInstructions() {
    // Covered in addArithmeticInstructions (AND, XOR, OR)
    // Add special logical operations here if needed
}

void Z80Instructions::addRotateShiftInstructions() {
    // RLCA, RRCA, RLA, RRA
    InstructionInfo rlca = {"RLCA", AddressingMode::Implied, {0x07}, 0, 4, ""};
    InstructionInfo rrca = {"RRCA", AddressingMode::Implied, {0x0F}, 0, 4, ""};
    InstructionInfo rla = {"RLA", AddressingMode::Implied, {0x17}, 0, 4, ""};
    InstructionInfo rra = {"RRA", AddressingMode::Implied, {0x1F}, 0, 4, ""};
    
    instructions_.push_back(rlca);
    instructions_.push_back(rrca);
    instructions_.push_back(rla);
    instructions_.push_back(rra);
}

void Z80Instructions::addBitInstructions() {
    // BIT, SET, RES b,r
    // Will be implemented with CB prefix
    // TODO: Add CB-prefixed instructions
}

void Z80Instructions::addJumpCallReturnInstructions() {
    // JP nn
    InstructionInfo jpnn;
    jpnn.mnemonic = "JP";
    jpnn.operandPattern = "NN";
    jpnn.mode = AddressingMode::Extended;
    jpnn.opcodes = {0xC3};
    jpnn.operandBytes = 2;
    jpnn.cycles = 10;
    instructions_.push_back(jpnn);
    
    // JP (HL)
    InstructionInfo jpHL;
    jpHL.mnemonic = "JP";
    jpHL.operandPattern = "(HL)";
    jpHL.mode = AddressingMode::RegisterIndirect;
    jpHL.opcodes = {0xE9};
    jpHL.operandBytes = 0;
    jpHL.cycles = 4;
    instructions_.push_back(jpHL);
    
    // JR e (relative offset)
    InstructionInfo jre;
    jre.mnemonic = "JR";
    jre.operandPattern = "E";
    jre.mode = AddressingMode::Relative;
    jre.opcodes = {0x18};
    jre.operandBytes = 1;
    jre.cycles = 12;
    instructions_.push_back(jre);
    
    // JR nn (label - will be converted to relative in code generation)
    InstructionInfo jrLabel;
    jrLabel.mnemonic = "JR";
    jrLabel.operandPattern = "NN";
    jrLabel.mode = AddressingMode::Relative;
    jrLabel.opcodes = {0x18};
    jrLabel.operandBytes = 1;
    jrLabel.cycles = 12;
    instructions_.push_back(jrLabel);
    
    // CALL nn
    InstructionInfo callnn;
    callnn.mnemonic = "CALL";
    callnn.operandPattern = "NN";
    callnn.mode = AddressingMode::Extended;
    callnn.opcodes = {0xCD};
    callnn.operandBytes = 2;
    callnn.cycles = 17;
    instructions_.push_back(callnn);
    
    // RET
    InstructionInfo ret;
    ret.mnemonic = "RET";
    ret.operandPattern = "";
    ret.mode = AddressingMode::Implied;
    ret.opcodes = {0xC9};
    ret.operandBytes = 0;
    ret.cycles = 10;
    instructions_.push_back(ret);
    
    // RST p (0, 8, 16, 24, 32, 40, 48, 56)
    for (int p = 0; p < 8; p++) {
        InstructionInfo rst;
        rst.mnemonic = "RST";
        rst.operandPattern = std::to_string(p * 8);
        rst.mode = AddressingMode::Implied;
        rst.opcodes = {static_cast<Byte>(0xC7 + (p << 3))};
        rst.operandBytes = 0;
        rst.cycles = 11;
        instructions_.push_back(rst);
    }
}

void Z80Instructions::addIOInstructions() {
    // IN A,(n)
    InstructionInfo inAn;
    inAn.mnemonic = "IN";
    inAn.operandPattern = "A,(N)";
    inAn.mode = AddressingMode::Immediate;
    inAn.opcodes = {0xDB};
    inAn.operandBytes = 1;
    inAn.cycles = 11;
    instructions_.push_back(inAn);
    
    // OUT (n),A
    InstructionInfo outnA;
    outnA.mnemonic = "OUT";
    outnA.operandPattern = "(N),A";
    outnA.mode = AddressingMode::Immediate;
    outnA.opcodes = {0xD3};
    outnA.operandBytes = 1;
    outnA.cycles = 11;
    instructions_.push_back(outnA);
}

void Z80Instructions::addMiscInstructions() {
    // NOP
    InstructionInfo nop;
    nop.mnemonic = "NOP";
    nop.operandPattern = "";
    nop.mode = AddressingMode::Implied;
    nop.opcodes = {0x00};
    nop.operandBytes = 0;
    nop.cycles = 4;
    instructions_.push_back(nop);
    
    // HALT
    InstructionInfo halt;
    halt.mnemonic = "HALT";
    halt.operandPattern = "";
    halt.mode = AddressingMode::Implied;
    halt.opcodes = {0x76};
    halt.operandBytes = 0;
    halt.cycles = 4;
    instructions_.push_back(halt);
    
    // DI and EI
    InstructionInfo di;
    di.mnemonic = "DI";
    di.operandPattern = "";
    di.mode = AddressingMode::Implied;
    di.opcodes = {0xF3};
    di.operandBytes = 0;
    di.cycles = 4;
    instructions_.push_back(di);
    
    InstructionInfo ei;
    ei.mnemonic = "EI";
    ei.operandPattern = "";
    ei.mode = AddressingMode::Implied;
    ei.opcodes = {0xFB};
    ei.operandBytes = 0;
    ei.cycles = 4;
    instructions_.push_back(ei);
}

} // namespace z80
