#include "evaluate.h"

namespace Andromeda {

namespace {

// Tapered piece values [Opening, Endgame]
constexpr int PieceValueMG[PIECE_TYPE_NB] = { 0, 100, 320, 330, 500, 950, 20000 };
constexpr int PieceValueEG[PIECE_TYPE_NB] = { 0, 120, 300, 320, 550, 1000, 20000 };

// Game phase weights
constexpr int PhaseWeight[PIECE_TYPE_NB] = { 0, 0, 1, 1, 2, 4, 0 };
constexpr int TotalPhase = 4 * 1 + 4 * 1 + 4 * 2 + 2 * 4; // 24

// Piece Square Tables (White's perspective, flip rank for Black)
const int PawnPST_MG[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    5,  10,  10, -20, -20,  10,  10,   5,
    5,  -5, -10,   0,   0, -10,  -5,   5,
    0,   0,   0,  20,  20,   0,   0,   0,
    5,   5,  10,  25,  25,  10,   5,   5,
   10,  10,  20,  30,  30,  20,  10,  10,
   50,  50,  50,  50,  50,  50,  50,  50,
    0,   0,   0,   0,   0,   0,   0,   0
};

const int PawnPST_EG[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
   10,  10,  10,  10,  10,  10,  10,  10,
   20,  20,  20,  20,  20,  20,  20,  20,
   40,  40,  40,  40,  40,  40,  40,  40,
   70,  70,  70,  70,  70,  70,  70,  70,
  100, 100, 100, 100, 100, 100, 100, 100,
    0,   0,   0,   0,   0,   0,   0,   0
};

const int KnightPST[64] = {
  -50, -40, -30, -30, -30, -30, -40, -50,
  -40, -20,   0,   5,   5,   0, -20, -40,
  -30,   5,  15,  20,  20,  15,   5, -30,
  -30,   0,  15,  25,  25,  15,   0, -30,
  -30,   5,  15,  25,  25,  15,   5, -30,
  -30,   0,  10,  15,  15,  10,   0, -30,
  -40, -20,   0,   0,   0,   0, -20, -40,
  -50, -40, -30, -30, -30, -30, -40, -50
};

const int BishopPST[64] = {
  -20, -10, -10, -10, -10, -10, -10, -20,
  -10,   5,   0,   0,   0,   0,   5, -10,
  -10,  10,  10,  10,  10,  10,  10, -10,
  -10,   0,  10,  15,  15,  10,   0, -10,
  -10,   5,   5,  10,  10,   5,   5, -10,
  -10,   0,   5,  10,  10,   5,   0, -10,
  -10,   0,   0,   0,   0,   0,   0, -10,
  -20, -10, -10, -10, -10, -10, -10, -20
};

const int RookPST[64] = {
    0,   0,   0,   5,   5,   0,   0,   0,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
    5,  10,  10,  10,  10,  10,  10,   5,
    0,   0,   0,   0,   0,   0,   0,   0
};

const int QueenPST[64] = {
  -20, -10, -10,  -5,  -5, -10, -10, -20,
  -10,   0,   5,   0,   0,   0,   0, -10,
  -10,   5,   5,   5,   5,   5,   0, -10,
    0,   0,   5,   5,   5,   5,   0,  -5,
   -5,   0,   5,   5,   5,   5,   0,  -5,
  -10,   0,   5,   5,   5,   5,   0, -10,
  -10,   0,   0,   0,   0,   0,   0, -10,
  -20, -10, -10,  -5,  -5, -10, -10, -20
};

const int KingPST_MG[64] = {
   20,  30,  10,   0,   0,  10,  30,  20,
   20,  20,   0,   0,   0,   0,  20,  20,
  -10, -20, -20, -20, -20, -20, -20, -10,
  -20, -30, -30, -40, -40, -30, -30, -20,
  -30, -40, -40, -50, -50, -40, -40, -30,
  -30, -40, -40, -50, -50, -40, -40, -30,
  -30, -40, -40, -50, -50, -40, -40, -30,
  -30, -40, -40, -50, -50, -40, -40, -30
};

const int KingPST_EG[64] = {
  -50, -30, -30, -30, -30, -30, -30, -50,
  -30, -10,   0,   0,   0,   0, -10, -30,
  -30,   0,  20,  20,  20,  20,   0, -30,
  -30,   0,  20,  40,  40,  20,   0, -30,
  -30,   0,  20,  40,  40,  20,   0, -30,
  -30,   0,  20,  20,  20,  20,   0, -30,
  -30, -10,   0,   0,   0,   0, -10, -30,
  -50, -30, -30, -30, -30, -30, -30, -50
};

inline Square relative_square(Color c, Square s) {
    return (c == WHITE) ? s : static_cast<Square>(s ^ 56);
}

} // namespace

void init_evaluation() {}

Value evaluate(const Position& pos) {
    int mg[COLOR_NB] = {0, 0};
    int eg[COLOR_NB] = {0, 0};
    int game_phase = 0;

    for (Color c : {WHITE, BLACK}) {
        Bitboard pawns = pos.pieces(c, PAWN);
        while (pawns) {
            Square s = pop_lsb(pawns);
            Square rel_s = relative_square(c, s);
            mg[c] += PieceValueMG[PAWN] + PawnPST_MG[rel_s];
            eg[c] += PieceValueEG[PAWN] + PawnPST_EG[rel_s];
        }

        Bitboard knights = pos.pieces(c, KNIGHT);
        while (knights) {
            Square s = pop_lsb(knights);
            Square rel_s = relative_square(c, s);
            mg[c] += PieceValueMG[KNIGHT] + KnightPST[rel_s];
            eg[c] += PieceValueEG[KNIGHT] + KnightPST[rel_s];
            game_phase += PhaseWeight[KNIGHT];
        }

        Bitboard bishops = pos.pieces(c, BISHOP);
        while (bishops) {
            Square s = pop_lsb(bishops);
            Square rel_s = relative_square(c, s);
            mg[c] += PieceValueMG[BISHOP] + BishopPST[rel_s];
            eg[c] += PieceValueEG[BISHOP] + BishopPST[rel_s];
            game_phase += PhaseWeight[BISHOP];
        }

        // Bishop pair bonus
        if (popcount(pos.pieces(c, BISHOP)) >= 2) {
            mg[c] += 30;
            eg[c] += 45;
        }

        Bitboard rooks = pos.pieces(c, ROOK);
        while (rooks) {
            Square s = pop_lsb(rooks);
            Square rel_s = relative_square(c, s);
            mg[c] += PieceValueMG[ROOK] + RookPST[rel_s];
            eg[c] += PieceValueEG[ROOK] + RookPST[rel_s];
            game_phase += PhaseWeight[ROOK];
        }

        Bitboard queens = pos.pieces(c, QUEEN);
        while (queens) {
            Square s = pop_lsb(queens);
            Square rel_s = relative_square(c, s);
            mg[c] += PieceValueMG[QUEEN] + QueenPST[rel_s];
            eg[c] += PieceValueEG[QUEEN] + QueenPST[rel_s];
            game_phase += PhaseWeight[QUEEN];
        }

        Square ksq = pos.king_square(c);
        if (ksq != SQ_NONE) {
            Square rel_s = relative_square(c, ksq);
            mg[c] += KingPST_MG[rel_s];
            eg[c] += KingPST_EG[rel_s];
        }
    }

    if (game_phase > 24) game_phase = 24;
    int mg_score = mg[WHITE] - mg[BLACK];
    int eg_score = eg[WHITE] - eg[BLACK];

    int tapered_eval = ((mg_score * game_phase) + (eg_score * (24 - game_phase))) / 24;
    return (pos.side_to_move() == WHITE) ? tapered_eval : -tapered_eval;
}

} // namespace Andromeda
