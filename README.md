# File-Compressor
A Command line compressor and decompressor built in C++ using Huffman Coding. 
- For Compressing a file - it reads the file, build the optimal Huffman binary tree based on the byte frequencies, assigns a unique binary code to the byte values and returns a packed bitstream output.
- For Decompressing a file - it reads the file, build the same binary tree by the frequency table and original size, unpacks the bitstream using the tree and returns the bitstream of byte values.

## Functionality

- The Compression and Decompression not only works for text file but for all binary files like .jpg,.png, etc.
- Packs and unpacks a byte bit by bit using BitWriter/BitReader.
- Decompression doesn't need any external metadata because the compressor output include a header which has 2 magic bytes,frequency table and original size at the start of the file.
- The compression handles the edge cases such as empty file, single unique byte file.
- Lossless on a round trip of compression and decompression can be verified using `diff`  

## Command line usage
1. Clone the repository
2. Build the project:
```bash
make
```
3. Run the compressor on any file, using a relative path or absolute path.
4. Compress and decompress a file:
```bash
./compress <input file> <output.huff>
./decompress <output.huff> <restored file>
```
5. To verify whether there is any difference in between **input file** and **restored file** use `diff`
```bash
diff <input file> <restored file>
```
 `diff` gives no output if the input file and restored file are identical, confirming that the restoration is lossless.  

## Project Architecture

```
                 main.cpp
                     │
     Reads command-line arguments
                     │
        ┌────────────┴────────────┐
        │                         │
   Compression               Decompression
        │                         │
        └────────────┬────────────┘
                     │
              compressor.cpp
                     │
      ┌──────────────┼──────────────┐
      │              │              │
 Huffman Tree    BitWriter     BitReader
      │
 Huffman Node
```

### Module Responsibilities

#### `main.cpp`
- Parses command-line arguments.
- Determines whether to compress or decompress based on the executable name.
- Invokes the appropriate function.

#### `compressor.cpp`
This is the actual algorithm sequence.
Implements the overall compression and decompression workflow.

**Compression**
1. Read the input file.
2. Build the frequency table.
3. Build the Huffman tree.
4. Generate Huffman codes.
5. Write metadata (header).
6. Read the file again and encode the file using `BitWriter`.

>Note : the input file is read twice - once to build the frequency table and once to actually encode each byte using resulting codes.

**Decompression**
1. Read the file header and verify that it matches the expected Huffman file signature (**HF**)
2. Reconstruct the Huffman tree.
3. Read encoded bits using `BitReader`.
4. Decode the original data.
5. Write the restored file.

#### `huffman_tree.cpp`
- Counts byte frequencies from the input stream.
- Builds the Huffman tree using a min-heap: repeatedly pops the two
  lowest-frequency nodes, merges them into a new parent, and reinserts
  the parent until one root remains.
- The heap is implemented manually over `std::vector` (`make_heap`/
  `push_heap`/`pop_heap`) rather than `std::priority_queue`, since
  `priority_queue::top()` only returns a `const` reference, and
  `std::unique_ptr` can only be removed by moving — which needs
  non-const access.
- Generates Huffman codes by walking the tree with DFS + backtracking.


#### `huffman_node.hpp`
Defines the node structure used to construct the Huffman tree.

- Stores the frequency, character (for leaf nodes), and pointers to left and right child nodes.
- Supports the creation of both **leaf nodes** and **internal connector nodes**.
- Provides an `is_leaf()` helper function to check whether a node represents a character or an internal node during tree traversal.

#### `bit_writer.hpp`
- Packs variable-length Huffman codes into bytes.
- Buffers bits before writing them to the output file.

#### `bit_reader.hpp`
- Reads one bit at a time from the compressed file.
- Supplies bits to the decoder until the original file is reconstructed.
 
 ### Compressed file Format

| Field | Size | Description |
|---|---|---|
| Magic bytes | 2 bytes | `'H'` `'F'` — identifies a valid compressed file |
| Unique count | 2 bytes | Number of distinct byte values in the file |
| Frequency table | 9 bytes × unique count | `(byte, 8-byte frequency)` pairs |
| Original size | 8 bytes | Decoded byte count — tells the decoder when to stop |
| Payload | remainder of file | Huffman-encoded bitstream |

## Benchmarks

Huffman coding alone can't match gzip on repetitive data — gzip's DEFLATE
algorithm layers LZ77 (repetition matching) on top of Huffman coding, while
this implementation only does the half. The gap is largest
on repetitive files and smallest on random(non repetitive) data

| File Type | Original Size | Huffman Size | Huffman Ratio | Huffman Time | Gzip Size | Gzip Ratio | Gzip Time |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Repetitive Text | 1,048,576 B | 131,093 B | 12.50% | 20.6 ms | 1,074 B | 0.10% | 5.3 ms |
| Lorem Text | 3,568,000 B | 1,861,282 B | 52.17% | 69.9 ms | 15,918 B | 0.45% | 17.0 ms |
| Random Binary | 102,400 B | 104,716 B | 102.26% | 5.8 ms | 102,456 B | 100.05% | 3.3 ms |
| Empty File | 0 B | 12 B | - | 1.1 ms | 37 B | - | 1.2 ms |
| Single-Byte File | 100 B | 34 B | 34.00% | 1.5 ms | 47 B | 47.00% | 0.9 ms |
| Tiny File | 5 B | 50 B | 1000.00% | 1.7 ms | 41 B | 820.00% | 2.0 ms |

For both huffman and gzip, tiny/random files may become larger after compression because the header information(magic bytes, frequency table, original size) take up more space than the data saved by compression. On the random 100 KB file,
the ~2.3 KB size increase closely matches the frequency table size for a
file using close to all 256 byte values.

## Limitations & Future Improvements
- Pure Huffman coding only — no LZ77-style repetition matching, so
  compression ratio trails gzip on repetitive data
- No format version byte — a future header change would break
  compatibility with previously compressed files
- No checksum — a corrupted compressed file could fail unpredictably
  or decode to incorrect output without any error