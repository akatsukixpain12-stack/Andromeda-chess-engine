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
- UCI protocol with Hash, Aggression, Threads, position, go, stop, eval and d

## Browser / WASM build

The browser build is intentionally compiled from the repository's actual lightweight UCI/search implementation. It does **not** compile the native-only Stockfish infrastructure in `engine.cpp`, NUMA, Syzygy, or native thread-pool code.

The build produces:

```
dist/andromeda.js
dist/andromeda.wasm
```

### Build locally

Install/activate Emscripten first, then:

```bash
bash Andromeda/scripts/build-wasm.sh
```

The script fails immediately if `em++` is unavailable and verifies that both generated files are non-empty.

### GitHub Actions

Every push and pull request to `main` runs the WASM build. The workflow validates the generated JavaScript/WASM pair and uploads them as the `andromeda-wasm` artifact.

The workflow **does not commit generated WASM files back to `main`**. This avoids build-commit loops and avoids changing the branch while another commit or pull request is being tested.

### Add Andromeda to a TypeScript website

Use `Andromeda/web/andromeda.ts` as the integration wrapper.

Copy it into your website and adjust the import path if your website stores the generated files elsewhere. The wrapper uses Emscripten's modularized ES-module factory and explicitly locates `andromeda.wasm`.

Example:

```ts
import { createAndromedaEngine } from "./andromeda";

const engine = await createAndromedaEngine();

engine.uci("uci");
engine.uci("isready");

engine.setOption("Hash", 64);
engine.setOption("Aggression", 75);

engine.positionStart();
const result = engine.go({ depth: 16 });

console.log(result);
// Contains UCI info lines and the final bestmove line.
```

For a position from a game:

```ts
engine.positionStart([
  "e2e4",
  "e7e5",
  "g1f3",
  "b8c6",
]);

const result = engine.go({ movetime: 1000 });
console.log(result);
```

For a FEN:

```ts
engine.positionFen(
  "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
);

const result = engine.go({ depth: 18 });
console.log(result);
```

The returned text is normal UCI output, so a website can extract the final move with:

```ts
const bestMove = result
  .split("\\n")
  .find((line) => line.startsWith("bestmove "))
  ?.split(/\\s+/)[1];

console.log(bestMove);
```

### Important website note

The current search implementation is synchronous. A long `go` call therefore runs on the JavaScript thread and can temporarily freeze the page.

For a production chess website, run the wrapper inside a Web Worker and send commands/results between the UI and worker. Do not use `setTimeout` as a fake workaround; the actual engine search must be moved off the UI thread.

### WASM file placement

Keep the generated files together:

```
your-site/
  dist/
    andromeda.js
    andromeda.wasm
  chess/
    andromeda.ts
```

Or change `wasmBaseUrl`:

```ts
const engine = await createAndromedaEngine(
  new URL("/engines/andromeda/", window.location.origin),
);
```

The server must serve `.wasm` as a WebAssembly binary. If your hosting platform does not set this automatically, configure:

```
Content-Type: application/wasm
```

Do not open the HTML file directly with `file://`; serve the website through a local/production HTTP(S) server.

## Strength note

Andromeda is being developed as an independent engine. Stockfish source was used as an architectural reference, but Stockfish code and NNUE weights are not copied into this repository. Playing strength must be established by objective testing such as tactical suites, self-play and engine-vs-engine matches before claiming superiority over Stockfish.

## License

See the repository license files.

