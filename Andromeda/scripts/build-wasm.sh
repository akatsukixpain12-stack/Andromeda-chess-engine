#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/src"
DIST="$ROOT/../dist"
mkdir -p "$DIST"
rm -f "$DIST/andromeda.js" "$DIST/andromeda.wasm" "$DIST/andromeda.js.map"

command -v em++ >/dev/null 2>&1 || {
    echo "error: Emscripten em++ was not found. Run this script inside an activated Emscripten SDK environment." >&2
    exit 127
}

cd "$SRC"

# Browser build uses the repository's actual lightweight UCI/search implementation.
# Do not add engine.cpp or the native-only Stockfish infrastructure here.
SOURCES=(
    main.cpp
    attacks.cpp
    bitboard.cpp
    evaluate.cpp
    movegen.cpp
    movepick.cpp
    position.cpp
    search.cpp
    timeman.cpp
    tt.cpp
    uci.cpp
)

em++ -O3 -flto -std=c++17 -DNDEBUG -msimd128 \
    "${SOURCES[@]}" \
    -o "$DIST/andromeda.js" \
    -sWASM=1 \
    -sMODULARIZE=1 \
    -sEXPORT_ES6=1 \
    -sEXPORT_NAME=AndromedaModule \
    -sALLOW_MEMORY_GROWTH=1 \
    -sNO_EXIT_RUNTIME=1 \
    -sASSERTIONS=0 \
    -sFILESYSTEM=0 \
    -sENVIRONMENT=web,worker \
    -sEXPORTED_FUNCTIONS="['_main','_initialize_engine','_send_uci_command','_get_uci_output']" \
    -sEXPORTED_RUNTIME_METHODS="['UTF8ToString','ccall','cwrap']" \
    --post-js "$ROOT/scripts/browser-api.js"

test -s "$DIST/andromeda.js"
test -s "$DIST/andromeda.wasm"

echo "WASM build OK: $DIST/andromeda.js"
echo "WASM build OK: $DIST/andromeda.wasm"
