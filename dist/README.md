# Andromeda browser engine

This directory contains the two runtime files a website needs to embed the browser engine:

- andromeda.js — Emscripten ES-module loader plus the createAndromedaEngine() convenience API.
- andromeda.wasm — compiled Andromeda engine core.

Keep both files in the same directory. andromeda.js automatically locates andromeda.wasm relative to itself.

## Minimal website integration

```js
import AndromedaModule from "./andromeda.js";

const module = await AndromedaModule();
const engine = module.createAndromedaEngine();

engine.uci("uci");
engine.uci("isready");
engine.uci("position startpos");
const output = engine.uci("go depth 15");
console.log(output);
```

The API is still UCI underneath, so sites can send normal commands such as position fen ..., go movetime ..., go depth ..., stop, and setoption name Hash value 128.

## Direct low-level API

The generated module also exports initialize_engine(), send_uci_command(command), and get_uci_output().

The generated artifacts are rebuilt by .github/workflows/build-wasm.yml whenever main changes.
