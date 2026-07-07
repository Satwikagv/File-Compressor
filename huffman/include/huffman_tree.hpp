#pragma once

#include "huffman_node.hpp"
#include <array>
#include <vector>
#include <istream>

// BitCode represents the variable-length path (a sequence of 0s and 1s)
using BitCode = std::vector<bool>;

// Frequency table for all 256 possible byte values
using FrequencyTable = std::array<uint64_t, 256>;

// Code table mapping each byte value to its Huffman BitCode
using CodeTable = std::array<BitCode, 256>;

// Scans an input stream byte-by-byte and records frequencies
FrequencyTable calculate_frequencies(std::istream& input);

// Builds the Huffman tree from a frequency table.
// Handles edge cases: empty files (returns nullptr) and single-unique-byte files.
std::unique_ptr<HuffmanNode> build_huffman_tree(const FrequencyTable& frequencies);

// Generates the CodeTable by traversing the Huffman tree
CodeTable generate_codes(const HuffmanNode* root);
