#include "huffman_tree.hpp"
#include <algorithm>
#include <queue>

FrequencyTable calculate_frequencies(std::istream& input) {
    FrequencyTable frequencies{}; // Initialize all counts to 0
    char c;
    while (input.get(c)) {
        frequencies[static_cast<uint8_t>(c)]++;
    }
    return frequencies;
}

std::unique_ptr<HuffmanNode> build_huffman_tree(const FrequencyTable& frequencies) {
    // To cleanly manage std::unique_ptr move-only semantics in a min-heap structure
    // without using unsafe const-casts on std::priority_queue, we use std::vector
    // managed as a heap via std::make_heap, std::push_heap, and std::pop_heap.
    std::vector<std::unique_ptr<HuffmanNode>> heap;

    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            heap.push_back(std::make_unique<HuffmanNode>(static_cast<uint8_t>(i), frequencies[i]));
        }
    }

    // Corner case 1: Empty file (all frequencies are zero)
    if (heap.empty()) {
        return nullptr;
    }

    // Initialize the heap layout
    std::make_heap(heap.begin(), heap.end(), CompareNodes());

    // Corner case 2: Only one unique byte in the file (e.g. "aaaa" or single byte file)
    // If not handled, this would yield a tree with only 1 leaf, generating a 0-bit code.
    // We resolve this by adding a dummy/sibling node with frequency 0, forcing a valid 1-bit code.
    if (heap.size() == 1) {
        // Pop the only element
        std::pop_heap(heap.begin(), heap.end(), CompareNodes());
        auto only_node = std::move(heap.back());
        heap.pop_back();

        // Create dummy node (choose a different byte value)
        uint8_t dummy_byte = (only_node->byte == 0) ? 1 : 0;
        auto dummy_node = std::make_unique<HuffmanNode>(dummy_byte, 0);

        // Parent node combines both
        auto root = std::make_unique<HuffmanNode>(
            only_node->frequency,
            std::move(only_node),
            std::move(dummy_node)
        );
        return root;
    }

    // Main Huffman Tree construction loop: Pop two nodes, combine them, push back
    while (heap.size() > 1) {
        // Pop the first smallest
        std::pop_heap(heap.begin(), heap.end(), CompareNodes());
        auto left = std::move(heap.back());
        heap.pop_back();

        // Pop the second smallest
        std::pop_heap(heap.begin(), heap.end(), CompareNodes());
        auto right = std::move(heap.back());
        heap.pop_back();

        // Combine frequencies
        uint64_t parent_freq = left->frequency + right->frequency;
        auto parent = std::make_unique<HuffmanNode>(parent_freq, std::move(left), std::move(right));

        // Push parent back to the heap
        heap.push_back(std::move(parent));
        std::push_heap(heap.begin(), heap.end(), CompareNodes());
    }

    // The remaining node is the root of the Huffman Tree
    std::pop_heap(heap.begin(), heap.end(), CompareNodes());
    auto root = std::move(heap.back());
    heap.pop_back();

    return root;
}

// Backtracking helper to generate codes recursively without copying vectors
void generate_codes_helper(const HuffmanNode* node, BitCode& current_path, CodeTable& table) {
    if (!node) return;

    if (node->is_leaf()) {
        table[node->byte] = current_path;
        return;
    }

    // Traverse left child (representing a 0 bit)
    current_path.push_back(false);
    generate_codes_helper(node->left.get(), current_path, table);
    current_path.pop_back(); // Backtrack

    // Traverse right child (representing a 1 bit)
    current_path.push_back(true);
    generate_codes_helper(node->right.get(), current_path, table);
    current_path.pop_back(); // Backtrack
}

CodeTable generate_codes(const HuffmanNode* root) {
    CodeTable table{};
    if (!root) return table;

    BitCode current_path;
    generate_codes_helper(root, current_path, table);
    return table;
}
