#pragma once

#include <memory>
#include <cstdint>

struct HuffmanNode {
    uint8_t byte;
    uint64_t frequency;
    std::unique_ptr<HuffmanNode> left;
    std::unique_ptr<HuffmanNode> right; 

    HuffmanNode(uint8_t b, uint64_t f)
        : byte(b), frequency(f), left(nullptr), right(nullptr) {}

    HuffmanNode(uint64_t f, std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r)
        : byte(0), frequency(f), left(std::move(l)), right(std::move(r)) {}

    bool is_leaf() const {
        return left == nullptr && right == nullptr;
    }
};

struct CompareNodes {
    bool operator()(const std::unique_ptr<HuffmanNode>& lhs, const std::unique_ptr<HuffmanNode>& rhs) const {
        return lhs->frequency > rhs->frequency;
    }
};
