# Andromeda browser engine build

andromeda.js is the Emscripten module and andromeda.wasm is the compiled engine core.

The engine exposes:
- initialize_engine()
- send_uci_command("uci")
- get_uci_output()

The generated WASM files are produced by .github/workflows/build-wasm.yml.
