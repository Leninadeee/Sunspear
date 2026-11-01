#ifndef BITBOARD_H
#define BITBOARD_H

#include "utils/bits.h"
#include "types.h"

namespace offsets {

    constexpr int Up = 8;
    constexpr int Down = -8;
    constexpr int Left = -1;
    constexpr int Right = 1;

    constexpr int UpLeft = Up + Left;
    constexpr int UpRight = Up + Right;
    constexpr int DownLeft = Down + Left;
    constexpr int DownRight = Down + Right;

}

namespace shifts {

    constexpr int Vertical = 8;
    constexpr int Horizontal = 1;

    // Relative to vertical direction
    constexpr int DiagonalLR = Vertical - Horizontal;
    constexpr int DiagonalRL = Vertical + Horizontal;

    // Relative to vertical direction
    constexpr int DiagonalL1R2 = Vertical + Vertical - Horizontal;
    constexpr int DiagonalR1L2 = Vertical + Vertical + Horizontal;
    constexpr int DiagonalL2R1 = Vertical - Horizontal - Horizontal;
    constexpr int DiagonalR2L1 = Vertical + Horizontal + Horizontal;

};

namespace boards {

        constexpr u64 Rank1{0x00000000000000FFULL};
		constexpr u64 Rank2{0x000000000000FF00ULL};
		constexpr u64 Rank3{0x0000000000FF0000ULL};
		constexpr u64 Rank4{0x00000000FF000000ULL};
		constexpr u64 Rank5{0x000000FF00000000ULL};
		constexpr u64 Rank6{0x0000FF0000000000ULL};
		constexpr u64 Rank7{0x00FF000000000000ULL};
		constexpr u64 Rank8{0xFF00000000000000ULL};

		constexpr u64 FileA{0x0101010101010101ULL};
		constexpr u64 FileB{0x0202020202020202ULL};
		constexpr u64 FileC{0x0404040404040404ULL};
		constexpr u64 FileD{0x0808080808080808ULL};
		constexpr u64 FileE{0x1010101010101010ULL};
		constexpr u64 FileF{0x2020202020202020ULL};
		constexpr u64 FileG{0x4040404040404040ULL};
		constexpr u64 FileH{0x8080808080808080ULL};

        constexpr u64 Fill {0xFFFFFFFFFFFFFFFFULL};

        constexpr u64 NotA  = Fill ^ FileA;
        constexpr u64 NotH  = Fill ^ FileH;
        constexpr u64 NotAB = Fill ^ (FileA | FileB);
        constexpr u64 NotGH = Fill ^ (FileH | FileG);

}

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

        constexpr Bitboard& operator&=(u64 rhs) { m_board &= rhs; return *this; }
        constexpr Bitboard& operator|=(u64 rhs) { m_board |= rhs; return *this; }

        constexpr Bitboard& operator|=(Bitboard rhs) { m_board |= rhs.m_board; return *this; }

        constexpr Bitboard operator~() const { return ~m_board; }

        constexpr Bitboard operator<<(int rhs) const { return m_board << rhs; }
        constexpr Bitboard operator>>(int rhs) const { return m_board >> rhs; }

        constexpr Bitboard& operator<<=(int rhs) { m_board <<= rhs; return *this; }
        constexpr Bitboard& operator>>=(int rhs) { m_board >>= rhs; return *this;}

        constexpr bool operator[](Square s) const { return m_board & (1ULL << static_cast<int>(s)); }

        constexpr operator auto() const { return m_board; }
        constexpr explicit operator bool() const { return m_board != 0; }

        constexpr bool isEmpty() const { return m_board == 0; }

        constexpr int popcount() const { return util::popcount(m_board); }
        //constexpr int lsb() const { return util::lsb(m_board); }
        constexpr static Bitboard point(Square sq) { return (1ULL << static_cast<int>(sq)); }

        constexpr static Bitboard border(int dir) {

			switch (dir) {
                case offsets::Up:        return boards::Rank8;
                case offsets::Down:      return boards::Rank1;
                case offsets::Left:      return boards::FileA;
                case offsets::Right:     return boards::FileH;
                case offsets::UpLeft:    return boards::FileA | boards::Rank8;
                case offsets::UpRight:   return boards::FileH | boards::Rank8;
                case offsets::DownLeft:  return boards::FileA | boards::Rank1;
                case offsets::DownRight: return boards::FileH | boards::Rank1;
                default: __builtin_unreachable();
			}
		}

        constexpr static Bitboard fillRay(Square src, int dir, Bitboard occ = 0) {

            Bitboard ray{};

            const bool right = (dir < 0);   // shift right if so
            const u8 shift = static_cast<u8>(dir < 0 ? -dir : dir); // get shift magnitude
            Bitboard blockers = Bitboard::border(dir);
            blockers |= occ;

            Bitboard singular = Bitboard::point(src);

            while (!(singular & blockers)) {
                if (right) ray |= (singular >>= shift);
                else       ray |= (singular <<= shift);
            }

            return ray;
        }

        //constexpr static Bitboard fillRay(Square src, int dir) {
        //    return fillRay(src, dir, 0);
        //}

    private:
        u64 m_board;
};

#endif