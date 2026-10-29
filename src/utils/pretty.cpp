#include <format>

#include "bits.h"
#include "pretty.h"

void printBitboard(Bitboard board, std::ostream& out) {

    for (int r = 7; r >= 0; r--) {
        for (int f = 0; f < 8; f++) {
            if (!f) out << std::format(" {:d}  ", r + 1);

            int sq = r * 8 + f;
            out << std::format(" {:d}", board[(Square)(sq)] ? 1 : 0);
        }
        out << "\n";
    }
    out << "\n     a b c d e f g h\n\n";
    out << std::format("     Bitboard: {}\n\n", board.val());

}