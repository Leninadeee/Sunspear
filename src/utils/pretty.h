#pragma once

#include <iostream>

#include "../bitboard.h"

class Position;

namespace util {
    void printBitboard(Bitboard board, std::ostream& out = std::cout);
    void prettyBoard(const Position& board, std::ostream& out = std::cout);
}
