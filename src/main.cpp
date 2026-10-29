#include <iostream>

#include "bitboard.h"
#include "types.h"
#include "utils/utils.h"

int main() {

    Bitboard a(1ULL << 2);
    Bitboard b(1ULL << 63);

    printBitboard(a);
    printBitboard(b);
}