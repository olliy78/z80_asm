/**
 * @file rel_writer.cpp
 * @brief Implementation of .REL format writer
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include "rel_writer.h"

namespace z80 {

RelWriter::RelWriter(const std::string& filename) : filename_(filename) {
    // TODO: Open file
}

void RelWriter::write() {
    // TODO: Write .REL format
}

void RelWriter::close() {
    // TODO: Close file
}

} // namespace z80
