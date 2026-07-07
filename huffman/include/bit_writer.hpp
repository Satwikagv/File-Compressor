#pragma once

#include <ostream>
#include <vector>
#include <cstdint>

class BitWriter {
public:
    explicit BitWriter(std::ostream& out) : out_(out), buffer_(0), bit_count_(0) {}

    // Flush any remaining bits in the buffer on destruction
    ~BitWriter() {
        flush();
    }

    // Prevent copying
    BitWriter(const BitWriter&) = delete;
    BitWriter& operator=(const BitWriter&) = delete;

    // Writes a single bit (true for 1, false for 0)
    void write_bit(bool bit) {
        buffer_ = (buffer_ << 1) | (bit ? 1 : 0);
        bit_count_++;

        if (bit_count_ == 8) {
            out_.put(static_cast<char>(buffer_));
            buffer_ = 0;
            bit_count_ = 0;
        }
    }

    // Writes a sequence of bits
    void write_bits(const std::vector<bool>& bits) {
        for (bool bit : bits) {
            write_bit(bit);
        }
    }

    // Writes a raw 8-bit byte directly (bypassing bit alignment)
    // To maintain correctness, this should only be called when the bit buffer is empty (flushed)
    void write_byte(uint8_t byte) {
        if (bit_count_ == 0) {
            out_.put(static_cast<char>(byte));
        } else {
            // Write bit-by-bit if not currently byte-aligned
            for (int i = 7; i >= 0; --i) {
                write_bit((byte >> i) & 1);
            }
        }
    }

    // Pads any unfinished byte with 0s and writes it out
    void flush() {
        if (bit_count_ > 0) {
            buffer_ <<= (8 - bit_count_);
            out_.put(static_cast<char>(buffer_));
            buffer_ = 0;
            bit_count_ = 0;
        }
        out_.flush();
    }

private:
    std::ostream& out_;
    uint8_t buffer_;
    uint8_t bit_count_;
};
