#pragma once
#include "type.h"
#include "bitboard.h"

namespace Andromeda {

extern Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
extern Bitboard KnightAttacks[SQUARE_NB];
extern Bitboard KingAttacks[SQUARE_NB];
extern Bitboard BishopMasks[SQUARE_NB];
extern Bitboard RookMasks[SQUARE_NB];

void init_attacks();

Bitboard bishop_attacks(Square s, Bitboard occ);
Bitboard rook_attacks(Square s, Bitboard occ);

inline Bitboard queen_attacks(Square s, Bitboard occ) {
    return bishop_attacks(s, occ) | rook_attacks(s, occ);
}

inline Bitboard attacks_bb(PieceType pt, Square s, Bitboard occ) {
    switch (pt) {
        case KNIGHT: return KnightAttacks[s];
        case BISHOP: return bishop_attacks(s, occ);
        case ROOK:   return rook_attacks(s, occ);
        case QUEEN:  return queen_attacks(s, occ);
        case KING:   return KingAttacks[s];
        default:     return 0;
    }
}

} // namespace Andromeda
