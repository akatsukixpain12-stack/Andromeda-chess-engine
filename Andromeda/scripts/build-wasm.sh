#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIST="$ROOT/../dist"
mkdir -p "$DIST"

cd "$ROOT/src"

em++ -O3 -flto -std=c++17 -DNDEBUG -msimd128 \
  main.cpp attacks.cpp bitboard.cpp evaluate.cpp movegen.cpp movepick.cpp position.cpp search.cpp timeman.cpp tt.cpp uci.cpp \
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
  --extern-post-js "$ROOT/scripts/browser-api.js"

echo "Built $DIST/andromeda.js and $DIST/andromeda.wasm"
