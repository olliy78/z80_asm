/**
 * @file z80_instructions.h
 * @brief Z80 instruction set definition and encoding
 * 
 * Contains the complete Z80 instruction set table with opcodes,
 * addressing modes, and encoding rules.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include "common/types.h"

namespace z80 {

/**
 * @struct InstructionInfo
 * @brief Information about a single Z80 instruction variant
 */
struct InstructionInfo {
    std::string mnemonic;        ///< Instruction mnemonic (e.g., "LD", "ADD")
    AddressingMode mode;         ///< Addressing mode
    std::vector<Byte> opcodes;   ///< Opcode bytes (may include prefix bytes)
    int operandBytes;            ///< Number of operand bytes following opcode
    int cycles;                  ///< Clock cycles (base timing)
    std::string operandPattern;  ///< Pattern for operand matching (e.g., "A,n", "HL,(nn)")
};

/**
 * @class Z80Instructions
 * @brief Complete Z80 instruction set table
 * 
 * Provides instruction lookup, validation, and opcode generation
 * for all Z80 instructions including:
 * - Standard 8080-compatible instructions
 * - Z80-specific extensions
 * - Indexed addressing with IX/IY
 * - Bit manipulation instructions
 */
class Z80Instructions {
public:
    /** @brief Initialize the instruction table */
    Z80Instructions();
    
    /**
     * @brief Find instruction matching mnemonic and operands
     * @param mnemonic Instruction mnemonic (e.g., "LD")
     * @param operands Operand string (e.g., "A,B")
     * @return Pointer to instruction info, or nullptr if not found
     */
    const InstructionInfo* findInstruction(const std::string& mnemonic, 
                                          const std::string& operands) const;
    
    /**
     * @brief Check if a string is a valid register name
     * @param name String to check
     * @return true if it's a register name
     */
    bool isRegister(const std::string& name) const;
    
    /**
     * @brief Check if a string is a valid mnemonic
     * @param name String to check
     * @return true if it's a valid mnemonic
     */
    bool isMnemonic(const std::string& name) const;
    
    /**
     * @brief Get the total number of instruction variants
     * @return Number of entries in the instruction table
     */
    size_t getInstructionCount() const { return instructions_.size(); }
    
private:
    /** @brief Initialize all instruction entries */
    void initializeInstructions();
    
    /** @brief Add 8-bit load instructions */
    void addLoad8BitInstructions();
    
    /** @brief Add 16-bit load instructions */
    void addLoad16BitInstructions();
    
    /** @brief Add arithmetic instructions */
    void addArithmeticInstructions();
    
    /** @brief Add logical instructions */
    void addLogicalInstructions();
    
    /** @brief Add rotate/shift instructions */
    void addRotateShiftInstructions();
    
    /** @brief Add bit manipulation instructions */
    void addBitInstructions();
    
    /** @brief Add jump/call/return instructions */
    void addJumpCallReturnInstructions();
    
    /** @brief Add I/O instructions */
    void addIOInstructions();
    
    /** @brief Add extended I/O instructions (ED prefix) */
    void addExtendedIOInstructions();
    
    /** @brief Add block transfer/compare instructions */
    void addBlockInstructions();
    
    /** @brief Add indexed addressing instructions (IX/IY) */
    void addIndexedInstructions();
    
    /** @brief Add miscellaneous instructions */
    void addMiscInstructions();
    
    std::vector<InstructionInfo> instructions_;  ///< Complete instruction table
};

} // namespace z80
