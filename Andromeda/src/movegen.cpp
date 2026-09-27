#include "movegen.h"

namespace Andromeda {

template<GenType Type>
Move* generate_moves(const Position& pos, Move* move_list) {
    Color us = pos.side_to_move();
    Color them = ~us;
    Bitboard us_bb = pos.pieces(us);
    Bitboard them_bb = pos.pieces(them);
    Bitboard occ = us_bb | them_bb;
    Bitboard empty = ~occ;
    Bitboard targets = 0;

    if constexpr (Type == GEN_CAPTURES) {
        targets = them_bb;
    } else if constexpr (Type == GEN_QUIETS) {
        targets = empty;
    } else {
        targets = ~us_bb;
    }

    // Pawn Moves
    Bitboard pawns = pos.pieces(us, PAWN);
    Rank promo_rank = (us == WHITE) ? RANK_8 : RANK_1;
    Rank start_rank = (us == WHITE) ? RANK_2 : RANK_7;
    int pawn_push = (us == WHITE) ? 8 : -8;

    while (pawns) {
        Square from = pop_lsb(pawns);
        Square single_push = static_cast<Square>(from + pawn_push);

        if (single_push >= 0 && single_push < 64 && !(occ & square_bb(single_push))) {
            if (rank_of(single_push) == promo_rank) {
                if constexpr (Type != GEN_QUIETS) {
                    *move_list++ = Move(from, single_push, PROMOTION, QUEEN);
                    *move_list++ = Move(from, single_push, PROMOTION, KNIGHT);
                    *move_list++ = Move(from, single_push, PROMOTION, ROOK);
                    *move_list++ = Move(from, single_push, PROMOTION, BISHOP);
                }
            } else {
                if constexpr (Type != GEN_CAPTURES) {
                    *move_list++ = Move(from, single_push, NORMAL);
                    if (rank_of(from) == start_rank) {
                        Square double_push = static_cast<Square>(from + 2 * pawn_push);
                        if (!(occ & square_bb(double_push))) {
                            *move_list++ = Move(from, double_push, NORMAL);
                        }
                    }
                }
            }
        }

        // Captures
        Bitboard attacks = PawnAttacks[us][from];
        Bitboard pawn_caps = attacks & them_bb;
        while (pawn_caps) {
            Square to = pop_lsb(pawn_caps);
            if (rank_of(to) == promo_rank) {
                *move_list++ = Move(from, to, PROMOTION, QUEEN);
                *move_list++ = Move(from, to, PROMOTION, KNIGHT);
                *move_list++ = Move(from, to, PROMOTION, ROOK);
                *move_list++ = Move(from, to, PROMOTION, BISHOP);
            } else if constexpr (Type != GEN_QUIETS) {
                *move_list++ = Move(from, to, NORMAL);
            }
        }

        // En passant
        if (pos.ep_square() != SQ_NONE && (attacks & square_bb(pos.ep_square()))) {
            if constexpr (Type != GEN_QUIETS) {
                *move_list++ = Move(from, pos.ep_square(), EN_PASSANT);
            }
        }
    }

    // Knights
    Bitboard knights = pos.pieces(us, KNIGHT);
    while (knights) {
        Square from = pop_lsb(knights);
        Bitboard att = KnightAttacks[from] & targets;
        while (att) {
            *move_list++ = Move(from, pop_lsb(att), NORMAL);
        }
    }

    // Bishops
    Bitboard bishops = pos.pieces(us, BISHOP);
    while (bishops) {
        Square from = pop_lsb(bishops);
        Bitboard att = bishop_attacks(from, occ) & targets;
        while (att) {
            *move_list++ = Move(from, pop_lsb(att), NORMAL);
        }
    }

    // Rooks
    Bitboard rooks = pos.pieces(us, ROOK);
    while (rooks) {
        Square from = pop_lsb(rooks);
        Bitboard att = rook_attacks(from, occ) & targets;
        while (att) {
            *move_list++ = Move(from, pop_lsb(att), NORMAL);
        }
    }

    // Queens
    Bitboard queens = pos.pieces(us, QUEEN);
    while (queens) {
        Square from = pop_lsb(queens);
        Bitboard att = queen_attacks(from, occ) & targets;
        while (att) {
            *move_list++ = Move(from, pop_lsb(att), NORMAL);
        }
    }

    // King
    Square ksq = pos.king_square(us);
    if (ksq != SQ_NONE) {
        Bitboard att = KingAttacks[ksq] & targets;
        while (att) {
            *move_list++ = Move(ksq, pop_lsb(att), NORMAL);
        }

        // Castling (quiets only or legal)
        if constexpr (Type != GEN_CAPTURES) {
            CastlingRights cr = pos.castling_rights();
            if (us == WHITE) {
                if ((cr & WHITE_OO) && !(occ & (square_bb(SQ_F1) | square_bb(SQ_G1)))) {
                    if (!pos.in_check() && !(pos.attackers_to(SQ_F1, occ) & them_bb)) {
                        *move_list++ = Move(SQ_E1, SQ_G1, CASTLING);
                    }
                }
                if ((cr & WHITE_OOO) && !(occ & (square_bb(SQ_D1) | square_bb(SQ_C1) | square_bb(SQ_B1)))) {
                    if (!pos.in_check() && !(pos.attackers_to(SQ_D1, occ) & them_bb)) {
                        *move_list++ = Move(SQ_E1, SQ_C1, CASTLING);
                    }
                }
            } else {
                if ((cr & BLACK_OO) && !(occ & (square_bb(SQ_F8) | square_bb(SQ_G8)))) {
                    if (!pos.in_check() && !(pos.attackers_to(SQ_F8, occ) & them_bb)) {
                        *move_list++ = Move(SQ_E8, SQ_G8, CASTLING);
                    }
                }
                if ((cr & BLACK_OOO) && !(occ & (square_bb(SQ_D8) | square_bb(SQ_C8) | square_bb(SQ_B8)))) {
                    if (!pos.in_check() && !(pos.attackers_to(SQ_D8, occ) & them_bb)) {
                        *move_list++ = Move(SQ_E8, SQ_C8, CASTLING);
                    }
                }
            }
        }
    }

    return move_list;
}

template Move* generate_moves<GEN_LEGAL>(const Position&, Move*);
template Move* generate_moves<GEN_CAPTURES>(const Position&, Move*);
template Move* generate_moves<GEN_QUIETS>(const Position&, Move*);

int generate_legal_moves(Position& pos, Move* move_list) {
    Move pseudo[256];
    Move* last = generate_moves<GEN_LEGAL>(pos, pseudo);
    int count = 0;
    StateInfo si;
    for (Move* m = pseudo; m < last; ++m) {
        if (pos.do_move(*m, si)) {
            pos.undo_move(*m);
            move_list[count++] = *m;
        }
    }
    return count;
}

} // namespace Andromeda
