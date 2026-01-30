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
            upper == "AF" || upper == "AF'" || upper == "BC" || upper == "DE" || upper == "HL" ||
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
    // RLCA, RRCA, RLA, RRA (Accumulator rotates)
    InstructionInfo rlca = {"RLCA", AddressingMode::Implied, {0x07}, 0, 4, ""};
    InstructionInfo rrca = {"RRCA", AddressingMode::Implied, {0x0F}, 0, 4, ""};
    InstructionInfo rla = {"RLA", AddressingMode::Implied, {0x17}, 0, 4, ""};
    InstructionInfo rra = {"RRA", AddressingMode::Implied, {0x1F}, 0, 4, ""};
    
    instructions_.push_back(rlca);
    instructions_.push_back(rrca);
    instructions_.push_back(rla);
    instructions_.push_back(rra);
    
    // CB-prefixed rotate/shift instructions
    const char* regs = "BCDEHL_A";
    const char* rotateOps[] = {"RLC", "RRC", "RL", "RR", "SLA", "SRA", "SRL"};
    const Byte rotateBase[] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x38};
    
    for (int op = 0; op < 7; op++) {
        for (int reg = 0; reg < 8; reg++) {
            if (reg == 6) continue; // (HL) handled separately
            
            InstructionInfo rotInst;
            rotInst.mnemonic = rotateOps[op];
            rotInst.operandPattern = std::string(1, regs[reg]);
            rotInst.mode = AddressingMode::Register;
            rotInst.opcodes = {0xCB, static_cast<Byte>(rotateBase[op] + reg)};
            rotInst.operandBytes = 0;
            rotInst.cycles = 8;
            instructions_.push_back(rotInst);
        }
        
        // Rotate/Shift (HL)
        InstructionInfo rotHL;
        rotHL.mnemonic = rotateOps[op];
        rotHL.operandPattern = "(HL)";
        rotHL.mode = AddressingMode::RegisterIndirect;
        rotHL.opcodes = {0xCB, static_cast<Byte>(rotateBase[op] + 6)};
        rotHL.operandBytes = 0;
        rotHL.cycles = 15;
        instructions_.push_back(rotHL);
    }
    
    // SLL (undocumented, but widely used)
    for (int reg = 0; reg < 8; reg++) {
        if (reg == 6) continue;
        
        InstructionInfo sll;
        sll.mnemonic = "SLL";
        sll.operandPattern = std::string(1, regs[reg]);
        sll.mode = AddressingMode::Register;
        sll.opcodes = {0xCB, static_cast<Byte>(0x30 + reg)};
        sll.operandBytes = 0;
        sll.cycles = 8;
        instructions_.push_back(sll);
    }
    
    InstructionInfo sllHL;
    sllHL.mnemonic = "SLL";
    sllHL.operandPattern = "(HL)";
    sllHL.mode = AddressingMode::RegisterIndirect;
    sllHL.opcodes = {0xCB, 0x36};
    sllHL.operandBytes = 0;
    sllHL.cycles = 15;
    instructions_.push_back(sllHL);
}

