#pragma once

#include <array>
#include <string>

#include "core.h"
#include "move.h"
#include "types.h"

class Position {
public:

    Position() = default;
    explicit Position(std::string& fen);

    bool setFen(std::string& fen);
    std::string toFen() const;

    constexpr Bitboard pieces(Piece p) const {
        return m_pieces[static_cast<int>(p)];
    }

    constexpr Bitboard pieces(Color c, PieceType pt) const {
        return m_pieces[static_cast<int>(makePiece(c, pt))];
    }

    constexpr Bitboard pieces(Color c) const {
        return m_occupancy[static_cast<int>(c)];
    }

    constexpr Bitboard pieces() const {
        return m_occupancy[0] | m_occupancy[1];
    }

    constexpr Piece pieceAt(Square sq) const {
        return m_board[static_cast<int>(sq)];
    }

    constexpr Color side() const { return m_side; }
    constexpr Square enPassant() const { return m_enPassant; }
    constexpr u8 castling() const { return m_castling; }
    constexpr u16 halfmove() const { return m_halfmove; }
    constexpr u16 fullmove() const { return m_fullmove; }

    constexpr Square kingSquare(Color c) const {
        return static_cast<Square>(util::lsb(pieces(c, PieceType::King).val()));
    }

    template<Color C>
    constexpr Bitboard pawns() const {
        return pieces(C, PieceType::Pawn);
    }

    template<Color C>
    constexpr Bitboard knights() const {
        return pieces(C, PieceType::Knight);
    }

    template<Color C>
    constexpr Bitboard bishops() const {
        return pieces(C, PieceType::Bishop);
    }

    template<Color C>
    constexpr Bitboard rooks() const {
        return pieces(C, PieceType::Rook);
    }

    template<Color C>
    constexpr Bitboard queens() const {
        return pieces(C, PieceType::Queen);
    }

    template<Color C>
    constexpr Bitboard king() const {
        return pieces(C, PieceType::King);
    }

    template<Color C>
    constexpr Bitboard oppPawns() const {
        return pawns<~C>();
    }

    template<Color C>
    constexpr Bitboard oppKnights() const {
        return knights<~C>();
    }

    template<Color C>
    constexpr Bitboard oppBishops() const {
        return bishops<~C>();
    }

    template<Color C>
    constexpr Bitboard oppRooks() const {
        return rooks<~C>();
    }

    template<Color C>
    constexpr Bitboard oppQueens() const {
        return queens<~C>();
    }

    template<Color C>
    constexpr Bitboard oppKing() const {
        return king<~C>();
    }

    bool makeMove(Move m);
    void unmakeMove();

    void clear();

private:
    std::array<Bitboard, 12> m_pieces{};
    std::array<Bitboard, 2>  m_occupancy{};
    std::array<Piece, 64>    m_board{};

    Color  m_side{Color::White};
    Square m_enPassant{Square::none};
    u8     m_castling{Castling::None};
    u16    m_halfmove{0};
    u16    m_fullmove{1};

    void setPiece(Square sq, Piece p);
    void removePiece(Square sq);
    void movePiece(Square src, Square dst);
};
