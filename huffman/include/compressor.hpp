#pragma once

#include <string>
#include <istream>
#include <ostream>

// Compresses the input stream into the output stream using Huffman Coding
bool compress_stream(std::istream& input, std::ostream& output);

// Decompresses the input stream into the output stream using Huffman Coding
bool decompress_stream(std::istream& input, std::ostream& output);

// Helper functions that wrap file operations
bool compress_file(const std::string& input_path, const std::string& output_path);
bool decompress_file(const std::string& input_path, const std::string& output_path);
