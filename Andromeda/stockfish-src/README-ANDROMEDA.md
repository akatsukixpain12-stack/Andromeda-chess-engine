# Stockfish source snapshot

This directory contains the official Stockfish source snapshot recorded in UPSTREAM_COMMIT.
It is kept separate from Andromeda/src because the current Andromeda implementation uses a different API;
blindly overwriting it with Stockfish files would break its current native and WebAssembly entry points.
Stockfish is GPLv3-or-later. Preserve all upstream notices when modifying or redistributing this source.
Upstream: https://github.com/official-stockfish/Stockfish
