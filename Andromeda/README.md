# Andromeda Chess Engine v4.0

Andromeda is a UCI chess engine written in C++17 with a browser-ready WebAssembly build.

## Engine core

- 64-bit bitboards and incremental Zobrist hashing
- Principal Variation Search with iterative deepening
- Aspiration windows
- Transposition-table cutoffs with mate-score normalization
- Null-move pruning
- Razoring and shallow futility pruning
- Late Move Reductions
- Quiescence search with check evasions
- Killer + history move ordering
- Tapered classical evaluation
- Mobility, passed-pawn, pawn-structure, rook-file, king-pressure and endgame terms
- Configurable Aggression evaluation parameter
- UCI protocol with Hash, Aggression, position, go, stop, eval and d

## Browser engine

The WebAssembly build is generated automatically into dist/andromeda.js and dist/andromeda.wasm.

The WASM module exports initialize_engine, send_uci_command and get_uci_output.

## Build

Native: cd Andromeda/src && make -j

WASM: bash Andromeda/scripts/build-wasm.sh

The repository workflow builds the browser artifacts with Emscripten and commits the generated files to dist/.

## Strength note

Andromeda is being developed as an independent engine. Stockfish source was used as an architectural reference, but Stockfish code and NNUE weights are not copied into this repository. Playing strength still needs objective testing such as tactical suites, self-play and engine-vs-engine matches before claiming superiority over Stockfish.

## License

See the repository license files.
