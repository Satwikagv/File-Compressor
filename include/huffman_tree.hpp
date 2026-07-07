#pragma once

#include "huffman_node.hpp"
#include <array>
#include <vector>
#include <istream>

using BitCode = std::vector<bool>;

using FrequencyTable = std::array<uint64_t, 256>;

using CodeTable = std::array<BitCode, 256>;

FrequencyTable calculate_frequencies(std::istream& input);

std::unique_ptr<HuffmanNode> build_huffman_tree(const FrequencyTable& frequencies);

CodeTable generate_codes(const HuffmanNode* root);
