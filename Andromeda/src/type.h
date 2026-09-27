#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <algorithm>
#include <array>

namespace Andromeda {

using Bitboard = uint64_t;
using Depth = int32_t;
using Value = int32_t;
using Key = uint64_t;

constexpr Value VALUE_ZERO      = 0;
constexpr Value VALUE_DRAW      = 0;
constexpr Value VALUE_MATE      = 32000;
constexpr Value VALUE_INFINITE  = 32500;
constexpr Value VALUE_NONE      = 32600;
constexpr Value VALUE_MATE_IN_MAX_PLY = VALUE_MATE - 256;
constexpr Value VALUE_MATED_IN_MAX_PLY = -VALUE_MATE_IN_MAX_PLY;

constexpr Depth MAX_DEPTH = 128;
constexpr int MAX_PLY = 256;

enum Color : uint8_t {
    WHITE = 0,
    BLACK = 1,
    COLOR_NB = 2
};

inline constexpr Color operator~(Color c) {
    return static_cast<Color>(c ^ 1);
}

enum PieceType : uint8_t {
    NO_PIECE_TYPE = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,
    PIECE_TYPE_NB = 7
};

enum Piece : uint8_t {
    NO_PIECE = 0,
    W_PAWN = 1, W_KNIGHT = 2, W_BISHOP = 3, W_ROOK = 4, W_QUEEN = 5, W_KING = 6,
    B_PAWN = 9, B_KNIGHT = 10, B_BISHOP = 11, B_ROOK = 12, B_QUEEN = 13, B_KING = 14,
    PIECE_NB = 16
};

inline constexpr Piece make_piece(Color c, PieceType pt) {
    return static_cast<Piece>((c << 3) | pt);
}

inline constexpr PieceType type_of(Piece p) {
    return static_cast<PieceType>(p & 7);
}

inline constexpr Color color_of(Piece p) {
    return static_cast<Color>((p >> 3) & 1);
}

enum Square : int8_t {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_NONE = 64,
    SQUARE_NB = 64
};

enum File : uint8_t {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NB
};

enum Rank : uint8_t {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB
};

inline constexpr Square make_square(File f, Rank r) {
    return static_cast<Square>((r << 3) | f);
}

inline constexpr File file_of(Square s) {
    return static_cast<File>(s & 7);
}

inline constexpr Rank rank_of(Square s) {
    return static_cast<Rank>(s >> 3);
}

enum MoveType : uint16_t {
    NORMAL = 0,
    PROMOTION = 1 << 14,
    EN_PASSANT = 2 << 14,
    CASTLING = 3 << 14
};

enum CastlingRights : uint8_t {
    NO_CASTLING = 0,
    WHITE_OO = 1,
    WHITE_OOO = 2,
    BLACK_OO = 4,
    BLACK_OOO = 8,
    WHITE_CASTLING = WHITE_OO | WHITE_OOO,
    BLACK_CASTLING = BLACK_OO | BLACK_OOO,
    ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING
};

class Move {
public:
    constexpr Move() : data_(0) {}
    constexpr explicit Move(uint16_t d) : data_(d) {}
    constexpr Move(Square from, Square to, MoveType type = NORMAL, PieceType pt = KNIGHT) {
        data_ = static_cast<uint16_t>(from | (to << 6) | type | ((pt - KNIGHT) << 12));
    }

    constexpr Square from() const { return static_cast<Square>(data_ & 0x3F); }
    constexpr Square to() const { return static_cast<Square>((data_ >> 6) & 0x3F); }
    constexpr MoveType type() const { return static_cast<MoveType>(data_ & (3 << 14)); }
    constexpr PieceType promotion_type() const { return static_cast<PieceType>(((data_ >> 12) & 3) + KNIGHT); }
    constexpr uint16_t raw() const { return data_; }
    constexpr bool is_none() const { return data_ == 0; }
    constexpr bool is_ok() const { return from() != to(); }

    constexpr bool operator==(const Move& m) const { return data_ == m.data_; }
    constexpr bool operator!=(const Move& m) const { return data_ != m.data_; }

    static constexpr Move none() { return Move(0); }
    static constexpr Move null() { return Move(0); }

private:
    uint16_t data_ = 0;
};

} // namespace Andromeda
