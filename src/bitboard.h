#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

namespace shifts {

    constexpr int Vertical = 8;
    constexpr int Horizontal = 1;

    // Relative to pawn pushing direction
    constexpr int DiagonalLR = Vertical - Horizontal;
    constexpr int DiagonalRL = Vertical + Horizontal;

    // Relative to pawn pushing direction
    constexpr int DiagonalL1R2 = Vertical + Vertical - Horizontal;
    constexpr int DiagonalR1L2 = Vertical + Vertical + Horizontal;
    constexpr int DiagonalL2R1 = Vertical - Horizontal - Horizontal;
    constexpr int DiagonalR2L1 = Vertical + Horizontal + Horizontal;

};

class Bitboard {
    public:
        constexpr Bitboard() = default;
        constexpr Bitboard(u64 u) : m_board(u) {};

        constexpr u64 val() const { return m_board; }

        constexpr Bitboard operator&(u64 rhs) const { return m_board & rhs; }
        constexpr Bitboard operator|(u64 rhs) const { return m_board | rhs; }
		constexpr Bitboard operator^(u64 rhs) const { return m_board ^ rhs; }

        constexpr Bitboard operator&(Bitboard rhs) const { return m_board & rhs.m_board; }
        constexpr Bitboard operator|(Bitboard rhs) const { return m_board | rhs.m_board; }
		constexpr Bitboard operator^(Bitboard rhs) const { return m_board ^ rhs.m_board; }

        constexpr Bitboard& operator|=(u64 rhs) { m_board |= rhs; return *this; }

        constexpr Bitboard& operator|=(Bitboard rhs) { m_board |= rhs.m_board; return *this; }

        constexpr Bitboard operator~() const { return ~m_board; }

        constexpr Bitboard operator<<(int l) const { return m_board << l; }
        constexpr Bitboard operator>>(int r) const { return m_board >> r; }

        constexpr bool operator[](Square s) const { return m_board & (1ULL << static_cast<int>(s)); }

        constexpr static Bitboard point(Square sq) { return (1ULL << static_cast<int>(sq)); }

    private:
        u64 m_board;
};

#endif