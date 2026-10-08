#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Andromeda regression/SPRT harness.
# This script is tooling only; it does not change engine evaluation/search code.
#
# Usage:
#   BOOK=/path/to/book.pgn ./scripts/test-sprt.sh
#
# Optional:
#   BASE_REF=HEAD~1 CANDIDATE_REF=HEAD GAMES=10000 ./scripts/test-sprt.sh

set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
ENGINE_ROOT="$ROOT/Andromeda"
BASE_REF="${BASE_REF:-HEAD~1}"
CANDIDATE_REF="${CANDIDATE_REF:-HEAD}"
BOOK="${BOOK:-}"
GAMES="${GAMES:-10000}"
TC="${TC:-10+0.1}"
THREADS="${THREADS:-1}"
HASH_MB="${HASH_MB:-16}"
DEPTH="${DEPTH:-12}"

for tool in git make awk diff; do
    command -v "$tool" >/dev/null 2>&1 || {
        echo "error: required tool not found: $tool" >&2
        exit 2
    }
done

command -v cutechess-cli >/dev/null 2>&1 || {
    echo "error: cutechess-cli is required for the SPRT stage" >&2
    exit 2
}

if [[ -z "$BOOK" || ! -f "$BOOK" ]]; then
    echo "error: set BOOK to an 8-move PGN opening book" >&2
    exit 2
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

BASE_DIR="$TMP/base"
CAND_DIR="$TMP/candidate"
BASE_BIN="$BASE_DIR/Andromeda/src/andromeda"
CAND_BIN="$CAND_DIR/Andromeda/src/andromeda"

echo "== Andromeda regression/SPRT harness =="
echo "base      : $BASE_REF"
echo "candidate : $CANDIDATE_REF"
echo "TC        : $TC"
echo "threads   : $THREADS"
echo "hash      : $HASH_MB MB"
echo "games     : $GAMES"
echo "book      : $BOOK"

git archive "$BASE_REF" | tar -x -C "$TMP"
mv "$TMP/Andromeda" "$BASE_DIR"

git archive "$CANDIDATE_REF" | tar -x -C "$TMP"
mv "$TMP/Andromeda" "$CAND_DIR"

build_engine() {
    local dir="$1"
    echo
    echo "== Building $dir =="
    make -C "$dir/Andromeda/src" clean
    make -C "$dir/Andromeda/src" -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
}

build_engine "$BASE_DIR"
build_engine "$CAND_DIR"

[[ -x "$BASE_BIN" ]] || { echo "error: base binary was not built" >&2; exit 1; }
[[ -x "$CAND_BIN" ]] || { echo "error: candidate binary was not built" >&2; exit 1; }

uci_check() {
    local bin="$1"
    printf 'uci\nisready\nquit\n' | "$bin" >/dev/null
}

echo
echo "== UCI smoke test =="
uci_check "$BASE_BIN"
uci_check "$CAND_BIN"

# The current Andromeda command loop does not expose Stockfish's textual
# 'bench' command.  Until that command exists, this is the deterministic
# node-count regression check: both binaries search the same fixed FEN set
# to the same depth and their reported node counts are compared.
BENCH_FENS=(
    "startpos"
    "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"
    "r3k2r/ppp1bppp/2n1p3/8/2BP4/2N2N2/PPP2PPP/R2Q1RK1 w kq - 0 10"
    "8/2p5/3p4/KP1P4/8/8/8/8 w - - 0 1"
)

run_nodes() {
    local bin="$1"
    local out="$2"
    : > "$out"

    for fen in "${BENCH_FENS[@]}"; do
        if [[ "$fen" == "startpos" ]]; then
            printf 'position startpos\ngo depth %s\n' "$DEPTH" |
                "$bin" | awk '/^bestmove/{print prev} /^info .* nodes [0-9]+/{prev=$0}'
        else
            printf 'position fen %s\ngo depth %s\n' "$fen" "$DEPTH" |
                "$bin" | awk '/^bestmove/{print prev} /^info .* nodes [0-9]+/{prev=$0}'
        fi
    done > "$out"
}

echo
echo "== Node-count regression check =="
run_nodes "$BASE_BIN" "$TMP/base.nodes"
run_nodes "$CAND_BIN" "$TMP/candidate.nodes"

BASE_NODES="$(awk 'match($0,/nodes [0-9]+/){s+=substr($0,RSTART+6,RLENGTH-6)} END{print s+0}' "$TMP/base.nodes")"
CAND_NODES="$(awk 'match($0,/nodes [0-9]+/){s+=substr($0,RSTART+6,RLENGTH-6)} END{print s+0}' "$TMP/candidate.nodes")"

echo "base nodes      : $BASE_NODES"
echo "candidate nodes : $CAND_NODES"

if [[ "$BASE_NODES" -eq 0 || "$CAND_NODES" -eq 0 ]]; then
    echo "error: could not obtain deterministic node counts" >&2
    exit 1
fi

if [[ "$BASE_NODES" -ne "$CAND_NODES" ]]; then
    echo "warning: node count changed; inspect before interpreting SPRT results"
fi

echo
echo "== SPRT =="
echo "cutechess-cli -engine cmd=$BASE_BIN name=Andromeda-base -engine cmd=$CAND_BIN name=Andromeda-candidate -each tc=$TC proto=uci option.Threads=$THREADS option.Hash=$HASH_MB -openings file=$BOOK format=pgn plies=8 -games $GAMES -sprt elo0=0 elo1=2 alpha=0.05 beta=0.05 -concurrency 1"

cutechess-cli \
    -engine cmd="$BASE_BIN" name=Andromeda-base \
    -engine cmd="$CAND_BIN" name=Andromeda-candidate \
    -each tc="$TC" proto=uci option.Threads="$THREADS" option.Hash="$HASH_MB" \
    -openings file="$BOOK" format=pgn plies=8 \
    -games "$GAMES" \
    -sprt elo0=0 elo1=2 alpha=0.05 beta=0.05 \
    -concurrency 1

echo
echo "SPRT finished. Interpret the result before accepting the candidate."
