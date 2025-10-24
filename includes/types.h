#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define MAX_MOVES   256
#define MAX_PLY     128

typedef uint64_t bb64;

typedef unsigned long long ull;

typedef enum
{
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,

    none,
}
Square;

typedef enum { WHITE, BLACK, BOTH } Color;

typedef enum
{
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    NO_PC
}
Piece;

typedef enum
{
    CASTLE_WK = 1 << 0, CASTLE_WQ = 1 << 1,
    CASTLE_BK = 1 << 2, CASTLE_BQ = 1 << 3
}
CastlingRights;

/* Current unused */
typedef enum
{
    GEN_ALL,
    GEN_CAPTURES,
    GEN_EVASIONS,
    GEN_QUIET,
    GEN_QUIET_CHECKS
}
GenMode;

typedef struct
{
    bb64     pcbb[12];              /* Occupancy bitboards for pieces */
    bb64     white, black, both;    /* Occupancy bitboards for colors */
    uint64_t zobrist;               /* Zobrist position hash          */
    uint16_t fmcount;               /* Move number                    */
    uint8_t  hmclock;               /* 50 move counter                */
    uint8_t  castling;              /* Castling rights bitmask        */
    uint8_t  side;                  /* Color to move                  */
    uint8_t  enpassant;             /* En passants square             */
}
Position;

typedef struct
{
    uint32_t moves[MAX_MOVES];
    uint32_t scores[MAX_MOVES];
    uint8_t  nmoves;
}
MoveList;

typedef struct
{
    uint32_t klr_table[2][MAX_PLY];
    uint32_t hist_table[12][64];
}
QuietTable;

#endif /* TYPES_H */