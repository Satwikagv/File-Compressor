#include "huffman_tree.hpp"
#include <algorithm>
#include <queue>

FrequencyTable calculate_frequencies(std::istream& input) {
    FrequencyTable frequencies{}; 
    char c;
    while (input.get(c)) {
        frequencies[static_cast<uint8_t>(c)]++;
    }
    return frequencies;
}

std::unique_ptr<HuffmanNode> build_huffman_tree(const FrequencyTable& frequencies) {
    std::vector<std::unique_ptr<HuffmanNode>> heap;

    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            heap.push_back(std::make_unique<HuffmanNode>(static_cast<uint8_t>(i), frequencies[i]));
        }
    }

    if (heap.empty()) {
        return nullptr;
    }

    std::make_heap(heap.begin(), heap.end(), CompareNodes());

    if (heap.size() == 1) {
        std::pop_heap(heap.begin(), heap.end(), CompareNodes());
        auto only_node = std::move(heap.back());
        heap.pop_back();

        uint8_t dummy_byte = (only_node->byte == 0) ? 1 : 0;
        auto dummy_node = std::make_unique<HuffmanNode>(dummy_byte, 0);

        auto root = std::make_unique<HuffmanNode>(
            only_node->frequency,
            std::move(only_node),
            std::move(dummy_node)
        );
        return root;
    }

    while (heap.size() > 1) {
        std::pop_heap(heap.begin(), heap.end(), CompareNodes());
        auto left = std::move(heap.back());
        heap.pop_back();

        std::pop_heap(heap.begin(), heap.end(), CompareNodes());
        auto right = std::move(heap.back());
        heap.pop_back();

        uint64_t parent_freq = left->frequency + right->frequency;
        auto parent = std::make_unique<HuffmanNode>(parent_freq, std::move(left), std::move(right));

        heap.push_back(std::move(parent));
        std::push_heap(heap.begin(), heap.end(), CompareNodes());
    }

    std::pop_heap(heap.begin(), heap.end(), CompareNodes());
    auto root = std::move(heap.back());
    heap.pop_back();

    return root;
}

void generate_codes_helper(const HuffmanNode* node, BitCode& current_path, CodeTable& table) {
    if (!node) return;

    if (node->is_leaf()) {
        table[node->byte] = current_path;
        return;
    }

    current_path.push_back(false);
    generate_codes_helper(node->left.get(), current_path, table);
    current_path.pop_back();

    current_path.push_back(true);
    generate_codes_helper(node->right.get(), current_path, table);
    current_path.pop_back(); 
}

CodeTable generate_codes(const HuffmanNode* root) {
    CodeTable table{}; 
    if (!root) return table;

    BitCode current_path;
    generate_codes_helper(root, current_path, table);
    return table;
}
