#pragma once

#include <memory>
#include <cstdint>

// Represents a node in the Huffman Tree.
// Uses std::unique_ptr for child nodes to guarantee automatic lifetime management and prevent leaks.
struct HuffmanNode {
    uint8_t byte;
    uint64_t frequency;
    std::unique_ptr<HuffmanNode> left;
    std::unique_ptr<HuffmanNode> right;

    // Leaf node constructor
    HuffmanNode(uint8_t b, uint64_t f)
        : byte(b), frequency(f), left(nullptr), right(nullptr) {}

    // Internal node constructor
    HuffmanNode(uint64_t f, std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r)
        : byte(0), frequency(f), left(std::move(l)), right(std::move(r)) {}

    bool is_leaf() const {
        return left == nullptr && right == nullptr;
    }
};

// Custom comparator for min-heap implementation using std::vector heap functions.
struct CompareNodes {
    bool operator()(const std::unique_ptr<HuffmanNode>& lhs, const std::unique_ptr<HuffmanNode>& rhs) const {
        return lhs->frequency > rhs->frequency; // Smallest frequency bubbles to the front/top
    }
};
