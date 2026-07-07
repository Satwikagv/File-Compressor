#pragma once

#include <istream>
#include <cstdint>

class BitReader {
public:
    explicit BitReader(std::istream& in) : in_(in), buffer_(0), bit_index_(8) {}

    // Prevent copying
    BitReader(const BitReader&) = delete;
    BitReader& operator=(const BitReader&) = delete;

    // Reads a single bit. Returns false if EOF is reached.
    bool read_bit(bool& bit) {
        if (bit_index_ == 8) {
            char c;
            if (in_.get(c)) {
                buffer_ = static_cast<uint8_t>(c);
                bit_index_ = 0;
            } else {
                return false; // EOF reached
            }
        }

        bit = (buffer_ >> (7 - bit_index_)) & 1;
        bit_index_++;
        return true;
    }

    // Reads a raw 8-bit byte. Returns false if EOF is reached before reading 8 bits.
    bool read_byte(uint8_t& byte) {
        if (bit_index_ == 8 || bit_index_ == 0) {
            char c;
            if (in_.get(c)) {
                byte = static_cast<uint8_t>(c);
                bit_index_ = 8; // Reset index to indicate block-level alignment
                return true;
            }
            return false;
        } else {
            // Read bit-by-bit if not currently byte-aligned
            uint16_t temp = 0;
            for (int i = 0; i < 8; ++i) {
                bool bit;
                if (!read_bit(bit)) {
                    return false;
                }
                temp = (temp << 1) | (bit ? 1 : 0);
            }
            byte = static_cast<uint8_t>(temp);
            return true;
        }
    }

private:
    std::istream& in_;
    uint8_t buffer_;
    uint8_t bit_index_;
};
