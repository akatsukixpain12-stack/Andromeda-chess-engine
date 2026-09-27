#include "position.h"
#include <sstream>
#include <cstring>

namespace Andromeda {

Key ZobristPieces[PIECE_NB][SQUARE_NB];
Key ZobristSide;
Key ZobristCastling[16];
Key ZobristEp[8];

namespace {
    uint64_t rand64(uint64_t& state) {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return state * 0x2545F4914F6CDD1DULL;
    }
}

void init_zobrist() {
    uint64_t state = 1070372;
    for (int p = 0; p < PIECE_NB; ++p) {
        for (int s = 0; s < SQUARE_NB; ++s) {
            ZobristPieces[p][s] = rand64(state);
        }
    }
    ZobristSide = rand64(state);
    for (int i = 0; i < 16; ++i) {
        ZobristCastling[i] = rand64(state);
    }
    for (int i = 0; i < 8; ++i) {
        ZobristEp[i] = rand64(state);
    }
}

Position::Position() : side_to_move_(WHITE), game_ply_(0), state_(nullptr) {
    clear();
}

void Position::clear() {
    std::fill(std::begin(board_), std::end(board_), NO_PIECE);
    std::fill(std::begin(by_type_bb_), std::end(by_type_bb_), 0);
    std::fill(std::begin(by_color_bb_), std::end(by_color_bb_), 0);
    side_to_move_ = WHITE;
    game_ply_ = 0;
    state_ = nullptr;
}

void Position::put_piece(Piece p, Square s) {
    board_[s] = p;
    Bitboard b = square_bb(s);
    by_type_bb_[type_of(p)] |= b;
    by_color_bb_[color_of(p)] |= b;
}

void Position::remove_piece(Square s) {
    Piece p = board_[s];
    board_[s] = NO_PIECE;
    Bitboard b = ~square_bb(s);
    by_type_bb_[type_of(p)] &= b;
    by_color_bb_[color_of(p)] &= b;
}

void Position::move_piece(Square from, Square to) {
    Piece p = board_[from];
    board_[from] = NO_PIECE;
    board_[to] = p;
    Bitboard b = square_bb(from) | square_bb(to);
    by_type_bb_[type_of(p)] ^= b;
    by_color_bb_[color_of(p)] ^= b;
}

void Position::set_startpos(StateInfo* si) {
    set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", si);
}

void Position::set(const std::string& fen, StateInfo* si) {
    clear();
    state_ = si;
    std::memset(state_, 0, sizeof(StateInfo));

    std::istringstream ss(fen);
    std::string pieces, side, castling, ep;
    int halfmove = 0, fullmove = 1;

    ss >> pieces >> side >> castling >> ep >> halfmove >> fullmove;

    int r = 7, f = 0;
    for (char c : pieces) {
        if (c == '/') {
            --r;
            f = 0;
        } else if (c >= '1' && c <= '8') {
            f += c - '0';
        } else {
            Square sq = make_square(static_cast<File>(f), static_cast<Rank>(r));
            Color color = (c >= 'a' && c <= 'z') ? BLACK : WHITE;
            char lc = std::tolower(c);
            PieceType pt = NO_PIECE_TYPE;
            if (lc == 'p') pt = PAWN;
            else if (lc == 'n') pt = KNIGHT;
            else if (lc == 'b') pt = BISHOP;
            else if (lc == 'r') pt = ROOK;
            else if (lc == 'q') pt = QUEEN;
            else if (lc == 'k') pt = KING;

            if (pt != NO_PIECE_TYPE) {
                put_piece(make_piece(color, pt), sq);
            }
            ++f;
        }
    }

    side_to_move_ = (side == "w" || side.empty()) ? WHITE : BLACK;

    state_->castling_rights = NO_CASTLING;
    if (castling.find('K') != std::string::npos) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights | WHITE_OO);
    if (castling.find('Q') != std::string::npos) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights | WHITE_OOO);
    if (castling.find('k') != std::string::npos) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights | BLACK_OO);
    if (castling.find('q') != std::string::npos) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights | BLACK_OOO);

    state_->ep_square = SQ_NONE;
    if (ep.size() == 2 && ep[0] >= 'a' && ep[0] <= 'h') {
        state_->ep_square = make_square(static_cast<File>(ep[0] - 'a'), static_cast<Rank>(ep[1] - '1'));
    }

    state_->halfmove_clock = halfmove;
    game_ply_ = (fullmove - 1) * 2 + (side_to_move_ == BLACK ? 1 : 0);

    // Compute key
    Key k = 0;
    for (int s = 0; s < 64; ++s) {
        Piece p = board_[s];
        if (p != NO_PIECE) k ^= ZobristPieces[p][s];
    }
    if (side_to_move_ == BLACK) k ^= ZobristSide;
    k ^= ZobristCastling[state_->castling_rights];
    if (state_->ep_square != SQ_NONE) k ^= ZobristEp[file_of(state_->ep_square)];

    state_->key = k;
}