void Z80Instructions::addBitInstructions() {
    // BIT, SET, RES b,r (CB-prefixed)
    const char* regs = "BCDEHL_A";  // Z80 register encoding
    
    // BIT b,r - Test bit b in register r
    for (int bit = 0; bit < 8; bit++) {
        for (int reg = 0; reg < 8; reg++) {
            if (reg == 6) continue; // (HL) handled separately
            InstructionInfo bitInst;
            bitInst.mnemonic = "BIT";
            bitInst.operandPattern = std::to_string(bit) + "," + std::string(1, regs[reg]);
            bitInst.mode = AddressingMode::Immediate;
            bitInst.opcodes = {0xCB, static_cast<Byte>(0x40 + (bit << 3) + reg)};
            bitInst.operandBytes = 0;
            bitInst.cycles = 8;
            instructions_.push_back(bitInst);
        }
        
        // BIT b,(HL)
        InstructionInfo bitHL;
        bitHL.mnemonic = "BIT";
        bitHL.operandPattern = std::to_string(bit) + ",(HL)";
        bitHL.mode = AddressingMode::RegisterIndirect;
        bitHL.opcodes = {0xCB, static_cast<Byte>(0x46 + (bit << 3))};
        bitHL.operandBytes = 0;
        bitHL.cycles = 12;
        instructions_.push_back(bitHL);
    }
    
    // SET b,r - Set bit b in register r
    for (int bit = 0; bit < 8; bit++) {
        for (int reg = 0; reg < 8; reg++) {
            if (reg == 6) continue;
            InstructionInfo setInst;
            setInst.mnemonic = "SET";
            setInst.operandPattern = std::to_string(bit) + "," + std::string(1, regs[reg]);
            setInst.mode = AddressingMode::Immediate;
            setInst.opcodes = {0xCB, static_cast<Byte>(0xC0 + (bit << 3) + reg)};
            setInst.operandBytes = 0;
            setInst.cycles = 8;
            instructions_.push_back(setInst);
        }
        
        // SET b,(HL)
        InstructionInfo setHL;
        setHL.mnemonic = "SET";
        setHL.operandPattern = std::to_string(bit) + ",(HL)";
        setHL.mode = AddressingMode::RegisterIndirect;
        setHL.opcodes = {0xCB, static_cast<Byte>(0xC6 + (bit << 3))};
        setHL.operandBytes = 0;
        setHL.cycles = 15;
        instructions_.push_back(setHL);
    }
    
    // RES b,r - Reset bit b in register r
    for (int bit = 0; bit < 8; bit++) {
        for (int reg = 0; reg < 8; reg++) {
            if (reg == 6) continue;
            InstructionInfo resInst;
            resInst.mnemonic = "RES";
            resInst.operandPattern = std::to_string(bit) + "," + std::string(1, regs[reg]);
            resInst.mode = AddressingMode::Immediate;
            resInst.opcodes = {0xCB, static_cast<Byte>(0x80 + (bit << 3) + reg)};
            resInst.operandBytes = 0;
            resInst.cycles = 8;
            instructions_.push_back(resInst);
        }
        
        // RES b,(HL)
        InstructionInfo resHL;
        resHL.mnemonic = "RES";
        resHL.operandPattern = std::to_string(bit) + ",(HL)";
        resHL.mode = AddressingMode::RegisterIndirect;
        resHL.opcodes = {0xCB, static_cast<Byte>(0x86 + (bit << 3))};
        resHL.operandBytes = 0;
        resHL.cycles = 15;
        instructions_.push_back(resHL);
    }
}

