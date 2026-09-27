#include "attacks.h"
#include <vector>

namespace Andromeda {

Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
Bitboard KnightAttacks[SQUARE_NB];
Bitboard KingAttacks[SQUARE_NB];
Bitboard BishopMasks[SQUARE_NB];
Bitboard RookMasks[SQUARE_NB];

namespace {
    const int BishopDirections[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    const int RookDirections[4][2]   = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    Bitboard sliding_attack(int r, int f, int dr, int df, Bitboard occ) {
        Bitboard attacks = 0;
        int cr = r + dr;
        int cf = f + df;
        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8) {
            Square sq = make_square(static_cast<File>(cf), static_cast<Rank>(cr));
            attacks |= square_bb(sq);
            if (occ & square_bb(sq)) break;
            cr += dr;
            cf += df;
        }
        return attacks;
    }
}

Bitboard bishop_attacks(Square s, Bitboard occ) {
    int r = rank_of(s);
    int f = file_of(s);
    Bitboard att = 0;
    for (auto& d : BishopDirections) {
        att |= sliding_attack(r, f, d[0], d[1], occ);
    }
    return att;
}

Bitboard rook_attacks(Square s, Bitboard occ) {
    int r = rank_of(s);
    int f = file_of(s);
    Bitboard att = 0;
    for (auto& d : RookDirections) {
        att |= sliding_attack(r, f, d[0], d[1], occ);
    }
    return att;
}

void init_attacks() {
    for (int s = 0; s < 64; ++s) {
        Square sq = static_cast<Square>(s);
        int r = rank_of(sq);
        int f = file_of(sq);

        // Pawn Attacks
        if (r < 7) {
            if (f > 0) PawnAttacks[WHITE][sq] |= square_bb(make_square(static_cast<File>(f - 1), static_cast<Rank>(r + 1)));
            if (f < 7) PawnAttacks[WHITE][sq] |= square_bb(make_square(static_cast<File>(f + 1), static_cast<Rank>(r + 1)));
        }
        if (r > 0) {
            if (f > 0) PawnAttacks[BLACK][sq] |= square_bb(make_square(static_cast<File>(f - 1), static_cast<Rank>(r - 1)));
            if (f < 7) PawnAttacks[BLACK][sq] |= square_bb(make_square(static_cast<File>(f + 1), static_cast<Rank>(r - 1)));
        }

        // Knight Attacks
        const int k_offsets[8][2] = {
            {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
            {1, -2}, {1, 2}, {2, -1}, {2, 1}
        };
        for (auto& o : k_offsets) {
            int nr = r + o[0];
            int nf = f + o[1];
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
                KnightAttacks[sq] |= square_bb(make_square(static_cast<File>(nf), static_cast<Rank>(nr)));
            }
        }

        // King Attacks
        const int king_offsets[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            {0, -1},           {0, 1},
            {1, -1},  {1, 0},  {1, 1}
        };
        for (auto& o : king_offsets) {
            int nr = r + o[0];
            int nf = f + o[1];
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
                KingAttacks[sq] |= square_bb(make_square(static_cast<File>(nf), static_cast<Rank>(nr)));
            }
        }
    }
}

} // namespace Andromeda
