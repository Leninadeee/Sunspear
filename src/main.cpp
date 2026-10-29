#include <iostream>

#include "attacks.h"
#include "bitboard.h"
#include "types.h"
#include "utils/utils.h"

int main() {

    Bitboard a(1ULL << static_cast<int>(Square::a5) << 7ULL);
    //Bitboard b(1ULL << static_cast<int>(Square::a2));

    Bitboard b1 = attacks::genKingAttackSingular(Square::h8);
    printBitboard(b1);

    Bitboard c = attacks::genKingAttackSingular(Square::h7);
    printBitboard(c);

    Bitboard d = attacks::genKingAttackSingular(Square::h2);
    printBitboard(d);

    Bitboard e = attacks::genKingAttackSingular(Square::h1);
    printBitboard(e);
}