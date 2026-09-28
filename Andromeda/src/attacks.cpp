#include "attacks.h"

#include <algorithm>

namespace Andromeda {

Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
Bitboard KnightAttacks[SQUARE_NB];
Bitboard KingAttacks[SQUARE_NB];
Bitboard BishopMasks[SQUARE_NB];
Bitboard RookMasks[SQUARE_NB];
Bitboard LineBB[SQUARE_NB][SQUARE_NB];
Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
Bitboard RayPassBB[SQUARE_NB][SQUARE_NB];

namespace {

Bitboard ray_between_or_line(Square a, Square b, bool between_only) {
    const int af = int(file_of(a)), ar = int(rank_of(a));
    const int bf = int(file_of(b)), br = int(rank_of(b));
    const int df = bf - af, dr = br - ar;

    int sf = 0, sr = 0;
    if (df == 0 && dr != 0) sr = dr > 0 ? 1 : -1;
    else if (dr == 0 && df != 0) sf = df > 0 ? 1 : -1;
    else if (std::abs(df) == std::abs(dr) && df != 0) {
        sf = df > 0 ? 1 : -1;
        sr = dr > 0 ? 1 : -1;
    } else {
        return 0;
    }

    Bitboard result = 0;
    int f = af, r = ar;
    if (!between_only)
        result |= square_bb(a);

    f += sf;
    r += sr;
    while (f != bf || r != br) {
        result |= square_bb(make_square(File(f), Rank(r)));
        f += sf;
        r += sr;
    }

    if (!between_only)
        result |= square_bb(b);

    return result;
}

} // namespace

Bitboard bishop_attacks(Square s, Bitboard occ) {
    return sliding_attack(BISHOP, s, occ);
}

Bitboard rook_attacks(Square s, Bitboard occ) {
    return sliding_attack(ROOK, s, occ);
}

void init_attacks() {
    for (int s = 0; s < SQUARE_NB; ++s) {
        PawnAttacks[WHITE][s] = pawn_attacks_bb(WHITE, square_bb(Square(s)));
        PawnAttacks[BLACK][s] = pawn_attacks_bb(BLACK, square_bb(Square(s)));
        KnightAttacks[s] = knight_attack(Square(s));
        KingAttacks[s] = king_attack(Square(s));
        BishopMasks[s] = sliding_attack(BISHOP, Square(s), 0);
        RookMasks[s] = sliding_attack(ROOK, Square(s), 0);
    }

    for (int a = 0; a < SQUARE_NB; ++a) {
        for (int b = 0; b < SQUARE_NB; ++b) {
            const Square s1 = Square(a);
            const Square s2 = Square(b);
            const Bitboard line = ray_between_or_line(s1, s2, false);
            LineBB[a][b] = line;

            if (line == 0 || a == b) {
                BetweenBB[a][b] = 0;
                RayPassBB[a][b] = 0;
                continue;
            }

            const int af = int(file_of(s1)), ar = int(rank_of(s1));
            const int bf = int(file_of(s2)), br = int(rank_of(s2));
            const int sf = (bf > af) - (bf < af);
            const int sr = (br > ar) - (br < ar);

            Bitboard between = 0;
            int f = af + sf, r = ar + sr;
            while (f != bf || r != br) {
                between |= square_bb(make_square(File(f), Rank(r)));
                f += sf;
                r += sr;
            }
            between &= ~square_bb(s1);
            between &= ~square_bb(s2);
            BetweenBB[a][b] = between;

            Bitboard pass = 0;
            f = bf + sf;
            r = br + sr;
            while (f >= 0 && f < 8 && r >= 0 && r < 8) {
                pass |= square_bb(make_square(File(f), Rank(r)));
                f += sf;
                r += sr;
            }
            RayPassBB[a][b] = pass;
        }
    }
}

} // namespace Andromeda
