#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <stdio.h>

#include "bitboard.h"

/* Bitboards with rank i set */
#define RANK_1  0xFF00000000000000ULL
#define RANK_2  0x00FF000000000000ULL
#define RANK_3  0x0000FF0000000000ULL
#define RANK_4  0x000000FF00000000ULL
#define RANK_5  0x00000000FF000000ULL
#define RANK_6  0x0000000000FF0000ULL
#define RANK_7  0x000000000000FF00ULL
#define RANK_8  0x00000000000000FFULL

extern void gen_pawn_moves(const Position *, Color);
/* TODO
extern void gen_king_moves(const Position *, Color);
extern void gen_knight_moves(const Position *, Color);
extern void gen_bishop_moves(const Position *, Color);
extern void gen_rook_moves(const Position *, Color);
extern void gen_queen_moves(const Position *, Color);
*/
extern void gen_all(void);

#endif /* MOVEGEN_H */