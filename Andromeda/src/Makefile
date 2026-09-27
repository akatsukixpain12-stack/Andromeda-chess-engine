CXX ?= g++
CXXFLAGS ?= -O3 -std=c++17 -Wall -Wextra -flto -march=native -DNDEBUG
LDFLAGS ?= -pthread

SRC = main.cpp attacks.cpp bitboard.cpp evaluate.cpp movegen.cpp movepick.cpp position.cpp search.cpp timeman.cpp tt.cpp uci.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = andromeda

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) andromeda.js andromeda.wasm andromeda.worker.js

wasm:
	em++ -O3 -std=c++17 $(SRC) -o andromeda.js \
		-s WASM=1 \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_FUNCTIONS="['_main', '_send_uci_command']" \
		-s EXPORTED_RUNTIME_METHODS="['cwrap', 'ccall']" \
		-s MODULARIZE=1 \
		-s EXPORT_NAME="AndromedaModule"

.PHONY: all clean wasm
