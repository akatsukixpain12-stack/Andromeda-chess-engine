#include "evaluate.h"
#include "attacks.h"
#include "bitboard.h"

#include <algorithm>
#include <cstdlib>

namespace Andromeda {

namespace {
constexpr int PieceValueMG[PIECE_TYPE_NB] = {0, 100, 320, 335, 500, 980, 20000};
constexpr int PieceValueEG[PIECE_TYPE_NB] = {0, 115, 305, 330, 560, 1010, 20000};
constexpr int PhaseWeight[PIECE_TYPE_NB] = {0, 0, 1, 1, 2, 4, 0};
constexpr int TotalPhase = 24;

const int PawnPST_MG[64] = {
     0,0,0,0,0,0,0,0, 5,10,10,-18,-18,10,10,5, 5,-5,-10,2,2,-10,-5,5,
     0,0,0,18,20,0,0,0, 4,6,10,22,22,10,6,4, 12,12,20,30,30,20,12,12,
     45,48,52,55,55,52,48,45, 0,0,0,0,0,0,0,0
};
const int PawnPST_EG[64] = {
     0,0,0,0,0,0,0,0, 12,15,18,22,22,18,15,12, 20,24,28,34,34,28,24,20,
     32,38,42,48,48,42,38,32, 46,52,58,64,64,58,52,46, 60,68,74,80,80,74,68,60,
     78,86,92,100,100,92,86,78, 0,0,0,0,0,0,0,0
};
const int KnightPST[64] = {
   -50,-40,-25,-20,-20,-25,-40,-50,-40,-18,0,8,8,0,-18,-40,-30,4,14,22,22,14,4,-30,
   -28,0,18,27,27,18,0,-28,-28,5,18,28,28,18,5,-28,-30,2,12,18,18,12,2,-30,-40,-18,0,2,2,0,-18,-40,-50,-40,-25,-20,-20,-25,-40,-50
};
const int BishopPST[64] = {
   -24,-12,-10,-10,-10,-10,-12,-24,-10,5,2,4,4,2,5,-10,-10,10,12,12,12,12,10,-10,
   -10,2,12,18,18,12,2,-10,-10,5,8,12,12,8,5,-10,-10,0,6,10,10,6,0,-10,-12,0,0,2,2,0,0,-12,-24,-12,-10,-10,-10,-10,-12,-24
};
const int RookPST[64] = {
    0,0,0,5,5,0,0,0,-4,0,1,2,2,1,0,-4,-4,0,1,2,2,1,0,-4,-2,2,3,5,5,3,2,-2,
   -2,2,3,5,5,3,2,-2,-4,0,1,2,2,1,0,-4,4,8,10,12,12,10,8,4,0,0,2,4,4,2,0,0
};
const int QueenPST[64] = {
   -18,-10,-8,-4,-4,-8,-10,-18,-8,0,4,7,7,4,0,-8,-8,4,6,9,9,6,4,-8,
   -4,2,8,10,10,8,2,-4,-4,2,8,10,10,8,2,-4,-8,4,6,9,9,6,4,-8,-8,0,4,7,7,4,0,-8,
   -18,-10,-8,-4,-4,-8,-10,-18
};
const int KingPST_MG[64] = {
    18,26,10,0,0,10,26,18,18,20,2,-4,-4,2,20,18,-10,-18,-22,-24,-24,-22,-18,-10,
   -20,-28,-34,-40,-40,-34,-28,-20,-28,-36,-44,-50,-50,-44,-36,-28,-30,-38,-46,-52,-52,-46,-38,-30,-30,-38,-46,-52,-52,-46,-38,-32,-40,-44,-48,-48,-44,-40,-32
};
const int KingPST_EG[64] = {
   -48,-30,-24,-20,-20,-24,-30,-48,-30,-10,2,6,6,2,-10,-30,-24,2,18,24,24,18,2,-24,
   -20,6,24,34,34,24,6,-20,-20,6,24,34,34,24,6,-20,-24,2,18,24,24,18,2,-24,-30,-10,2,6,6,2,-10,-30,-48,-30,-24,-20,-20,-24,-30,-48
};

int g_aggression = 55;

inline Square relative_square(Color c, Square s) {
    return c == WHITE ? s : static_cast<Square>(s ^ 56);
}
inline int pawn_file_count(Bitboard pawns, int file) {
    return popcount(pawns & (FILE_A_BB << file));
}
bool passed_pawn(Color c, Square s, Bitboard enemy_pawns) {
    const int f = file_of(s), r = rank_of(s);
    for (int ef = std::max(0, f - 1); ef <= std::min(7, f + 1); ++ef) {
        if (c == WHITE) {
            for (int er = r + 1; er < 8; ++er)
                if (enemy_pawns & square_bb(make_square(static_cast<File>(ef), static_cast<Rank>(er))))
                    return false;
        } else {
            for (int er = r - 1; er >= 0; --er)
                if (enemy_pawns & square_bb(make_square(static_cast<File>(ef), static_cast<Rank>(er))))
                    return false;
        }
    }
    return true;
}
void pawn_structure(Color c, const Position& pos, int& mg, int& eg) {
    const Bitboard own = pos.pieces(c, PAWN);
    const Bitboard enemy = pos.pieces(~c, PAWN);

    for (int f = 0; f < 8; ++f) {
        const int count = pawn_file_count(own, f);
        if (count > 1) {
            mg -= 12 * (count - 1);
            eg -= 10 * (count - 1);
        }
        if (count == 0)
            continue;
        const bool left = f > 0 && pawn_file_count(own, f - 1) > 0;
        const bool right = f < 7 && pawn_file_count(own, f + 1) > 0;
        if (!left && !right) {
            mg -= 10;
            eg -= 8;
        }
    }

    Bitboard pawns = own;
    while (pawns) {
        const Square s = pop_lsb(pawns);
        if (passed_pawn(c, s, enemy)) {
            const int adv = c == WHITE ? rank_of(s) : 7 - rank_of(s);
            mg += 6 + adv * 7;
            eg += 16 + adv * 13;
        }
    }
}
int mobility(Color c, const Position& pos) {
    const Bitboard own = pos.pieces(c);
    const Bitboard occ = pos.pieces();
    int score = 0;

    Bitboard b = pos.pieces(c, KNIGHT);
    while (b) score += 4 * popcount(KnightAttacks[pop_lsb(b)] & ~own);
    b = pos.pieces(c, BISHOP);
    while (b) score += 4 * popcount(bishop_attacks(pop_lsb(b), occ) & ~own);
    b = pos.pieces(c, ROOK);
    while (b) score += 2 * popcount(rook_attacks(pop_lsb(b), occ) & ~own);
    b = pos.pieces(c, QUEEN);
    while (b) score += popcount(queen_attacks(pop_lsb(b), occ) & ~own);

    return score;
}
int rook_file_score(Color c, const Position& pos) {
    const Bitboard own_pawns = pos.pieces(c, PAWN);
    const Bitboard enemy_pawns = pos.pieces(~c, PAWN);
    int score = 0;
    Bitboard rooks = pos.pieces(c, ROOK);
    while (rooks) {
        const int f = file_of(pop_lsb(rooks));
        if (pawn_file_count(own_pawns, f) == 0)
            score += pawn_file_count(enemy_pawns, f) == 0 ? 18 : 9;
    }
    return score;
}
int king_pressure(Color attacker, const Position& pos) {
    const Color defender = ~attacker;
    const Square king = pos.king_square(defender);
    if (king == SQ_NONE)
        return 0;

    const Bitboard ring = KingAttacks[king];
    const Bitboard occ = pos.pieces();
    int score = 0;

    Bitboard b = pos.pieces(attacker, KNIGHT);
    while (b) score += 3 * popcount(KnightAttacks[pop_lsb(b)] & ring);
    b = pos.pieces(attacker, BISHOP);
    while (b) score += 2 * popcount(bishop_attacks(pop_lsb(b), occ) & ring);
    b = pos.pieces(attacker, ROOK);
    while (b) score += 2 * popcount(rook_attacks(pop_lsb(b), occ) & ring);
    b = pos.pieces(attacker, QUEEN);
    while (b) score += 4 * popcount(queen_attacks(pop_lsb(b), occ) & ring);

    return score;
}
int piece_material(Color c, const Position& pos) {
    int score = 0;
    for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN})
        score += PieceValueMG[pt] * popcount(pos.pieces(c, pt));
    return score;
}
int center_king_bonus(Color c, const Position& pos) {
    const Square k = pos.king_square(c);
    if (k == SQ_NONE)
        return 0;
    const int f = file_of(k), r = rank_of(k);
    return 6 - (std::abs(f - 3) + std::abs(r - 3));
}
} // namespace

