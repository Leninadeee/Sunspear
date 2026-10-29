#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

class Bitboard {
    public:
        Bitboard() = default;
        Bitboard(u64 u) : m_board(u) {};

        constexpr u64 val() const { return m_board; }
        constexpr bool operator[](Square s) const { return m_board & (1ULL << static_cast<int>(s)); }

    private:
        u64 m_board;
};

#endif