#pragma once

#include <istream>
#include <cstdint>

class BitReader {
public:
    explicit BitReader(std::istream& in) : in_(in), buffer_(0), bit_index_(8) {}

    BitReader(const BitReader&) = delete;
    BitReader& operator=(const BitReader&) = delete;

    bool read_bit(bool& bit) {
        if (bit_index_ == 8) {
            char c;
            if (in_.get(c)) {
                buffer_ = static_cast<uint8_t>(c);
                bit_index_ = 0;
            } else {
                return false;
            }
        }

        bit = (buffer_ >> (7 - bit_index_)) & 1;
        bit_index_++;
        return true;
    }

    bool read_byte(uint8_t& byte) {
        if (bit_index_ == 8 || bit_index_ == 0) {
            char c;
            if (in_.get(c)) {
                byte = static_cast<uint8_t>(c);
                bit_index_ = 8;
                return true;
            }
            return false;
        } else {
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