void init_evaluation() {}
void set_aggression(int value) { g_aggression = std::clamp(value, 0, 100); }
int aggression() { return g_aggression; }

Value evaluate(const Position& pos) {
    int mg[COLOR_NB] = {0, 0};
    int eg[COLOR_NB] = {0, 0};
    int phase = 0;

    for (Color c : {WHITE, BLACK}) {
        Bitboard pawns = pos.pieces(c, PAWN);
        while (pawns) {
            const Square s = pop_lsb(pawns), r = relative_square(c, s);
            mg[c] += PieceValueMG[PAWN] + PawnPST_MG[r];
            eg[c] += PieceValueEG[PAWN] + PawnPST_EG[r];
        }
        Bitboard knights = pos.pieces(c, KNIGHT);
        while (knights) {
            const Square s = pop_lsb(knights), r = relative_square(c, s);
            mg[c] += PieceValueMG[KNIGHT] + KnightPST[r];
            eg[c] += PieceValueEG[KNIGHT] + KnightPST[r];
            phase += PhaseWeight[KNIGHT];
        }
        Bitboard bishops = pos.pieces(c, BISHOP);
        while (bishops) {
            const Square s = pop_lsb(bishops), r = relative_square(c, s);
            mg[c] += PieceValueMG[BISHOP] + BishopPST[r];
            eg[c] += PieceValueEG[BISHOP] + BishopPST[r];
            phase += PhaseWeight[BISHOP];
        }
        if (popcount(pos.pieces(c, BISHOP)) >= 2) {
            mg[c] += 32;
            eg[c] += 48;
        }
        Bitboard rooks = pos.pieces(c, ROOK);
        while (rooks) {
            const Square s = pop_lsb(rooks), r = relative_square(c, s);
            mg[c] += PieceValueMG[ROOK] + RookPST[r];
            eg[c] += PieceValueEG[ROOK] + RookPST[r];
            phase += PhaseWeight[ROOK];
        }
        Bitboard queens = pos.pieces(c, QUEEN);
        while (queens) {
            const Square s = pop_lsb(queens), r = relative_square(c, s);
            mg[c] += PieceValueMG[QUEEN] + QueenPST[r];
            eg[c] += PieceValueEG[QUEEN] + QueenPST[r];
            phase += PhaseWeight[QUEEN];
        }
        const Square king = pos.king_square(c);
        if (king != SQ_NONE) {
            const Square r = relative_square(c, king);
            mg[c] += KingPST_MG[r];
            eg[c] += KingPST_EG[r];
        }

        const int mob = mobility(c, pos);
        mg[c] += mob;
        eg[c] += mob / 2;

        int ps_mg = 0, ps_eg = 0;
        pawn_structure(c, pos, ps_mg, ps_eg);
        mg[c] += ps_mg;
        eg[c] += ps_eg;

        const int rf = rook_file_score(c, pos);
        mg[c] += rf;
        eg[c] += rf / 2;

        const int pressure = king_pressure(c, pos);
        mg[c] += pressure * (55 + g_aggression) / 100;
        eg[c] += pressure * (50 + g_aggression / 2) / 150;
    }

    phase = std::min(phase, TotalPhase);

    int mg_score = mg[WHITE] - mg[BLACK];
    int eg_score = eg[WHITE] - eg[BLACK];

    mg_score += (king_pressure(WHITE, pos) - king_pressure(BLACK, pos))
             * g_aggression / 100;

    int score = (mg_score * phase + eg_score * (TotalPhase - phase)) / TotalPhase;

    if (phase <= 8)
        score += (center_king_bonus(WHITE, pos) - center_king_bonus(BLACK, pos)) * 3;

    score += (piece_material(WHITE, pos) - piece_material(BLACK, pos)) / 1000;

    return pos.side_to_move() == WHITE ? score : -score;
}

} // namespace Andromeda
