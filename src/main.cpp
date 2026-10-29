#include <iostream>

#include "bitboard.h"
#include "types.h"
#include "utils/utils.h"

int main() {

    Bitboard a(1ULL << static_cast<int>(Square::e4));
    //Bitboard b(1ULL << static_cast<int>(Square::a2));

    Bitboard b1 = a >> shifts::DiagonalL2R1; //g3
    Bitboard b2 = a >> shifts::DiagonalL1R2; //f2
    Bitboard b3 = a >> shifts::DiagonalR2L1; //c3
    Bitboard b4 = a >> shifts::DiagonalR1L2; //d2

    printBitboard(a);
    printBitboard(b1);
    printBitboard(b2);
    printBitboard(b3);
    printBitboard(b4);
}