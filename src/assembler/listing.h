/**
 * @file listing.h
 * @brief Assembly listing file (.prn) generator
 * 
 * Generates formatted listing output compatible with M80's .prn format.
 * Includes:
 * - Page headers with title and page numbers
 * - Address, machine code, and source text columns
 * - Symbol table at end of listing
 * - Macro expansion tracking
 * 
 * Format:
 * AAAA BBBBBBBBBB   Source line
 * Where:
 * - AAAA = Address (4 hex digits)
 * - BBBBBBBBBB = Machine code (up to 10 hex digits, 5 bytes)
 * - Source line = Original source text
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include "common/types.h"
#include "symbol_table.h"
#include <string>
#include <vector>
#include <fstream>
#include <memory>

namespace z80 {

/**
 * @struct ListingLine
 * @brief Single line in listing output
 */
struct ListingLine {
    int lineNumber;                 ///< Source line number
    Address address;                ///< Assembly address
    std::vector<Byte> code;         ///< Generated machine code
    std::string source;             ///< Source text
    bool hasAddress;                ///< True if line has an address
    bool isError;                   ///< True if line has error
    std::string errorMessage;       ///< Error message if any
};

/**
 * @class Listing
 * @brief Generates assembly listing files
 * 
 * Creates formatted .prn output files compatible with Microsoft M80.
 */
class Listing {
public:
    /**
     * @brief Construct a listing generator
     * @param filename Output filename for listing
     * @param title Optional title for page headers
     */
    Listing(const std::string& filename, const std::string& title = "");
    
    /**
     * @brief Destructor - ensures file is closed
     */
    ~Listing();
    
    /**
     * @brief Add a line to the listing
     * @param line Listing line to add
     */
    void addLine(const ListingLine& line);
    
    /**
     * @brief Set page title
     * @param title Title text
     */
    void setTitle(const std::string& title);
    
    /**
     * @brief Set page length
     * @param lines Lines per page (10-255)
     */
    void setPageLength(int lines);
    
    /**
     * @brief Enable/disable symbol table output
     * @param enable true to enable
     */
    void setSymbolTableOutput(bool enable);
    
    /**
     * @brief Write symbol table at end of listing
     * @param symbolTable Symbol table to output
     */
    void writeSymbolTable(const SymbolTable& symbolTable);
    
    /**
     * @brief Finalize and close the listing file
     * @return true if successful
     */
    bool close();
    
    /**
     * @brief Check if listing file is open
     * @return true if open
     */
    bool isOpen() const { return file_.is_open(); }

private:
    std::string filename_;          ///< Output filename
    std::ofstream file_;            ///< Output file stream
    std::string title_;             ///< Page header title
    int pageLength_;                ///< Lines per page
    int currentLine_;               ///< Current line on page
    int pageNumber_;                ///< Current page number
    bool symbolTableOutput_;        ///< Output symbol table?
    std::vector<ListingLine> lines_; ///< Accumulated lines
    
    /**
     * @brief Write page header
     */
    void writePageHeader();
    
    /**
     * @brief Write a single listing line
     * @param line Line to write
     */
    void writeListingLine(const ListingLine& line);
    
    /**
     * @brief Format machine code bytes as hex string
     * @param code Byte vector
     * @param offset Start offset in vector
     * @param count Number of bytes to format
     * @return Formatted hex string
     */
    std::string formatMachineCode(const std::vector<Byte>& code, size_t offset, size_t count);
    
    /**
     * @brief Format address as 4-digit hex
     * @param addr Address value
     * @return Formatted address string
     */
    std::string formatAddress(Address addr);
    
    /**
     * @brief Check if new page needed and handle page break
     */
    void checkPageBreak();
};

} // namespace z80
