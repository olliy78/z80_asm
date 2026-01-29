/**
 * @file listing.h
 * @brief Assembly listing file (.prn) generator
 * 
 * Generates formatted listing output compatible with M80's .prn format.
 * Includes:
 * - Page headers with title and page numbers
 * - Address, machine code, and source text columns
 * - Symbol table at end of listing
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>

namespace z80 {

/**
 * @class Listing
 * @brief Generates assembly listing files
 * 
 * Creates formatted .prn output files compatible with Microsoft M80.
 * 
 * @todo Implement listing generation
 */
class Listing {
public:
    /**
     * @brief Construct a listing generator
     * @param filename Output filename for listing
     */
    Listing(const std::string& filename);
    
    /**
     * @brief Write a line to the listing
     * @param line Line to write
     * @todo Implement formatted output
     */
    void writeLine(const std::string& line);
    
    /** @brief Close the listing file */
    void close();
    
private:
    std::string filename_;  ///< Output filename
};

} // namespace z80
