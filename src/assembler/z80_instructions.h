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
 * @class Z80Instructions
 * @brief Complete Z80 instruction set table
 * 
 * Provides instruction lookup, validation, and opcode generation
 * for all Z80 instructions including:
 * - Standard 8080-compatible instructions
 * - Z80-specific extensions
 * - Indexed addressing with IX/IY
 * - Bit manipulation instructions
 * 
 * @todo Implement instruction table
 */
class Z80Instructions {
public:
    Z80Instructions();
    
    // TODO: Complete Z80 instruction table
};

} // namespace z80