void Z80Instructions::addIOInstructions() {
    // IN A,(N)
    InstructionInfo inAN;
    inAN.mnemonic = "IN";
    inAN.operandPattern = "A,(N)";
    inAN.mode = AddressingMode::Immediate;
    inAN.opcodes = {0xDB};
    inAN.operandBytes = 1;
    inAN.cycles = 11;
    instructions_.push_back(inAN);
    
    // OUT (N),A
    InstructionInfo outNA;
    outNA.mnemonic = "OUT";
    outNA.operandPattern = "(N),A";
    outNA.mode = AddressingMode::Immediate;
    outNA.opcodes = {0xD3};
    outNA.operandBytes = 1;
    outNA.cycles = 11;
    instructions_.push_back(outNA);
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
    
    // Conditional JP cc,nn
    const char* conditions[] = {"NZ", "Z", "NC", "C", "PO", "PE", "P", "M"};
    const Byte jpCondOpcodes[] = {0xC2, 0xCA, 0xD2, 0xDA, 0xE2, 0xEA, 0xF2, 0xFA};
    
    for (int i = 0; i < 8; i++) {
        InstructionInfo jpCond;
        jpCond.mnemonic = "JP";
        jpCond.operandPattern = std::string(conditions[i]) + ",NN";
        jpCond.mode = AddressingMode::Extended;
        jpCond.opcodes = {jpCondOpcodes[i]};
        jpCond.operandBytes = 2;
        jpCond.cycles = 10;
        instructions_.push_back(jpCond);
    }
    
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
    
    // Conditional JR (only Z, NZ, C, NC)
    const char* jrConditions[] = {"NZ", "Z", "NC", "C"};
    const Byte jrCondOpcodes[] = {0x20, 0x28, 0x30, 0x38};
    
    for (int i = 0; i < 4; i++) {
        // With offset E
        InstructionInfo jrCondE;
        jrCondE.mnemonic = "JR";
        jrCondE.operandPattern = std::string(jrConditions[i]) + ",E";
        jrCondE.mode = AddressingMode::Relative;
        jrCondE.opcodes = {jrCondOpcodes[i]};
        jrCondE.operandBytes = 1;
        jrCondE.cycles = 12;  // 12 if taken, 7 if not
        instructions_.push_back(jrCondE);
        
        // With label (NN) - will be converted to relative
        InstructionInfo jrCondLabel;
        jrCondLabel.mnemonic = "JR";
        jrCondLabel.operandPattern = std::string(jrConditions[i]) + ",NN";
        jrCondLabel.mode = AddressingMode::Relative;
        jrCondLabel.opcodes = {jrCondOpcodes[i]};
        jrCondLabel.operandBytes = 1;
        jrCondLabel.cycles = 12;
        instructions_.push_back(jrCondLabel);
    }
    
    // CALL nn
    InstructionInfo callnn;
    callnn.mnemonic = "CALL";
    callnn.operandPattern = "NN";
    callnn.mode = AddressingMode::Extended;
    callnn.opcodes = {0xCD};
    callnn.operandBytes = 2;
    callnn.cycles = 17;
    instructions_.push_back(callnn);
    
    // Conditional CALL cc,nn
    const Byte callCondOpcodes[] = {0xC4, 0xCC, 0xD4, 0xDC, 0xE4, 0xEC, 0xF4, 0xFC};
    
    for (int i = 0; i < 8; i++) {
        InstructionInfo callCond;
        callCond.mnemonic = "CALL";
        callCond.operandPattern = std::string(conditions[i]) + ",NN";
        callCond.mode = AddressingMode::Extended;
        callCond.opcodes = {callCondOpcodes[i]};
        callCond.operandBytes = 2;
        callCond.cycles = 17;  // 17 if taken, 10 if not
        instructions_.push_back(callCond);
    }
    
    // RET
    InstructionInfo ret;
    ret.mnemonic = "RET";
    ret.operandPattern = "";
    ret.mode = AddressingMode::Implied;
    ret.opcodes = {0xC9};
    ret.operandBytes = 0;
    ret.cycles = 10;
    instructions_.push_back(ret);
    
    // Conditional RET cc
    const Byte retCondOpcodes[] = {0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8};
    
    for (int i = 0; i < 8; i++) {
        InstructionInfo retCond;
        retCond.mnemonic = "RET";
        retCond.operandPattern = std::string(conditions[i]);
        retCond.mode = AddressingMode::Implied;
        retCond.opcodes = {retCondOpcodes[i]};
        retCond.operandBytes = 0;
        retCond.cycles = 11;  // 11 if taken, 5 if not
        instructions_.push_back(retCond);
    }
    
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
    
    // PUSH and POP for register pairs
    const char* pushPopRegs[] = {"BC", "DE", "HL", "AF"};
    const Byte pushOpcodes[] = {0xC5, 0xD5, 0xE5, 0xF5};
    const Byte popOpcodes[] = {0xC1, 0xD1, 0xE1, 0xF1};
    
    for (int i = 0; i < 4; i++) {
        InstructionInfo push;
        push.mnemonic = "PUSH";
        push.operandPattern = pushPopRegs[i];
        push.mode = AddressingMode::Register;
        push.opcodes = {pushOpcodes[i]};
        push.operandBytes = 0;
        push.cycles = 11;
        instructions_.push_back(push);
        
        InstructionInfo pop;
        pop.mnemonic = "POP";
        pop.operandPattern = pushPopRegs[i];
        pop.mode = AddressingMode::Register;
        pop.opcodes = {popOpcodes[i]};
        pop.operandBytes = 0;
        pop.cycles = 10;
        instructions_.push_back(pop);
    }
    
    // EX DE,HL
    InstructionInfo exDEHL;
    exDEHL.mnemonic = "EX";
    exDEHL.operandPattern = "DE,HL";
    exDEHL.mode = AddressingMode::Register;
    exDEHL.opcodes = {0xEB};
    exDEHL.operandBytes = 0;
    exDEHL.cycles = 4;
    instructions_.push_back(exDEHL);
    
    // EX AF,AF'
    InstructionInfo exAF;
    exAF.mnemonic = "EX";
    exAF.operandPattern = "AF,AF'";
    exAF.mode = AddressingMode::Register;
    exAF.opcodes = {0x08};
    exAF.operandBytes = 0;
    exAF.cycles = 4;
    instructions_.push_back(exAF);
    
    // EX (SP),HL
    InstructionInfo exSPHL;
    exSPHL.mnemonic = "EX";
    exSPHL.operandPattern = "(SP),HL";
    exSPHL.mode = AddressingMode::RegisterIndirect;
    exSPHL.opcodes = {0xE3};
    exSPHL.operandBytes = 0;
    exSPHL.cycles = 19;
    instructions_.push_back(exSPHL);
    
    // EXX
    InstructionInfo exx;
    exx.mnemonic = "EXX";
    exx.operandPattern = "";
    exx.mode = AddressingMode::Implied;
    exx.opcodes = {0xD9};
    exx.operandBytes = 0;
    exx.cycles = 4;
    instructions_.push_back(exx);
    
    // DAA (Decimal Adjust Accumulator)
    InstructionInfo daa;
    daa.mnemonic = "DAA";
    daa.operandPattern = "";
    daa.mode = AddressingMode::Implied;
    daa.opcodes = {0x27};
    daa.operandBytes = 0;
    daa.cycles = 4;
    instructions_.push_back(daa);
    
    // CPL (Complement)
    InstructionInfo cpl;
    cpl.mnemonic = "CPL";
    cpl.operandPattern = "";
    cpl.mode = AddressingMode::Implied;
    cpl.opcodes = {0x2F};
    cpl.operandBytes = 0;
    cpl.cycles = 4;
    instructions_.push_back(cpl);
    
    // NEG (Negate)
    InstructionInfo neg;
    neg.mnemonic = "NEG";
    neg.operandPattern = "";
    neg.mode = AddressingMode::Implied;
    neg.opcodes = {0xED, 0x44};
    neg.operandBytes = 0;
    neg.cycles = 8;
    instructions_.push_back(neg);
    
    // CCF (Complement Carry Flag)
    InstructionInfo ccf;
    ccf.mnemonic = "CCF";
    ccf.operandPattern = "";
    ccf.mode = AddressingMode::Implied;
    ccf.opcodes = {0x3F};
    ccf.operandBytes = 0;
    ccf.cycles = 4;
    instructions_.push_back(ccf);
    
    // SCF (Set Carry Flag)
    InstructionInfo scf;
    scf.mnemonic = "SCF";
    scf.operandPattern = "";
    scf.mode = AddressingMode::Implied;
    scf.opcodes = {0x37};
    scf.operandBytes = 0;
    scf.cycles = 4;
    instructions_.push_back(scf);
}

} // namespace z80
