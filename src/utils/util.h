#ifndef UTIL_H
#define UTIL_H

#include "bits.h"
#include "pretty.h"

constexpr int squareRank(Square sq) noexcept {
    return static_cast<int>(sq) >> 3;
}

constexpr int squareFile(Square sq) noexcept {
    return static_cast<int>(sq) & 7;
}

constexpr Color operator~(Color c) noexcept {
    return static_cast<Color>(static_cast<u8>(c) ^ 1);
}

#endif