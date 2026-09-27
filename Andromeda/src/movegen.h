#pragma once
#include "position.h"

namespace Andromeda {

enum GenType {
    GEN_LEGAL,
    GEN_CAPTURES,
    GEN_QUIETS
};

template<GenType Type>
struct ExtMove {
    Move move;
    int value;
};

template<GenType Type>
Move* generate_moves(const Position& pos, Move* move_list);

int generate_legal_moves(Position& pos, Move* move_list);

} // namespace Andromeda
