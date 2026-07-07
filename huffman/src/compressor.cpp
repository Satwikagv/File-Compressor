#include "compressor.hpp"
#include "huffman_tree.hpp"
#include "bit_writer.hpp"
#include "bit_reader.hpp"
#include <fstream>
#include <iostream>

bool compress_stream(std::istream& input, std::ostream& output) {
    // 1. Calculate frequency distribution
    FrequencyTable frequencies = calculate_frequencies(input);

    uint64_t original_size = 0;
    uint16_t unique_count = 0;
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            unique_count++;
            original_size += frequencies[i];
        }
    }

    // 2. Build tree and generate lookup table of variable-length codes
    auto root = build_huffman_tree(frequencies);
    auto codes = generate_codes(root.get());

    // 3. Write header fields
    // Magic bytes (2 bytes)
    output.put('H');
    output.put('F');

    // Unique count (2 bytes, big-endian serialization)
    output.put(static_cast<char>((unique_count >> 8) & 0xFF));
    output.put(static_cast<char>(unique_count & 0xFF));

    // Frequency table entries (active bytes and their 64-bit frequencies)
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            output.put(static_cast<char>(i));
            uint64_t freq = frequencies[i];
            for (int j = 7; j >= 0; --j) {
                output.put(static_cast<char>((freq >> (j * 8)) & 0xFF));
            }
        }
    }

    // Original file size (8 bytes, big-endian serialization)
    // This allows the decompressor to stop decoding exactly at the end of content,
    // ignoring padding bits in the final byte.
    for (int j = 7; j >= 0; --j) {
        output.put(static_cast<char>((original_size >> (j * 8)) & 0xFF));
    }

    // 4. Reset stream to encode content in the second pass
    input.clear();
    input.seekg(0, std::ios::beg);

    // 5. Bitwise write data
    BitWriter writer(output);
    char c;
    while (input.get(c)) {
        writer.write_bits(codes[static_cast<uint8_t>(c)]);
    }

    writer.flush();
    return true;
}

bool decompress_stream(std::istream& input, std::ostream& output) {
    // 1. Read and verify magic bytes
    int m1 = input.get();
    int m2 = input.get();
    if (m1 != 'H' || m2 != 'F') {
        std::cerr << "Error: Invalid file format (missing HF header magic bytes).\n";
        return false;
    }

    // 2. Read unique character count (2 bytes)
    int h = input.get();
    int l = input.get();
    if (h == EOF || l == EOF) {
        std::cerr << "Error: Truncated header unique count.\n";
        return false;
    }
    uint16_t unique_count = (static_cast<uint16_t>(h) << 8) | l;

    // 3. Read frequency table
    FrequencyTable frequencies{};
    for (uint16_t i = 0; i < unique_count; ++i) {
        int byte_val = input.get();
        if (byte_val == EOF) {
            std::cerr << "Error: Truncated header frequency table entry.\n";
            return false;
        }

        uint64_t freq = 0;
        for (int j = 0; j < 8; ++j) {
            int b = input.get();
            if (b == EOF) {
                std::cerr << "Error: Truncated frequency value.\n";
                return false;
            }
            freq = (freq << 8) | static_cast<uint8_t>(b);
        }
        frequencies[static_cast<uint8_t>(byte_val)] = freq;
    }

    // 4. Read original size (8 bytes)
    uint64_t original_size = 0;
    for (int j = 0; j < 8; ++j) {
        int b = input.get();
        if (b == EOF) {
            std::cerr << "Error: Truncated original size field.\n";
            return false;
        }
        original_size = (original_size << 8) | static_cast<uint8_t>(b);
    }

    // Empty file edge case
    if (original_size == 0) {
        return true;
    }

    // 5. Rebuild Huffman Tree
    auto root = build_huffman_tree(frequencies);
    if (!root) {
        std::cerr << "Error: Failed to rebuild Huffman tree.\n";
        return false;
    }

    // 6. Decode bitstream using the Huffman Tree
    BitReader reader(input);
    uint64_t bytes_decoded = 0;

    while (bytes_decoded < original_size) {
        const HuffmanNode* current = root.get();
        while (!current->is_leaf()) {
            bool bit;
            if (!reader.read_bit(bit)) {
                std::cerr << "Error: Unexpected EOF in compressed bitstream.\n";
                return false;
            }
            if (bit) {
                current = current->right.get();
            } else {
                current = current->left.get();
            }
        }
        output.put(static_cast<char>(current->byte));
        bytes_decoded++;
    }

    return true;
}

bool compress_file(const std::string& input_path, const std::string& output_path) {
    std::ifstream input(input_path, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Error: Failed to open input file: " << input_path << "\n";
        return false;
    }

    std::ofstream output(output_path, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error: Failed to open output file: " << output_path << "\n";
        return false;
    }

    return compress_stream(input, output);
}

bool decompress_file(const std::string& input_path, const std::string& output_path) {
    std::ifstream input(input_path, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Error: Failed to open compressed file: " << input_path << "\n";
        return false;
    }

    std::ofstream output(output_path, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error: Failed to open restoration file: " << output_path << "\n";
        return false;
    }

    return decompress_stream(input, output);
}
