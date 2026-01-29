/**
 * @file bit_writer.h
 * @brief MSB-first bit writer for .REL file format
 * 
 * The .REL format uses a bitstream encoding where bits are packed
 * MSB-first into bytes. This means the first bit written goes to
 * bit position 7, the second to bit 6, etc.
 * 
 * Example:
 *   Writing bits: 1, 0, 0, 0, 0, 1, 0, 1
 *   Results in byte: 0x85 (10000101 binary)
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include "common/types.h"
#include <vector>
#include <cstdint>

namespace z80 {

/**
 * @class BitWriter
 * @brief Writes bits MSB-first to a byte stream
 * 
 * Implements the MSB-first bit packing required by the .REL format.
 * Bits are accumulated in a current byte, starting from bit 7 (MSB)
 * and working down to bit 0 (LSB). When a byte is complete, it is
 * written to the output buffer.
 */
class BitWriter {
public:
    /**
     * @brief Construct a new Bit Writer
     */
    BitWriter();
    
    /**
     * @brief Write a single bit
     * @param bit Bit value (true = 1, false = 0)
     */
    void writeBit(bool bit);
    
    /**
     * @brief Write multiple bits from a value
     * @param value Value to write bits from
     * @param numBits Number of bits to write (1-64)
     * 
     * Bits are written MSB-first from the value.
     * Example: writeBits(0x5, 3) writes bits 1, 0, 1
     */
    void writeBits(uint64_t value, int numBits);
    
    /**
     * @brief Write a complete byte as 8 bits
     * @param byte Byte value to write
     */
    void writeByte(Byte byte);
    
    /**
     * @brief Write a 16-bit word MSB-first
     * @param word Word value to write
     */
    void writeWord(uint16_t word);
    
    /**
     * @brief Flush any partial byte to output (pad with zeros)
     * 
     * This should be called at the end of a module to ensure
     * all bits are written to the output buffer.
     */
    void flush();
    
    /**
     * @brief Get the current bit position (0-7)
     * @return Current bit position within the current byte
     * 
     * Position 7 = MSB (next bit to write)
     * Position 0 = LSB
     * Position -1 would trigger a flush (but is handled internally)
     */
    int getBitPosition() const { return bitPosition_; }
    
    /**
     * @brief Check if currently on a byte boundary
     * @return true if the next bit will start a new byte
     */
    bool isAligned() const { return bitPosition_ == 7; }
    
    /**
     * @brief Get the output buffer
     * @return const reference to the byte buffer
     */
    const std::vector<Byte>& getBuffer() const { return buffer_; }
    
    /**
     * @brief Clear the buffer and reset state
     */
    void clear();
    
    /**
     * @brief Get total number of bits written (including partial byte)
     * @return Total bit count
     */
    size_t getTotalBits() const;

private:
    std::vector<Byte> buffer_;      ///< Output byte buffer
    Byte currentByte_;              ///< Current byte being accumulated
    int bitPosition_;               ///< Current bit position (7=MSB, 0=LSB)
    
    /**
     * @brief Flush the current byte to the buffer
     */
    void flushByte();
};

} // namespace z80
