/**
 * @file output_writer.h
 * @brief Base class for output file writers
 */

#ifndef Z80_OUTPUT_WRITER_H
#define Z80_OUTPUT_WRITER_H

#include "assembler.h"
#include <string>

namespace z80 {

/**
 * @brief Abstract base class for output writers
 */
class OutputWriter {
public:
    virtual ~OutputWriter() = default;
    
    /**
     * @brief Write assembled module to file
     * @param module Assembled module to write
     * @param filename Output filename
     * @return true if successful
     */
    virtual bool write(const AssembledModule& module, const std::string& filename) = 0;
};

/**
 * @brief REL format writer
 */
class RELOutputWriter : public OutputWriter {
public:
    bool write(const AssembledModule& module, const std::string& filename) override;
};

/**
 * @brief Listing format writer (.PRN)
 */
class ListingOutputWriter : public OutputWriter {
public:
    bool write(const AssembledModule& module, const std::string& filename) override;
};

/**
 * @brief Intel HEX format writer
 */
class HEXOutputWriter : public OutputWriter {
public:
    bool write(const AssembledModule& module, const std::string& filename) override;
};

} // namespace z80

#endif // Z80_OUTPUT_WRITER_H
