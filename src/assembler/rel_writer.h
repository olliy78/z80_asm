/**
 * @file rel_writer.h
 * @brief Writer for .REL (relocatable object) file format
 * 
 * Implements the Microsoft/Digital Research .REL format used by M80.
 * The format uses a bitstream encoding with special link items and
 * code/data items containing relocation information.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include "bit_writer.h"
#include "symbol_table.h"
#include "common/types.h"
#include <string>
#include <vector>
#include <fstream>

namespace z80 {

// Forward declaration
class Parser;
struct ParsedLine;

/**
 * @enum ItemType
 * @brief Type bits for .REL items
 */
enum class ItemType : uint8_t {
    Special = 0,      ///< 00 = Special link item
    Absolute = 1,     ///< 01 = Absolute (ASEG)
    ProgramRel = 2,   ///< 10 = Program relative (CSEG)
    DataRel = 3,      ///< 11 = Data relative (DSEG)
    CommonRel = 3     ///< 11 = Common relative (COMMON)
};

/**
 * @enum SpecialLinkCode
 * @brief Code field for special link items
 */
enum class SpecialLinkCode : uint8_t {
    EntrySymbol = 0,      ///< 0000 = Entry symbol
    SelectCommon = 1,     ///< 0001 = Select common block
    ProgramName = 2,      ///< 0010 = Program (module) name
    RequestLibrary = 3,   ///< 0011 = Request library search
    ExtensionLink = 4,    ///< 0100 = Extension link item
    CommonSize = 5,       ///< 0101 = Define common size
    ChainExternal = 6,    ///< 0110 = Chain external
    EntryPoint = 7,       ///< 0111 = Define entry point
    ExternalMinus = 8,    ///< 1000 = External - data
    ExternalPlus = 9,     ///< 1001 = External + data
    DataAreaSize = 10,    ///< 1010 = Data area size
    SetLocation = 11,     ///< 1011 = Set location counter
    ChainAddress = 12,    ///< 1100 = Chain address
    ProgramSize = 13,     ///< 1101 = Program area size
    EndModule = 14,       ///< 1110 = End of module
    EndFile = 15          ///< 1111 = End of file
};

/**
 * @class RELWriter
 * @brief Writes .REL relocatable object files
 */
class RELWriter {
public:
    /**
     * @brief Construct a new REL Writer
     */
    RELWriter();
    
    /**
     * @brief Write .REL file from assembled parser data
     * 
     * High-level method that generates a complete .REL file from a Parser.
     * Handles all aspects of REL generation:
     * - Module header and type detection
     * - Segment size calculation
     * - PUBLIC/EXTRN symbol export
     * - Code/data emission with proper segmentation
     * 
     * This is the preferred way to generate .REL files, maintaining proper
     * separation of concerns (Parser assembles, RELWriter outputs).
     * 
     * @param parser Parser instance with assembled code
     * @param filename Output .REL filename
     * @param moduleName Module name (derived from filename if empty)
     * @return true on success, false on error
     */
    bool writeFromParser(const Parser& parser, const std::string& filename, const std::string& moduleName = "");
    
    /**
     * @brief Begin a new module
     * @param moduleName Module/program name (max 6 significant chars)
     * @param isRelocatable true for relocatable (CSEG/DSEG), false for absolute (ASEG)
     */
    void beginModule(const std::string& moduleName, bool isRelocatable);
    
    /**
     * @brief Write program area size
     * @param size Size in bytes
     */
    void writeProgramSize(uint16_t size);
    
    /**
     * @brief Write data area size
     * @param size Size in bytes
     */
    void writeDataSize(uint16_t size);
    
    /**
     * @brief Write common block size
     * @param name Common block name (max 6 chars)
     * @param size Size in bytes
     */
    void writeCommonSize(const std::string& name, uint16_t size);
    
    /**
     * @brief Write entry symbol (PUBLIC symbol)
     * @param name Symbol name (max 6 chars)
     * @param value Symbol value
     * @param type Symbol type (Absolute/ProgramRel/DataRel)
     */
    void writeEntrySymbol(const std::string& name, uint16_t value, ItemType type);
    
    /**
     * @brief Write chain external (EXTRN symbol reference)
     * @param name External symbol name (max 6 chars)
     */
    void writeChainExternal(const std::string& name);
    
    /**
     * @brief Set location counter
     * @param address Address value
     * @param type Location type (Absolute/ProgramRel/DataRel)
     */
    void setLocation(uint16_t address, ItemType type);
    
    /**
     * @brief Write absolute data bytes
     * @param data Byte data to write
     */
    void writeAbsoluteData(const std::vector<Byte>& data);
    
    /**
     * @brief Write program-relative data bytes
     * @param data Byte data to write
     */
    void writeProgramData(const std::vector<Byte>& data);
    
    /**
     * @brief Write data-relative data bytes
     * @param data Byte data to write
     */
    void writeDataData(const std::vector<Byte>& data);
    
    /**
     * @brief Write relocatable address (chain address item)
     * @param offset Offset to relocatable address in current data stream
     */
    void writeChainAddress(uint16_t offset);
    
    /**
     * @brief End current module
     */
    void endModule();
    
    /**
     * @brief End file (can contain multiple modules)
     */
    void endFile();
    
    /**
     * @brief Write output to file
     * @param filename Output filename
     * @return true if successful
     */
    bool writeToFile(const std::string& filename);
    
    /**
     * @brief Get the output buffer
     * @return const reference to buffer
     */
    const std::vector<Byte>& getBuffer() const { return writer_.getBuffer(); }

private:
    BitWriter writer_;              ///< Bit writer for bitstream
    bool isRelocatable_;            ///< Current module relocatable?
    std::string currentModule_;     ///< Current module name
    
    /**
     * @brief Write special link item header
     * @param code Special link code
     */
    void writeSpecialItemHeader(SpecialLinkCode code);
    
    /**
     * @brief Write data item header
     * @param type Item type (Absolute/ProgramRel/DataRel)
     */
    void writeDataItemHeader(ItemType type);
    
    /**
     * @brief Write a symbol name (max 6 chars, padded with spaces)
     * @param name Symbol name
     */
    void writeSymbolName(const std::string& name);
    
    /**
     * @brief Truncate symbol name to 6 significant characters
     * @param name Symbol name
     * @return Truncated name
     */
    std::string truncateSymbolName(const std::string& name) const;
};

} // namespace z80
