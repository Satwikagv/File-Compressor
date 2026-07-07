#pragma once

#include <string>
#include <istream>
#include <ostream>

bool compress_stream(std::istream& input, std::ostream& output);

bool decompress_stream(std::istream& input, std::ostream& output);

bool compress_file(const std::string& input_path, const std::string& output_path);
bool decompress_file(const std::string& input_path, const std::string& output_path);
