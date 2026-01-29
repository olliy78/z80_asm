/**
 * @file bit_writer.cpp
 * @brief Implementation of MSB-first bit writer
 */

#include "bit_writer.h"

namespace z80 {

BitWriter::BitWriter()
    : currentByte_(0)
    , bitPosition_(7)
{
}

void BitWriter::writeBit(bool bit) {
    if (bit) {
        currentByte_ |= (1 << bitPosition_);
    }
    
    bitPosition_--;
    
    if (bitPosition_ < 0) {
        flushByte();
    }
}

void BitWriter::writeBits(uint64_t value, int numBits) {
    if (numBits <= 0 || numBits > 64) {
        return;
    }
    
    // Write bits MSB-first
    for (int i = numBits - 1; i >= 0; i--) {
        bool bit = (value >> i) & 1;
        writeBit(bit);
    }
}

void BitWriter::writeByte(Byte byte) {
    writeBits(byte, 8);
}

void BitWriter::writeWord(uint16_t word) {
    // Write MSB first, then LSB
    writeByte(static_cast<Byte>(word >> 8));
    writeByte(static_cast<Byte>(word & 0xFF));
}

void BitWriter::flush() {
    if (bitPosition_ < 7) {
        // Partial byte exists - flush it (remaining bits are already 0)
        flushByte();
    }
}

void BitWriter::flushByte() {
    buffer_.push_back(currentByte_);
    currentByte_ = 0;
    bitPosition_ = 7;
}

void BitWriter::clear() {
    buffer_.clear();
    currentByte_ = 0;
    bitPosition_ = 7;
}

size_t BitWriter::getTotalBits() const {
    size_t completedBits = buffer_.size() * 8;
    int bitsInCurrentByte = (bitPosition_ < 7) ? (7 - bitPosition_) : 0;
    return completedBits + bitsInCurrentByte;
}

} // namespace z80
