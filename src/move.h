// 16 bit encoding
#pragma once

#include "types.h"
#include "position.h"
#include "util.h"

enum class MoveFlag : u8 {
    Quiet               = 0,
    DoublePawnPush      = 1,
    KingCastle          = 2,
    QueenCastle         = 3,
    Capture             = 4,
    EnPassant           = 5,
    KnightPromo         = 8,
    BishopPromo         = 9,
    RookPromo           = 10,
    QueenPromo          = 11,
    KnightPromoCapture  = 12,
    BishopPromoCapture  = 13,
    RookPromoCapture    = 14,
    QueenPromoCapture   = 15,
};

// [0.....5] [6....10] [11...15]
//    src       dst      type

class Move {
public:
    constexpr Move() : m_data(0) {}
    constexpr explicit Move(u16 raw) : m_data(raw) {}
    constexpr Move(Square from, Square to, MoveFlag flag = MoveFlag::Quiet)
        : m_data(static_cast<u16>(static_cast<u16>(from) |
                 (static_cast<u16>(to) << 6) |
                 (static_cast<u16>(flag) << 12))) {}

    constexpr Square from() const {
        return static_cast<Square>(m_data & 0x3F);
    }

    constexpr Square to() const {
        return static_cast<Square>((m_data >> 6) & 0x3F);
    }

    constexpr MoveFlag flag() const {
        return static_cast<MoveFlag>((m_data >> 12) & 0x0F);
    }

    constexpr u16 raw() const { return m_data; }

    constexpr bool operator==(const Move& other) const = default;
    constexpr bool operator!=(const Move& other) const = default;
    constexpr explicit operator bool() const { return m_data != 0; }

private:
    u16 m_data{0};
};
