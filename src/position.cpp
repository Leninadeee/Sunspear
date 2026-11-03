#include <sstream>

#include "position.h"

void Position::clear() {

    m_pieces.fill(Bitboard(0));
    m_occupancy.fill(Bitboard(0));
    m_board.fill(Piece::None);

    m_side = Color::White;
    m_enPassant = Square::none;
    m_castling = Castling::None;
    m_halfmove = 0;
    m_fullmove = 1;
}

void Position::setPiece(Square sq, Piece p) {

    const int intsq = static_cast<int>(sq);
    const int intpc = static_cast<int>(p);
    const int intcol = static_cast<int>(pieceColor(p));
    const Bitboard point = Bitboard::point(sq);

    m_pieces[intpc] |= point;
    m_occupancy[intcol] |= point;
    m_board[intsq] = p;
}

void Position::removePiece(Square sq) {

    const int intsq = static_cast<int>(sq);
    const Piece p = m_board[intsq];
    const int intpc = static_cast<int>(p);
    const int intcol = static_cast<int>(pieceColor(p));
    const Bitboard point = Bitboard::point(sq);

    m_pieces[intpc] &= ~point;
    m_occupancy[intcol] &= ~point;
    m_board[intsq] = Piece::None;
}

void Position::movePiece(Square src, Square dst) {

    const int intsrc = static_cast<int>(src);
    const int intdst = static_cast<int>(dst);
    const Piece p = m_board[intsrc];
    const int intpc = static_cast<int>(p);
    const int intcol = static_cast<int>(pieceColor(p));
    const Bitboard fromTo = Bitboard::point(src) | Bitboard::point(dst);

    m_pieces[intpc] ^= fromTo;
    m_occupancy[intcol] ^= fromTo;
    m_board[intsrc] = Piece::None;
    m_board[intdst] = p;
}

bool Position::setFen(const std::string& fen) {

    clear();

    std::string s(fen);
    std::istringstream ss(s);
    std::string piecePlacement, sideStr, castlingStr, epStr;
    int halfmove = 0;
    int fullmove = 1;

    if (!(ss >> piecePlacement >> sideStr)) {
        return false;
    }
    ss >> castlingStr >> epStr >> halfmove >> fullmove;

    int rank = 7;
    int file = 0;

    for (char c : piecePlacement) {

        if (c == '/') {
            rank--;
            file = 0;
            if (rank < 0) return false;
        }
        else if (std::isdigit(c)) {
            file += (c - '0');
        }
        else {
            Piece p = Piece::None;
            switch (c) {
                case 'P': p = Piece::WhitePawn; break;
                case 'N': p = Piece::WhiteKnight; break;
                case 'B': p = Piece::WhiteBishop; break;
                case 'R': p = Piece::WhiteRook; break;
                case 'Q': p = Piece::WhiteQueen; break;
                case 'K': p = Piece::WhiteKing; break;
                case 'p': p = Piece::BlackPawn; break;
                case 'n': p = Piece::BlackKnight; break;
                case 'b': p = Piece::BlackBishop; break;
                case 'r': p = Piece::BlackRook; break;
                case 'q': p = Piece::BlackQueen; break;
                case 'k': p = Piece::BlackKing; break;
                default: return false;
            }
            if (file < 8 && rank >= 0) {
                setPiece(makeSquare(rank, file), p);
                file++;
            }
        }
    }

    m_side = (sideStr == "b" || sideStr == "B") ? Color::Black : Color::White;

    m_castling = Castling::None;
    if (castlingStr != "-") {
        for (char c : castlingStr) {
            switch (c) {
                case 'K': m_castling |= Castling::WhiteOO;  break;
                case 'Q': m_castling |= Castling::WhiteOOO; break;
                case 'k': m_castling |= Castling::BlackOO;  break;
                case 'q': m_castling |= Castling::BlackOOO; break;
                default: break;
            }
        }
    }

    if (epStr.size() >= 2 && epStr[0] >= 'a' && epStr[0] <= 'h' && epStr[1] >= '1' && epStr[1] <= '8') {
        int epFile = epStr[0] - 'a';
        int epRank = epStr[1] - '1';
        m_enPassant = makeSquare(epRank, epFile);
    }
    else {
        m_enPassant = Square::none;
    }

    m_halfmove = static_cast<u16>(halfmove);
    m_fullmove = static_cast<u16>(fullmove > 0 ? fullmove : 1);

    return true;
}

std::string Position::toFen() const {
    std::string s;

    for (int r = 7; r >= 0; r--) {

        int emptyCount = 0;

        for (int f = 0; f < 8; f++) {

            Piece p = m_board[static_cast<int>(makeSquare(r, f))];

            if (p == Piece::None) {
                emptyCount++;
            }
            else {
                if (emptyCount > 0) {
                    s += std::to_string(emptyCount);
                    emptyCount = 0;
                }
                char c = '?';
                switch (p) {
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
                    default: break;
                }
                s += c;
            }
        }
        if (emptyCount > 0) {
            s += std::to_string(emptyCount);
        }

        if (r > 0) {
            s += '/';
        }
    }

    s += (m_side == Color::White) ? " w " : " b ";

    std::string castling;
    if (m_castling & Castling::WhiteOO)  castling += 'K';
    if (m_castling & Castling::WhiteOOO) castling += 'Q';
    if (m_castling & Castling::BlackOO)  castling += 'k';
    if (m_castling & Castling::BlackOOO) castling += 'q';

    s += castling.empty() ? "-" : castling;
    s += ' ';

    if (m_enPassant != Square::none) {
        s += static_cast<char>('a' + squareFile(m_enPassant));
        s += static_cast<char>('1' + squareRank(m_enPassant));
    }
    else {
        s += '-';
    }

    s += ' ' + std::to_string(m_halfmove) + ' ' + std::to_string(m_fullmove);

    return s;
}