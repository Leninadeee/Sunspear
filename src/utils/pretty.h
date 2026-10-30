#ifndef PRETTY_H
#define PRETTY_H

#include <iostream>

#include "../bitboard.h"

namespace util {
    void printBitboard(Bitboard board, std::ostream& out = std::cout);
    void prettyBitboard(Bitboard board, std::ostream& out = std::cout);
}

#endif
