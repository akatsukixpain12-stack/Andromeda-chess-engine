# Andromeda Analysis Website

This is a separate UI folder; it does not replace or modify the existing engine UI.

Modes:
1. Andromeda vs People
2. Andromeda vs Stockfish 18

The board uses Chessboard.js 1.0.0 from the supplied package. Chess.js handles legal game state and move validation.

Runtime files:
- ../dist/andromeda.js
- ../dist/andromeda.wasm
- website/stockfish.js
- website/stockfish.wasm

Important: the supplied Stockfish 18 JavaScript wrapper loads a separate stockfish.wasm file, so that binary must be placed beside stockfish.js before Engine vs Engine can run.

Serve over HTTP, for example: python -m http.server 8080
Then open /website/.