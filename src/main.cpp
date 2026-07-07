#include "compressor.hpp"
#include <iostream>
#include <string>
#include <chrono>

void print_usage(const std::string& prog_name, bool decompress_mode) {
    if (decompress_mode) {
        std::cerr << "Usage: " << prog_name << " <compressed_file> <restored_file>\n";
    } else {
        std::cerr << "Usage: " << prog_name << " <input_file> <compressed_output_file>\n";
    }
}

int main(int argc, char* argv[]) {
    std::string prog_path = argv[0];
    size_t last_slash = prog_path.find_last_of("/\\");
    std::string prog_name = (last_slash == std::string::npos) ? prog_path : prog_path.substr(last_slash + 1);

    bool decompress_mode = (prog_name.find("decompress") != std::string::npos);

    if (argc != 3) {
        print_usage(argv[0], decompress_mode);
        return 1;
    }

    std::string src = argv[1];
    std::string dest = argv[2];

    auto start_time = std::chrono::high_resolution_clock::now();

    bool success;
    if (decompress_mode) {
        std::cout << "Decompressing '" << src << "' -> '" << dest << "'\n";
        success = decompress_file(src, dest);
    } else {
        std::cout << "Compressing '" << src << "' -> '" << dest << "'\n";
        success = compress_file(src, dest);
    }

    if (!success) {
        std::cerr << "Error: Operation failed!\n";
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end_time - start_time;

    std::cout << "Success! Execution Time: " << duration.count() << " ms\n";
    return 0;
}
