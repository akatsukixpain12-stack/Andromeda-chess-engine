#!/bin/bash
set -e
echo "=== Building Andromeda WebAssembly Engine ==="
mkdir -p ../dist ../build/wasm
cd ../src
em++ -O3 -std=c++17 main.cpp attacks.cpp bitboard.cpp evaluate.cpp movegen.cpp movepick.cpp position.cpp search.cpp timeman.cpp tt.cpp uci.cpp \
    -o ../dist/andromeda.js \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORTED_FUNCTIONS="['_main', '_send_uci_command']" \
    -s EXPORTED_RUNTIME_METHODS="['cwrap', 'ccall']" \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="AndromedaModule"
echo "Build complete: ../dist/andromeda.js & ../dist/andromeda.wasm"