std::string Position::fen() const {
    std::string res;
    for (int r = 7; r >= 0; --r) {
        int empty = 0;
        for (int f = 0; f < 8; ++f) {
            Piece p = board_[make_square(static_cast<File>(f), static_cast<Rank>(r))];
            if (p == NO_PIECE) {
                ++empty;
            } else {
                if (empty > 0) {
                    res += std::to_string(empty);
                    empty = 0;
                }
                char c = '?';
                PieceType pt = type_of(p);
                if (pt == PAWN) c = 'p';
                else if (pt == KNIGHT) c = 'n';
                else if (pt == BISHOP) c = 'b';
                else if (pt == ROOK) c = 'r';
                else if (pt == QUEEN) c = 'q';
                else if (pt == KING) c = 'k';
                if (color_of(p) == WHITE) c = std::toupper(c);
                res += c;
            }
        }
        if (empty > 0) res += std::to_string(empty);
        if (r > 0) res += '/';
    }

    res += (side_to_move_ == WHITE ? " w " : " b ");
    std::string cast;
    if (state_->castling_rights & WHITE_OO) cast += 'K';
    if (state_->castling_rights & WHITE_OOO) cast += 'Q';
    if (state_->castling_rights & BLACK_OO) cast += 'k';
    if (state_->castling_rights & BLACK_OOO) cast += 'q';
    res += cast.empty() ? "-" : cast;

    res += " ";
    if (state_->ep_square != SQ_NONE) {
        res += static_cast<char>('a' + file_of(state_->ep_square));
        res += static_cast<char>('1' + rank_of(state_->ep_square));
    } else {
        res += "-";
    }

    res += " " + std::to_string(state_->halfmove_clock);
    res += " " + std::to_string(game_ply_ / 2 + 1);
    return res;
}

Bitboard Position::attackers_to(Square s, Bitboard occ) const {
    return (PawnAttacks[BLACK][s] & pieces(WHITE, PAWN))
         | (PawnAttacks[WHITE][s] & pieces(BLACK, PAWN))
         | (KnightAttacks[s] & pieces(KNIGHT))
         | (bishop_attacks(s, occ) & (pieces(BISHOP) | pieces(QUEEN)))
         | (rook_attacks(s, occ) & (pieces(ROOK) | pieces(QUEEN)))
         | (KingAttacks[s] & pieces(KING));
}

bool Position::in_check() const {
    Square ksq = king_square(side_to_move_);
    if (ksq == SQ_NONE) return false;
    return attackers_to(ksq, pieces()) & pieces(~side_to_move_);
}

bool Position::is_capture(Move m) const {
    return board_[m.to()] != NO_PIECE || m.type() == EN_PASSANT;
}

