#pragma once
#include "type.h"
#include <immintrin.h>

namespace Andromeda {

constexpr Bitboard ALL_SQUARES = ~0ULL;
constexpr Bitboard FILE_A_BB = 0x0101010101010101ULL;
constexpr Bitboard FILE_B_BB = FILE_A_BB << 1;
constexpr Bitboard FILE_C_BB = FILE_A_BB << 2;
constexpr Bitboard FILE_D_BB = FILE_A_BB << 3;
constexpr Bitboard FILE_E_BB = FILE_A_BB << 4;
constexpr Bitboard FILE_F_BB = FILE_A_BB << 5;
constexpr Bitboard FILE_G_BB = FILE_A_BB << 6;
constexpr Bitboard FILE_H_BB = FILE_A_BB << 7;

constexpr Bitboard RANK_1_BB = 0x00000000000000FFULL;
constexpr Bitboard RANK_2_BB = RANK_1_BB << 8;
constexpr Bitboard RANK_3_BB = RANK_1_BB << 16;
constexpr Bitboard RANK_4_BB = RANK_1_BB << 24;
constexpr Bitboard RANK_5_BB = RANK_1_BB << 32;
constexpr Bitboard RANK_6_BB = RANK_1_BB << 40;
constexpr Bitboard RANK_7_BB = RANK_1_BB << 48;
constexpr Bitboard RANK_8_BB = RANK_1_BB << 56;

inline constexpr Bitboard square_bb(Square s) {
    return 1ULL << s;
}

inline int popcount(Bitboard b) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(b);
#elif defined(_MSC_VER)
    return static_cast<int>(__popcnt64(b));
#else
    int count = 0;
    while (b) { count += b & 1; b >>= 1; }
    return count;
#endif
}

inline Square lsb(Bitboard b) {
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<Square>(__builtin_ctzll(b));
#elif defined(_MSC_VER)
    unsigned long idx;
    _BitScanForward64(&idx, b);
    return static_cast<Square>(idx);
#else
    for (int i = 0; i < 64; ++i) if ((b >> i) & 1) return static_cast<Square>(i);
    return SQ_NONE;
#endif
}

inline Square pop_lsb(Bitboard& b) {
    Square s = lsb(b);
    b &= b - 1;
    return s;
}

void init_bitboards();

} // namespace Andromeda
