/**
 * @file assembler.cpp
 * @brief Z80 Assembler implementation
 */

#include "assembler.h"
#include <algorithm>

namespace z80 {

Assembler::Assembler() {
}

std::unique_ptr<AssembledModule> Assembler::assemble(const std::string& filename) {
    errors_.clear();
    
    // Parse the source file
    if (!parser_.assemble(filename)) {
        errors_ = parser_.getErrors();
        return nullptr;
    }
    
    if (parser_.hasErrors()) {
        errors_ = parser_.getErrors();
    }
    
    // Create assembled module
    auto module = std::make_unique<AssembledModule>();
    module->moduleName = deriveModuleName(filename);
    module->lines = parser_.getLines();
    module->symbolTable = parser_.getSymbolTable();
    module->isRelocatable = isModuleRelocatable(module->lines);
    
    // Calculate segment sizes
    calculateSegmentSizes(*module);
    
    return module;
}

void Assembler::calculateSegmentSizes(AssembledModule& module) {
    module.csegSize = 0;
    module.dsegSize = 0;
    module.commonSize = 0;
    
    for (const auto& line : module.lines) {
        if (line.code.empty()) continue;
        
        Word endAddress = line.address + line.code.size();
        
        switch (line.segment) {
            case SegmentType::CSEG:
                if (endAddress > module.csegSize) {
                    module.csegSize = endAddress;
                }
                break;
            case SegmentType::DSEG:
                if (endAddress > module.dsegSize) {
                    module.dsegSize = endAddress;
                }
                break;
            case SegmentType::COMMON:
                if (endAddress > module.commonSize) {
                    module.commonSize = endAddress;
                }
                break;
            default:
                break;
        }
    }
}

bool Assembler::isModuleRelocatable(const std::vector<ParsedLine>& lines) {
    for (const auto& line : lines) {
        if (!line.code.empty()) {
            if (line.segment == SegmentType::CSEG || 
                line.segment == SegmentType::DSEG ||
                line.segment == SegmentType::COMMON) {
                return true;
            }
        }
    }
    return false;
}

std::string Assembler::deriveModuleName(const std::string& filename) {
    // Extract basename without extension
    size_t lastSlash = filename.find_last_of("/\\");
    size_t lastDot = filename.find_last_of('.');
    size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
    size_t end = (lastDot == std::string::npos) ? filename.length() : lastDot;
    return filename.substr(start, end - start);
}

} // namespace z80
