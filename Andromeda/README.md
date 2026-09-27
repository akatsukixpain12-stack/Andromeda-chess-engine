# 🌌 Andromeda Chess Engine v3.0

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Engine: UCI](https://img.shields.io/badge/Protocol-UCI%202.0-brightgreen.svg)]()
[![Build: WebAssembly](https://img.shields.io/badge/Platform-C%2B%2B17%20%7C%20WASM-purple.svg)]()
[![Performance: NNUE](https://img.shields.io/badge/Architecture-Bitboard%20%2B%20NNUE-orange.svg)]()

Andromeda is an ultra-fast, tournament-grade UCI chess engine designed from the ground up in modern C++17 and compiled to both high-throughput native binaries and high-performance WebAssembly (`.wasm` + `.js`).

---

## ⚡ Direct Downloads (WebAssembly & UCI Binaries)

Click the links below to instantly download the engine builds for web integration or GUI clients:

- 📦 **[Download andromeda.wasm](/dist/andromeda.wasm)** (`andromeda.wasm` - Compiled WebAssembly Core)
- 📜 **[Download andromeda.js](/dist/andromeda.js)** (`andromeda.js` - JS Module Wrapper)
- ⚙️ **[Download andromeda.worker.js](/dist/andromeda.worker.js)** (`andromeda.worker.js` - Background Thread Web Worker)

---

## 🚀 Quick Web Integration

To use Andromeda inside any website or chess application:

```html
<script src="/dist/andromeda.js"></script>
<script>
  const engine = new Andromeda();
  engine.onMessage((line) => {
    console.log('[UCI Out]', line);
  });

  // Initialize and request best move
  engine.sendCommand('uci');
  engine.sendCommand('isready');
  engine.sendCommand('position startpos moves e2e4 e7e5');
  engine.sendCommand('go depth 18');
</script>
```

### Web Worker Setup (Non-blocking search)

```js
const worker = new Worker('/dist/andromeda.worker.js');
worker.onmessage = (e) => {
  console.log('Andromeda:', e.data);
};
worker.postMessage('uci');
worker.postMessage('go depth 20');
```

---

## 🛠️ Engine Architecture

| Component | Technical Specification |
|---|---|
| **Representation** | 64-bit Magic Bitboards with AVX2/AVX-512 hardware popcount |
| **Evaluation** | Dual Tapered Classical HCE + 768→256→32→1 NNUE network |
| **Search Algorithm** | Principal Variation Search (PVS) with Aspiration Windows |
| **Pruning & Reductions** | Null-Move Pruning (NMP), Late Move Reductions (LMR), Delta Pruning, Futility |
| **Move Ordering** | TT Move → Good MVV-LVA Captures → Killer Moves (2-ply) → History Heuristic |
| **Transposition Table** | Lockless Zobrist 64-bit hashing with dynamic bucket aging |
| **UCI Options** | `Hash`, `Threads`, `OwnBook`, `MultiPV`, `Skill Level` |

---

## 💻 Compiling From Source

### 1. Native Build (GCC / Clang)
```bash
cd Andromeda/src
make -j$(nproc)
./andromeda
```

### 2. WebAssembly Build (Emscripten)
```bash
cd Andromeda/scripts
./build-wasm.sh
```

---

## 📜 Standard UCI Commands Supported

- `uci` - Handshake and engine capability negotiation
- `isready` - Synchronization probe
- `setoption name <name> value <val>` - Set transposition table size or CPU threads
- `ucinewgame` - Clear hash tables and initialize game state
- `position [startpos | fen <fen>] moves <m1> <m2> ...` - Set active board state
- `go [depth <d>] [wtime <ms>] [btime <ms>] [movetime <ms>] [infinite]` - Execute search
- `eval` - Output static evaluation breakdown
- `stop` - Halt active search immediately

---

## 📄 License

Andromeda is released under the **MIT License**.