bool Position::do_move(Move m, StateInfo& new_si) {
    new_si = *state_;
    new_si.previous = state_;
    state_ = &new_si;

    Square from = m.from();
    Square to = m.to();
    MoveType type = m.type();
    Piece piece = board_[from];
    Piece captured = board_[to];
    Color us = side_to_move_;
    Color them = ~us;

    state_->captured_piece = captured;
    state_->key ^= ZobristSide;
    if (state_->ep_square != SQ_NONE) {
        state_->key ^= ZobristEp[file_of(state_->ep_square)];
        state_->ep_square = SQ_NONE;
    }

    state_->halfmove_clock++;
    if (type_of(piece) == PAWN || captured != NO_PIECE) {
        state_->halfmove_clock = 0;
    }

    if (type == CASTLING) {
        move_piece(from, to);
        state_->key ^= ZobristPieces[piece][from] ^ ZobristPieces[piece][to];
        Square rfrom = SQ_NONE, rto = SQ_NONE;
        if (to > from) { // King side
            rfrom = make_square(FILE_H, rank_of(from));
            rto = make_square(FILE_F, rank_of(from));
        } else {
            rfrom = make_square(FILE_A, rank_of(from));
            rto = make_square(FILE_D, rank_of(from));
        }
        Piece r = board_[rfrom];
        move_piece(rfrom, rto);
        state_->key ^= ZobristPieces[r][rfrom] ^ ZobristPieces[r][rto];
    } else if (type == EN_PASSANT) {
        move_piece(from, to);
        state_->key ^= ZobristPieces[piece][from] ^ ZobristPieces[piece][to];
        Square cap_sq = make_square(file_of(to), rank_of(from));
        Piece cap_p = board_[cap_sq];
        remove_piece(cap_sq);
        state_->key ^= ZobristPieces[cap_p][cap_sq];
    } else if (type == PROMOTION) {
        remove_piece(from);
        if (captured != NO_PIECE) {
            remove_piece(to);
            state_->key ^= ZobristPieces[captured][to];
        }
        Piece promoted = make_piece(us, m.promotion_type());
        put_piece(promoted, to);
        state_->key ^= ZobristPieces[piece][from] ^ ZobristPieces[promoted][to];
    } else {
        if (captured != NO_PIECE) {
            remove_piece(to);
            state_->key ^= ZobristPieces[captured][to];
        }
        move_piece(from, to);
        state_->key ^= ZobristPieces[piece][from] ^ ZobristPieces[piece][to];

        if (type_of(piece) == PAWN && std::abs(rank_of(to) - rank_of(from)) == 2) {
            Square ep = make_square(file_of(from), static_cast<Rank>((rank_of(from) + rank_of(to)) / 2));
            state_->ep_square = ep;
            state_->key ^= ZobristEp[file_of(ep)];
        }
    }

    // Castling updates
    state_->key ^= ZobristCastling[state_->castling_rights];
    if (from == SQ_E1 || to == SQ_E1) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights & ~WHITE_CASTLING);
    if (from == SQ_E8 || to == SQ_E8) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights & ~BLACK_CASTLING);
    if (from == SQ_A1 || to == SQ_A1) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights & ~WHITE_OOO);
    if (from == SQ_H1 || to == SQ_H1) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights & ~WHITE_OO);
    if (from == SQ_A8 || to == SQ_A8) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights & ~BLACK_OOO);
    if (from == SQ_H8 || to == SQ_H8) state_->castling_rights = static_cast<CastlingRights>(state_->castling_rights & ~BLACK_OO);
    state_->key ^= ZobristCastling[state_->castling_rights];

    side_to_move_ = them;
    game_ply_++;

    // Check legality: us king cannot be in check
    if (attackers_to(king_square(us), pieces()) & pieces(them)) {
        undo_move(m);
        return false;
    }
    return true;
}

void Position::undo_move(Move m) {
    side_to_move_ = ~side_to_move_;
    game_ply_--;

    Square from = m.from();
    Square to = m.to();
    MoveType type = m.type();
    Color us = side_to_move_;

    if (type == CASTLING) {
        move_piece(to, from);
        Square rfrom = SQ_NONE, rto = SQ_NONE;
        if (to > from) {
            rfrom = make_square(FILE_H, rank_of(from));
            rto = make_square(FILE_F, rank_of(from));
        } else {
            rfrom = make_square(FILE_A, rank_of(from));
            rto = make_square(FILE_D, rank_of(from));
        }
        move_piece(rto, rfrom);
    } else if (type == EN_PASSANT) {
        move_piece(to, from);
        Square cap_sq = make_square(file_of(to), rank_of(from));
        put_piece(make_piece(~us, PAWN), cap_sq);
    } else if (type == PROMOTION) {
        remove_piece(to);
        put_piece(make_piece(us, PAWN), from);
        if (state_->captured_piece != NO_PIECE) {
            put_piece(state_->captured_piece, to);
        }
    } else {
        move_piece(to, from);
        if (state_->captured_piece != NO_PIECE) {
            put_piece(state_->captured_piece, to);
        }
    }

    state_ = state_->previous;
}

void Position::do_null_move(StateInfo& new_si) {
    new_si = *state_;
    new_si.previous = state_;
    state_ = &new_si;

    state_->key ^= ZobristSide;
    if (state_->ep_square != SQ_NONE) {
        state_->key ^= ZobristEp[file_of(state_->ep_square)];
        state_->ep_square = SQ_NONE;
    }

    side_to_move_ = ~side_to_move_;
    game_ply_++;
}

void Position::undo_null_move() {
    side_to_move_ = ~side_to_move_;
    game_ply_--;
    state_ = state_->previous;
}

bool Position::is_draw(int ply) const {
    if (state_->halfmove_clock >= 100) return true;
    
    // Repetition check
    int count = 0;
    StateInfo* curr = state_->previous;
    while (curr && curr != state_->previous->previous) {
        if (curr->key == state_->key) {
            ++count;
            if (count >= 1) return true;
        }
        curr = curr->previous;
    }
    return false;
}

} // namespace Andromeda
