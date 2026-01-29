/**
 * @file rel_writer.h
 * @brief Microsoft Relocatable Object format (.rel) writer
 * 
 * Writes assembled code in Microsoft .REL format for linking.
 * Must be byte-for-byte compatible with M80 output.
 * 
 * Format includes:
 * - Program/Data/Common segments
 * - Symbol table (PUBLIC/EXTERNAL)
 * - Relocation information
 * - Module name and size information
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>

namespace z80 {

/**
 * @class RelWriter
 * @brief Writes Microsoft .REL format files
 * 
 * Generates Microsoft Relocatable Object Module format output.
 * Must match M80 output byte-for-byte for compatibility.
 * 
 * @todo Implement .REL format writer
 */
class RelWriter {
public:
    /**
     * @brief Construct a .REL file writer
     * @param filename Output filename
     */
    RelWriter(const std::string& filename);
    
    /**
     * @brief Write the .REL file
     * @todo Implement .REL format output
     */
    void write();
    
    /** @brief Close the .REL file */
    void close();
    
private:
    std::string filename_;  ///< Output filename
};

} // namespace z80
