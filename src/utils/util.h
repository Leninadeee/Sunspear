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

#endif