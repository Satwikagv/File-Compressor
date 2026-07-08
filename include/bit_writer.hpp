#pragma once

#include <ostream>
#include <vector>
#include <cstdint>

class BitWriter {
public:
    explicit BitWriter(std::ostream& out) : out_(out), buffer_(0), bit_count_(0) {}

    ~BitWriter() {
        flush();
    }

    BitWriter(const BitWriter&) = delete;
    BitWriter& operator=(const BitWriter&) = delete;

    void write_bit(bool bit) {
        buffer_ = (buffer_ << 1) | (bit ? 1 : 0);
        bit_count_++;

        if (bit_count_ == 8) {
            out_.put(static_cast<char>(buffer_));
            buffer_ = 0;
            bit_count_ = 0;
        }
    }

    void write_bits(const std::vector<bool>& bits) {
        for (bool bit : bits) {
            write_bit(bit);
        }
    }

    void write_byte(uint8_t byte) {
        if (bit_count_ == 0) {
            out_.put(static_cast<char>(byte));
        } else {
            for (int i = 7; i >= 0; --i) {
                write_bit((byte >> i) & 1);
            }
        }
    }
    
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
