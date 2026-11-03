#pragma once

#include "types.h"

enum class Square : u8 {

    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,

    none

};

constexpr int squareRank(Square sq) {
    return static_cast<int>(sq) >> 3;
}

constexpr int squareFile(Square sq) {
    return static_cast<int>(sq) & 7;
}

constexpr Square makeSquare(int rank, int file) {
    return static_cast<Square>((rank << 3) | file);
}

enum class Color : u8 {
    White,
    Black,
    None,
};

constexpr Color operator~(Color c) {
    return static_cast<Color>(static_cast<u8>(c) ^ 1);
}

enum class PieceType : u8 {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None
};

enum class Piece : u8 {
    WhitePawn,
    WhiteKnight,
    WhiteBishop,
    WhiteRook,
    WhiteQueen,
    WhiteKing,

    BlackPawn,
    BlackKnight,
    BlackBishop,
    BlackRook,
    BlackQueen,
    BlackKing,

    None
};

constexpr Piece makePiece(Color c, PieceType pt) {
    return static_cast<Piece>(static_cast<u8>(c) * 6 + static_cast<u8>(pt));
}

constexpr PieceType pieceType(Piece p) {
    return (p == Piece::None) ? PieceType::None : static_cast<PieceType>(static_cast<u8>(p) % 6);
}

constexpr Color pieceColor(Piece p) {
    return (p == Piece::None) ? Color::None : static_cast<Color>(static_cast<u8>(p) / 6);
}

namespace Castling {

    constexpr u8 None     = 0;
    constexpr u8 WhiteOO  = 1 << 0;
    constexpr u8 WhiteOOO = 1 << 1;
    constexpr u8 BlackOO  = 1 << 2;
    constexpr u8 BlackOOO = 1 << 3;

    constexpr u8 WhiteAll = WhiteOO  | WhiteOOO;
    constexpr u8 BlackAll = BlackOO  | BlackOOO;
    constexpr u8 All      = WhiteAll | BlackAll;
}