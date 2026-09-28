#include "benchmark.h"
#include "position.h"
#include "movegen.h"
#include "attacks.h"
#include "bitboard.h"
#include "evaluate.h"
#include <cstdint>

namespace Andromeda {
uint64_t benchmark_nodes(Position& pos, int depth) {
    if (depth <= 0) return 1;
    Move moves[256];
    int n = generate_legal_moves(pos, moves);
    if (depth == 1) return static_cast<uint64_t>(n);
    uint64_t nodes = 0;
    StateInfo si;
    for (int i=0;i<n;++i) {
        if (!pos.do_move(moves[i], si)) continue;
        nodes += benchmark_nodes(pos, depth-1);
        pos.undo_move(moves[i]);
    }
    return nodes;
}
}