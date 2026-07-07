import os
import subprocess
import time
import hashlib
import random

# File paths
COMPRESS_BIN = "../compress"
DECOMPRESS_BIN = "../decompress"
TEST_DIR = "test_files"

def calculate_md5(filepath):
    hasher = hashlib.md5()
    with open(filepath, 'rb') as f:
        buf = f.read()
        hasher.update(buf)
    return hasher.hexdigest()

def create_test_files():
    os.makedirs(TEST_DIR, exist_ok=True)
    
    # 1. Repetitive text file (1 MB of 'A')
    with open(os.path.join(TEST_DIR, "repetitive.txt"), "w") as f:
        f.write("A" * (1024 * 1024))
        
    # 2. English-like text file (approx 1 MB, by repeating sample text)
    sample_text = (
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
        "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
        "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n"
    )
    with open(os.path.join(TEST_DIR, "lorem_text.txt"), "w") as f:
        f.write(sample_text * 8000) # Roughly 3.5 MB
        
    # 3. Maximum entropy random binary file (100 KB - uncompressible)
    with open(os.path.join(TEST_DIR, "random.bin"), "wb") as f:
        f.write(os.urandom(100 * 1024))
        
    # 4. Edge Case: Empty file (0 bytes)
    with open(os.path.join(TEST_DIR, "empty.txt"), "w") as f:
        pass
        
    # 5. Edge Case: Single unique character (100 bytes of 'X')
    with open(os.path.join(TEST_DIR, "single_char.txt"), "w") as f:
        f.write("X" * 100)
        
    # 6. Edge Case: Tiny file (5 bytes)
    with open(os.path.join(TEST_DIR, "tiny.txt"), "w") as f:
        f.write("hello")

def run_command(args):
    start = time.perf_counter()
    result = subprocess.run(args, capture_output=True, text=True)
    end = time.perf_counter()
    duration_ms = (end - start) * 1000.0
    return duration_ms, result.returncode == 0

def run_benchmarks():
    files = [
        ("repetitive.txt", "Repetitive Text"),
        ("lorem_text.txt", "Lorem Text"),
        ("random.bin", "Random Binary"),
        ("empty.txt", "Empty File"),
        ("single_char.txt", "Single-Byte File"),
        ("tiny.txt", "Tiny File")
    ]
    
    print("\n# Compression Benchmark Results\n")
    print("| File Type | Original Size | Huffman Size | Huffman Ratio | Huffman Time | Gzip Size | Gzip Ratio | Gzip Time | Verified? |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    
    for filename, desc in files:
        orig_path = os.path.join(TEST_DIR, filename)
        huff_path = os.path.join(TEST_DIR, filename + ".hf")
        restored_path = os.path.join(TEST_DIR, filename + ".restored")
        gzip_path = os.path.join(TEST_DIR, filename + ".gz")
        
        orig_size = os.path.getsize(orig_path)
        
        # 1. Custom Huffman Compression
        comp_time, comp_success = run_command([COMPRESS_BIN, orig_path, huff_path])
        huff_size = os.path.getsize(huff_path) if comp_success and os.path.exists(huff_path) else 0
        huff_ratio = (huff_size / orig_size * 100.0) if orig_size > 0 else 100.0
        
        # 2. Custom Huffman Decompression
        decomp_time, decomp_success = run_command([DECOMPRESS_BIN, huff_path, restored_path])
        
        # Verify integrity
        if decomp_success and os.path.exists(restored_path):
            verified = "Yes" if calculate_md5(orig_path) == calculate_md5(restored_path) else "CORRUPTED"
        else:
            verified = "Failed"
            
        # 3. Gzip Compression (Level 6 default)
        # We copy file to compress with gzip to keep original intact
        subprocess.run(["cp", orig_path, gzip_path + "_tmp"])
        gzip_time, gzip_success = run_command(["gzip", "-f", gzip_path + "_tmp"])
        gzip_real_path = gzip_path + "_tmp.gz"
        gzip_size = os.path.getsize(gzip_real_path) if gzip_success and os.path.exists(gzip_real_path) else 0
        gzip_ratio = (gzip_size / orig_size * 100.0) if orig_size > 0 else 100.0
        
        # Cleanup gzip tmp files
        if os.path.exists(gzip_real_path):
            os.remove(gzip_real_path)
        
        # Formatting values
        huff_ratio_str = f"{huff_ratio:.2f}%" if orig_size > 0 else "-"
        gzip_ratio_str = f"{gzip_ratio:.2f}%" if orig_size > 0 else "-"
        comp_time_str = f"{comp_time:.1f} ms"
        gzip_time_str = f"{gzip_time:.1f} ms"
        
        print(f"| {desc} | {orig_size:,} B | {huff_size:,} B | {huff_ratio_str} | {comp_time_str} | {gzip_size:,} B | {gzip_ratio_str} | {gzip_time_str} | {verified} |")
        
        # Cleanup huffman temp files
        if os.path.exists(huff_path):
            os.remove(huff_path)
        if os.path.exists(restored_path):
            os.remove(restored_path)

if __name__ == "__main__":
    # Ensure binary files exist
    if not os.path.exists(COMPRESS_BIN) or not os.path.exists(DECOMPRESS_BIN):
        print(f"Error: Executables '{COMPRESS_BIN}' and '{DECOMPRESS_BIN}' not found. Run 'make' first.")
        exit(1)
        
    create_test_files()
    run_benchmarks()
    
    # Cleanup test directory
    for f in os.listdir(TEST_DIR):
        os.remove(os.path.join(TEST_DIR, f))
    os.rmdir(TEST_DIR)
