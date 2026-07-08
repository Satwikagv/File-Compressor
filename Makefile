CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
SRC = src/main.cpp src/huffman_tree.cpp src/compressor.cpp
OBJ = $(SRC:.cpp=.o)

.PHONY: all release debug clean

all: release

release: CXXFLAGS += -O3
release: clean_bin compress decompress

debug: CXXFLAGS += -g -O0
debug: clean_bin compress decompress 

compress: $(OBJ)
	$(CXX) $(CXXFLAGS) -o compress $(OBJ)

decompress: compress
	ln -sf compress decompress

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean_bin:
	rm -f compress decompress

clean:
	rm -f src/*.o compress decompress
