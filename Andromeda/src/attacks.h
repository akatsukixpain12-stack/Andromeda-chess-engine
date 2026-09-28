/*
 * Andromeda chess engine
 *
 * This file contains Stockfish-inspired bitboard attack primitives.
 * Stockfish is GPLv3; see Copying.txt for the applicable license.
 */
#ifndef ATTACKS_H_INCLUDED
#define ATTACKS_H_INCLUDED

#include <array>
#include <cassert>
#include <utility>

#include "type.h"
#include "bitboard.h"

namespace Andromeda {

// Direction values match an A1..H8 little-endian rank/file board.
enum Direction : int {
    NORTH = 8, SOUTH = -8, EAST = 1, WEST = -1,
    NORTH_EAST = 9, SOUTH_EAST = -7,
    SOUTH_WEST = -9, NORTH_WEST = 7
};

inline constexpr bool is_ok(Square s) {
    return s >= SQ_A1 && s <= SQ_H8;
}

inline constexpr Bitboard pawn_attacks_bb(Color c, Bitboard b) {
    return c == WHITE
        ? ((b << 7) & ~FILE_H_BB) | ((b << 9) & ~FILE_A_BB)
        : ((b >> 9) & ~FILE_H_BB) | ((b >> 7) & ~FILE_A_BB);
}

constexpr Bitboard safe_destination(Square s, int step) {
    const int to_i = int(s) + step;
    if (to_i < 0 || to_i >= 64)
        return 0;
    const Square to = Square(to_i);
    const int df = int(file_of(s)) - int(file_of(to));
    return (df >= -2 && df <= 2) ? square_bb(to) : 0;
}

constexpr Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied) {
    Bitboard attacks = 0;
    constexpr Direction rook_dirs[4] = {NORTH, SOUTH, EAST, WEST};
    constexpr Direction bishop_dirs[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

    const auto& dirs = (pt == ROOK) ? rook_dirs : bishop_dirs;
    for (Direction d : dirs) {
        for (Square s = sq; Bitboard dest = safe_destination(s, d); s = Square(int(s) + d)) {
            attacks |= dest;
            if (occupied & dest)
                break;
        }
    }
    return attacks;
}

constexpr Bitboard knight_attack(Square sq) {
    Bitboard b = 0;
    for (int step : {-17, -15, -10, -6, 6, 10, 15, 17})
        b |= safe_destination(sq, step);
    return b;
}

constexpr Bitboard king_attack(Square sq) {
    Bitboard b = 0;
    for (int step : {-9, -8, -7, -1, 1, 7, 8, 9})
        b |= safe_destination(sq, step);
    return b;
}

constexpr Bitboard pseudo_attacks(PieceType pt, Square sq) {
    switch (pt) {
    case ROOK:   return sliding_attack(ROOK, sq, 0);
    case BISHOP: return sliding_attack(BISHOP, sq, 0);
    case QUEEN:  return sliding_attack(ROOK, sq, 0) | sliding_attack(BISHOP, sq, 0);
    case KNIGHT: return knight_attack(sq);
    case KING:   return king_attack(sq);
    default:     return 0;
    }
}

inline constexpr auto PseudoAttacks = []() constexpr {
    std::array<std::array<Bitboard, SQUARE_NB>, PIECE_TYPE_NB> a{};

    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = Square(i);
        a[PAWN][i]   = pawn_attacks_bb(WHITE, square_bb(s));
        a[KNIGHT][i] = knight_attack(s);
        a[BISHOP][i] = sliding_attack(BISHOP, s, 0);
        a[ROOK][i]   = sliding_attack(ROOK, s, 0);
        a[QUEEN][i]  = a[BISHOP][i] | a[ROOK][i];
        a[KING][i]   = king_attack(s);
    }
    return a;
}();

extern Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
extern Bitboard KnightAttacks[SQUARE_NB];
extern Bitboard KingAttacks[SQUARE_NB];
extern Bitboard BishopMasks[SQUARE_NB];
extern Bitboard RookMasks[SQUARE_NB];

extern Bitboard LineBB[SQUARE_NB][SQUARE_NB];
extern Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
extern Bitboard RayPassBB[SQUARE_NB][SQUARE_NB];

void init_attacks();

Bitboard bishop_attacks(Square s, Bitboard occupied);
Bitboard rook_attacks(Square s, Bitboard occupied);
inline Bitboard queen_attacks(Square s, Bitboard occupied) {
    return bishop_attacks(s, occupied) | rook_attacks(s, occupied);
}

inline Bitboard line_bb(Square s1, Square s2) {
    assert(is_ok(s1) && is_ok(s2));
    return LineBB[s1][s2];
}

inline Bitboard between_bb(Square s1, Square s2) {
    assert(is_ok(s1) && is_ok(s2));
    return BetweenBB[s1][s2];
}

inline Bitboard ray_pass_bb(Square s1, Square s2) {
    assert(is_ok(s1) && is_ok(s2));
    return RayPassBB[s1][s2];
}

inline Bitboard attacks_bb(PieceType pt, Square s, Color c = COLOR_NB) {
    assert(is_ok(s));
    if (pt == PAWN)
        return c < COLOR_NB ? PawnAttacks[c][s] : 0;
    return PseudoAttacks[pt][s];
}

inline Bitboard attacks_bb(PieceType pt, Square s, Bitboard occupied) {
    assert(pt != PAWN && is_ok(s));
    switch (pt) {
    case BISHOP: return bishop_attacks(s, occupied);
    case ROOK:   return rook_attacks(s, occupied);
    case QUEEN:  return queen_attacks(s, occupied);
    default:     return PseudoAttacks[pt][s];
    }
}

inline Bitboard attacks_bb(Piece pc, Square s, Bitboard occupied) {
    return type_of(pc) == PAWN ? PawnAttacks[color_of(pc)][s]
                                : attacks_bb(type_of(pc), s, occupied);
}

} // namespace Andromeda

#endif // ATTACKS_H_INCLUDED
