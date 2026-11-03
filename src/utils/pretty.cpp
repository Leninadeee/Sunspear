#include <format>

#include "../position.h"
#include "bits.h"
#include "pretty.h"

void util::printBitboard(Bitboard board, std::ostream& out) {

    for (int rank = 7; rank >= 0; rank--) {

        for (int file = 0; file < 8; file++) {
    
            if (!file)
                out << std::format(" {:d}  ", rank + 1);

            out << std::format(" {:d}", board[makeSquare(rank, file)] ? 1 : 0);
        }
        out << "\n";
    }
    out << "\n     a b c d e f g h\n\n";
    out << std::format("     Bitboard: {}\n\n", board.val());
}

void util::prettyBoard(const Position &position, std::ostream &out) {

    for (int rank = 7; rank >= 0; rank--) {

        out << " +---+---+---+---+---+---+---+---+\n";

        for (int file = 0; file < 8; file++) {
            char c = ' ';
            switch (position.pieceAt(rank, file)) {
                case Piece::WhitePawn:   c = 'P'; break;
                case Piece::WhiteKnight: c = 'N'; break;
                case Piece::WhiteBishop: c = 'B'; break;
                case Piece::WhiteRook:   c = 'R'; break;
                case Piece::WhiteQueen:  c = 'Q'; break;
                case Piece::WhiteKing:   c = 'K'; break;
                case Piece::BlackPawn:   c = 'p'; break;
                case Piece::BlackKnight: c = 'n'; break;
                case Piece::BlackBishop: c = 'b'; break;
                case Piece::BlackRook:   c = 'r'; break;
                case Piece::BlackQueen:  c = 'q'; break;
                case Piece::BlackKing:   c = 'k'; break;
                case Piece::None:        c = ' '; break;
            }

            out << " | " << c;
        }

        out << " | " << (rank + 1) << "\n";
    }

    out << " +---+---+---+---+---+---+---+---+\n";
    out << "   a   b   c   d   e   f   g   h\n\n";

    out << (position.side() == Color::White ? "White" : "Black") << " to move\n";
}