/**
 * @file output_writer.cpp
 * @brief Output file writers implementation
 */

#include "output_writer.h"
#include "rel_writer.h"
#include <fstream>

namespace z80 {

// REL Output Writer
bool RELOutputWriter::write(const AssembledModule& module, const std::string& filename) {
    RELWriter writer;
    
    // Begin module
    writer.beginModule(module.moduleName, module.isRelocatable);
    
    // Write segment sizes
    if (module.csegSize > 0) {
        writer.writeProgramSize(module.csegSize);
    }
    if (module.dsegSize > 0) {
        writer.writeDataSize(module.dsegSize);
    }
    if (module.commonSize > 0) {
        writer.writeCommonSize("COMMON", module.commonSize);
    }
    
    // TODO: Write PUBLIC symbols
    // for (auto& sym : module.symbolTable.getPublicSymbols()) {
    //     writer.writeEntrySymbol(sym.name, sym.value, ...);
    // }
    
    // TODO: Write EXTRN symbols
    // for (auto& sym : module.symbolTable.getExternalSymbols()) {
    //     writer.writeChainExternal(sym.name);
    // }
    
    // Write code/data by segment
    // Group consecutive bytes by segment
    std::vector<Byte> currentData;
    SegmentType currentSegType = SegmentType::CSEG;
    Address currentAddr = 0;
    bool hasData = false;
    
    for (const auto& line : module.lines) {
        if (line.code.empty()) continue;
        
        // If segment changed or address is not continuous, flush current data
        if (hasData && (line.segment != currentSegType || line.address != currentAddr)) {
            // Write accumulated data
            if (currentSegType == SegmentType::ASEG) {
                writer.writeAbsoluteData(currentData);
            } else if (currentSegType == SegmentType::CSEG) {
                writer.writeProgramData(currentData);
            } else if (currentSegType == SegmentType::DSEG) {
                writer.writeDataData(currentData);
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
            writer.setLocation(line.address, locType);
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
            writer.writeAbsoluteData(currentData);
        } else if (currentSegType == SegmentType::CSEG) {
            writer.writeProgramData(currentData);
        } else if (currentSegType == SegmentType::DSEG) {
            writer.writeDataData(currentData);
        }
    }
    
    // End module and file
    writer.endModule();
    writer.endFile();
    
    // Write to file
    return writer.writeToFile(filename);
}

// Listing Output Writer
bool ListingOutputWriter::write(const AssembledModule& module, const std::string& filename) {
    // TODO: Implement .PRN listing format
    std::ofstream file(filename);
    if (!file) return false;
    
    file << "; Assembly listing for " << module.moduleName << "\n";
    file << "; (Listing format not yet implemented)\n";
    
    return true;
}

// HEX Output Writer
bool HEXOutputWriter::write(const AssembledModule& module, const std::string& filename) {
    // TODO: Implement Intel HEX format
    std::ofstream file(filename);
    if (!file) return false;
    
    file << "; Intel HEX output for " << module.moduleName << "\n";
    file << "; (HEX format not yet implemented)\n";
    
    return true;
}

} // namespace z80
