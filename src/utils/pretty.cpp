#include <format>

#include "bits.h"
#include "pretty.h"

void printBitboard(Bitboard board, std::ostream& out) {

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            if (!f) out << std::format(" {:d}  ", 8 - r);

            int sq = r * 8 + f;
            out << std::format(" {:d}", board[(Square)(8 * r + f)], sq ? 1 : 0);
        }
        out << "\n";
    }
    out << "\n     a b c d e f g h\n\n";
    out << std::format("     Bitboard: {}\n\n", board.val());
}