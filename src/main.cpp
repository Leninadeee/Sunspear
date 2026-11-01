#include <iostream>

#include "attacks.h"
#include "bitboard.h"
#include "types.h"
#include "utils/util.h"

int main() {

    Bitboard a(1ULL << static_cast<int>(Square::a5));

    Bitboard occ{};
    occ |= Bitboard(1ULL << static_cast<int>(Square::e2));
    occ |= Bitboard(1ULL << static_cast<int>(Square::e7));
    occ |= Bitboard(1ULL << static_cast<int>(Square::a4));
    occ |= Bitboard(1ULL << static_cast<int>(Square::f4));


    util::printBitboard(attacks::generateOccupancyRookRays(Square::e4, occ));
    occ &= 0;

    occ |= Bitboard(1ULL << static_cast<int>(Square::f5));
    occ |= Bitboard(1ULL << static_cast<int>(Square::c2));
    occ |= Bitboard(1ULL << static_cast<int>(Square::b7));
    occ |= Bitboard(1ULL << static_cast<int>(Square::h1));


    util::printBitboard(attacks::generateOccupancyBishopRays(Square::a1, occ));

    
    util::printBitboard(Bitboard(770));
}