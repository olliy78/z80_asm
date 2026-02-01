/**
 * @file rel_writer.cpp
 * @brief Implementation of .REL writer
 */

#include "rel_writer.h"
#include "parser.h"
#include <algorithm>
#include <cctype>

namespace z80 {

RELWriter::RELWriter()
    : isRelocatable_(false)
{
}

bool RELWriter::writeFromParser(const Parser& parser, const std::string& filename, const std::string& moduleName) {
    // Derive module name from filename if not provided
    std::string modName = moduleName;
    if (modName.empty()) {
        // Extract basename without extension
        size_t lastSlash = filename.find_last_of("/\\");
        size_t lastDot = filename.find_last_of('.');
        size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
        size_t end = (lastDot == std::string::npos) ? filename.length() : lastDot;
        modName = filename.substr(start, end - start);
    }
    
    const auto& lines = parser.getLines();
    const auto& symbolTable = parser.getSymbolTable();
    
    // Determine if module is relocatable (has CSEG or DSEG code)
    bool isRelocatable = false;
    for (const auto& line : lines) {
        if (!line.code.empty()) {
            if (line.segment == SegmentType::CSEG || line.segment == SegmentType::DSEG) {
                isRelocatable = true;
                break;
            }
        }
    }
    
    // Begin module
    beginModule(modName, isRelocatable);
    
    // Calculate segment sizes
    Address csegSize = 0, dsegSize = 0;
    for (const auto& line : lines) {
        if (!line.code.empty()) {
            if (line.segment == SegmentType::CSEG) {
                Address endAddr = line.address + line.code.size();
                if (endAddr > csegSize) csegSize = endAddr;
            } else if (line.segment == SegmentType::DSEG) {
                Address endAddr = line.address + line.code.size();
                if (endAddr > dsegSize) dsegSize = endAddr;
            }
        }
    }
    
    // Write sizes
    if (csegSize > 0) {
        writeProgramSize(csegSize);
    }
    if (dsegSize > 0) {
        writeDataSize(dsegSize);
    }
    
    // Write PUBLIC symbols (Entry Points)
    auto publicSymbols = symbolTable.getPublicSymbols();
    for (const Symbol* sym : publicSymbols) {
        // Find symbol name from symbol table
        std::string symbolName;
        for (const auto& pair : symbolTable.getAllSymbols()) {
            if (&pair.second == sym) {
                symbolName = pair.first;
                break;
            }
        }
        
        if (symbolName.empty()) continue;
        
        // Determine item type based on segment
        ItemType itemType = ItemType::Absolute;
        if (sym->segment == SegmentType::CSEG) {
            itemType = ItemType::ProgramRel;
        } else if (sym->segment == SegmentType::DSEG) {
            itemType = ItemType::DataRel;
        }
        
        writeEntrySymbol(symbolName, sym->value, itemType);
    }
    
    // Write EXTERNAL symbols (Chain Externals)
    auto externalSymbols = symbolTable.getExternalSymbols();
    for (const Symbol* sym : externalSymbols) {
        // Find symbol name from symbol table
        std::string symbolName;
        for (const auto& pair : symbolTable.getAllSymbols()) {
            if (&pair.second == sym) {
                symbolName = pair.first;
                break;
            }
        }
        
        if (symbolName.empty()) continue;
        
        writeChainExternal(symbolName);
    }
    
    // Write code/data by segment
    // Group consecutive bytes by segment
    std::vector<Byte> currentData;
    SegmentType currentSegType = SegmentType::CSEG;
    Address currentAddr = 0;
    bool hasData = false;
    
    for (const auto& line : lines) {
        if (line.code.empty()) continue;
        
        // If segment changed or address is not continuous, flush current data
        if (hasData && (line.segment != currentSegType || line.address != currentAddr)) {
            // Write accumulated data
            if (currentSegType == SegmentType::ASEG) {
                writeAbsoluteData(currentData);
            } else if (currentSegType == SegmentType::CSEG) {
                writeProgramData(currentData);
            } else if (currentSegType == SegmentType::DSEG) {
                writeDataData(currentData);
            }
            currentData.clear();
            hasData = false;
        }
        
        // If starting new segment, set location
        if (!hasData) {
            ItemType locType = ItemType::Absolute;
            if (line.segment == SegmentType::CSEG) {
                locType = ItemType::ProgramRel;
            } else if (line.segment == SegmentType::DSEG) {
                locType = ItemType::DataRel;
            }
            setLocation(line.address, locType);
            currentSegType = line.segment;
            currentAddr = line.address;
        }
        
        // Accumulate bytes
        for (Byte b : line.code) {
            currentData.push_back(b);
        }
        currentAddr += line.code.size();
        hasData = true;
    }
    
    // Flush remaining data
    if (hasData) {
        if (currentSegType == SegmentType::ASEG) {
            writeAbsoluteData(currentData);
        } else if (currentSegType == SegmentType::CSEG) {
            writeProgramData(currentData);
        } else if (currentSegType == SegmentType::DSEG) {
            writeDataData(currentData);
        }
    }
    
    // End module and file
    endModule();
    endFile();
    
    // Write to file
    return writeToFile(filename);
}

void RELWriter::beginModule(const std::string& moduleName, bool isRelocatable) {
    isRelocatable_ = isRelocatable;
    currentModule_ = truncateSymbolName(moduleName);
    
    // Write control bit (1 = relocatable, 0 = absolute)
    writer_.writeBit(isRelocatable);
    
    // Write special link item: Program Name
    writeSpecialItemHeader(SpecialLinkCode::ProgramName);
    
    // Write name length (3 bits, value = actual length 1-8)
    // For BIOSMO (6 chars): length field = 6 (binary 110)
    int len = std::min(6, (int)currentModule_.length());
    writer_.writeBits(len, 3);
    
    // Write name
    for (int i = 0; i < len; i++) {
        writer_.writeByte(currentModule_[i]);
    }
}

void RELWriter::writeProgramSize(uint16_t size) {
    writeSpecialItemHeader(SpecialLinkCode::ProgramSize);
    writer_.writeWord(size);
}

void RELWriter::writeDataSize(uint16_t size) {
    writeSpecialItemHeader(SpecialLinkCode::DataAreaSize);
    writer_.writeWord(size);
}

void RELWriter::writeCommonSize(const std::string& name, uint16_t size) {
    // Select common block
    writeSpecialItemHeader(SpecialLinkCode::SelectCommon);
    writeSymbolName(name);
    
    // Define common size
    writeSpecialItemHeader(SpecialLinkCode::CommonSize);
    writer_.writeWord(size);
}

void RELWriter::writeEntrySymbol(const std::string& name, uint16_t value, ItemType type) {
    writeSpecialItemHeader(SpecialLinkCode::EntrySymbol);
    
    // Write type (2 bits)
    writer_.writeBits(static_cast<uint8_t>(type), 2);
    
    // Write name and value
    writeSymbolName(name);
    writer_.writeWord(value);
}

void RELWriter::writeChainExternal(const std::string& name) {
    writeSpecialItemHeader(SpecialLinkCode::ChainExternal);
    
    // Write type bits (usually 00 for undefined)
    writer_.writeBits(0, 2);
    
    // Write name
    writeSymbolName(name);
}

void RELWriter::setLocation(uint16_t address, ItemType type) {
    writeSpecialItemHeader(SpecialLinkCode::SetLocation);
    
    // Write type (2 bits)
    writer_.writeBits(static_cast<uint8_t>(type), 2);
    
    // Write address
    writer_.writeWord(address);
}

void RELWriter::writeAbsoluteData(const std::vector<Byte>& data) {
    if (data.empty()) return;
    
    writeDataItemHeader(ItemType::Absolute);
    
    // Write length
    writer_.writeWord(data.size());
    
    // Write data bytes
    for (Byte b : data) {
        writer_.writeByte(b);
    }
}

void RELWriter::writeProgramData(const std::vector<Byte>& data) {
    if (data.empty()) return;
    
    writeDataItemHeader(ItemType::ProgramRel);
    
    // Write length
    writer_.writeWord(data.size());
    
    // Write data bytes
    for (Byte b : data) {
        writer_.writeByte(b);
    }
}

void RELWriter::writeDataData(const std::vector<Byte>& data) {
    if (data.empty()) return;
    
    writeDataItemHeader(ItemType::DataRel);
    
    // Write length
    writer_.writeWord(data.size());
    
    // Write data bytes
    for (Byte b : data) {
        writer_.writeByte(b);
    }
}

void RELWriter::writeChainAddress(uint16_t offset) {
    writeSpecialItemHeader(SpecialLinkCode::ChainAddress);
    writer_.writeWord(offset);
}

void RELWriter::endModule() {
    writeSpecialItemHeader(SpecialLinkCode::EndModule);
    writer_.flush();  // Ensure byte boundary
}

void RELWriter::endFile() {
    writeSpecialItemHeader(SpecialLinkCode::EndFile);
    writer_.flush();
}

bool RELWriter::writeToFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }
    
    const auto& buffer = writer_.getBuffer();
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    
    return file.good();
}

void RELWriter::writeSpecialItemHeader(SpecialLinkCode code) {
    // Type: 00 (Special)
    writer_.writeBits(static_cast<uint8_t>(ItemType::Special), 2);
    
    // Code: 4 bits
    writer_.writeBits(static_cast<uint8_t>(code), 4);
}

void RELWriter::writeDataItemHeader(ItemType type) {
    // Type: 01/10/11 (Absolute/ProgramRel/DataRel)
    writer_.writeBits(static_cast<uint8_t>(type), 2);
}

void RELWriter::writeSymbolName(const std::string& name) {
    std::string truncated = truncateSymbolName(name);
    
    // Pad to 6 characters with spaces
    for (size_t i = 0; i < 6; i++) {
        char c = (i < truncated.length()) ? truncated[i] : ' ';
        writer_.writeByte(static_cast<Byte>(c));
    }
}

std::string RELWriter::truncateSymbolName(const std::string& name) const {
    // M80 uses first 6 characters as significant
    if (name.length() <= 6) {
        return name;
    }
    return name.substr(0, 6);
}

} // namespace z80
