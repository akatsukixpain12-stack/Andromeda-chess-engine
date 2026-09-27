#pragma once
#include "type.h"
#include "bitboard.h"
#include "attacks.h"
#include <string>
#include <vector>

namespace Andromeda {

struct StateInfo {
    Key key;
    Key pawn_key;
    Value non_pawn_material[COLOR_NB];
    CastlingRights castling_rights;
    Square ep_square;
    int halfmove_clock;
    int plies_from_null;
    Piece captured_piece;
    StateInfo* previous;
};

class Position {
public:
    Position();
    void set(const std::string& fen, StateInfo* si);
    void set_startpos(StateInfo* si);
    std::string fen() const;

    bool do_move(Move m, StateInfo& new_si);
    void undo_move(Move m);
    void do_null_move(StateInfo& new_si);
    void undo_null_move();

    Bitboard pieces(PieceType pt) const { return by_type_bb_[pt]; }
    Bitboard pieces(Color c) const { return by_color_bb_[c]; }
    Bitboard pieces(Color c, PieceType pt) const { return by_color_bb_[c] & by_type_bb_[pt]; }
    Bitboard pieces() const { return by_color_bb_[WHITE] | by_color_bb_[BLACK]; }
    Piece piece_on(Square s) const { return board_[s]; }

    Color side_to_move() const { return side_to_move_; }
    CastlingRights castling_rights() const { return state_->castling_rights; }
    Square ep_square() const { return state_->ep_square; }
    Key key() const { return state_->key; }
    int game_ply() const { return game_ply_; }
    int halfmove_clock() const { return state_->halfmove_clock; }
    Square king_square(Color c) const { return lsb(pieces(c, KING)); }

    bool in_check() const;
    bool gives_check(Move m) const;
    bool is_draw(int ply) const;
    bool is_legal(Move m) const;
    bool is_capture(Move m) const;

    Bitboard attackers_to(Square s, Bitboard occ) const;

private:
    void clear();
    void put_piece(Piece p, Square s);
    void remove_piece(Square s);
    void move_piece(Square from, Square to);

    Piece board_[SQUARE_NB];
    Bitboard by_type_bb_[PIECE_TYPE_NB];
    Bitboard by_color_bb_[COLOR_NB];
    Color side_to_move_;
    int game_ply_;
    StateInfo* state_;
};

extern Key ZobristPieces[PIECE_NB][SQUARE_NB];
extern Key ZobristSide;
extern Key ZobristCastling[16];
extern Key ZobristEp[8];

void init_zobrist();

} // namespace Andromeda
