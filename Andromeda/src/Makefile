CXX ?= g++
CXXFLAGS ?= -O3 -std=c++17 -Wall -Wextra -flto -march=native -DNDEBUG
LDFLAGS ?= -pthread

SRC = main.cpp attacks.cpp bitboard.cpp evaluate.cpp engine.cpp memory.cpp misc.cpp movegen.cpp movepick.cpp position.cpp score.cpp search.cpp thread.cpp timeman.cpp tt.cpp uci.cpp ucioption.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = andromeda

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

wasm:
	../../scripts/build-wasm.sh

.PHONY: all clean wasm
