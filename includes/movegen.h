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

static const char cstlmask[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

extern void gen_all(const Position *, MoveList *, GenMode);
extern void add_move(MoveList *, uint32_t);
extern void print_moves(const MoveList *);
extern bool make_move(Position *, uint32_t);

/* Generate a 32 bit (24 used) integer encoding a move*/
static inline uint32_t encode(int src, int dst, Piece pc, Piece promo,
                              bool cap, bool dbl, bool en, bool cstl)
{
    return  (src)         |
            (dst   << 6)  |
            (pc    << 12) |
            (promo << 16) |
            (cap   << 20) |
            (dbl   << 21) |
            (en    << 22) |
            (cstl  << 23);

}

static inline void decode(uint32_t encoding, int *src, int *dst, Piece *pc,
                         Piece *promo, bool *cap, bool *dbl, bool *en,
                         bool *cstl)
{
    *src    =  encoding & 0x0000003f;
    *dst    = (encoding & 0x00000fc0) >> 6;
    *pc     = (encoding & 0x0000f000) >> 12;
    *promo  = (encoding & 0x000f0000) >> 16;
    *cap    = (encoding & 0x00100000) >> 20;
    *dbl    = (encoding & 0x00200000) >> 21;
    *en     = (encoding & 0x00400000) >> 22;
    *cstl   = (encoding & 0x00800000) >> 23;
}

static inline void print_move(uint32_t encoding)
{
    int   src    =  encoding & 0x0000003f;
    int   dst    = (encoding & 0x00000fc0) >> 6;
    Piece pc     = (encoding & 0x0000f000) >> 12;
    Piece promo  = (encoding & 0x000f0000) >> 16;
    bool  cap    = (encoding & 0x00100000) >> 20;
    bool  dbl    = (encoding & 0x00200000) >> 21;
    bool  en     = (encoding & 0x00400000) >> 22;
    bool  cstl   = (encoding & 0x00800000) >> 23;

    char buf1[3]; idxtosq(src, buf1);
    char buf2[3]; idxtosq(dst, buf2);

    printf("Move: %c%s%s%c %d%d%d%d\n", idxtopc(pc), buf1, buf2,
                                  (promo == 0) ? ' ' : idxtopc(promo),
                                  cap, dbl, en, cstl);
}

/* Decode a field given an encoding */
static inline int dcdsrc(uint32_t encoding)
{
    return (encoding & 0x0000003f);
}

static inline int dcddst(uint32_t encoding)
{
    return (encoding & 0x00000fc0) >> 6;
}

static inline Piece dcdpc(uint32_t encoding)
{
    return (Piece)((encoding & 0x0000f000) >> 12);
}

static inline Piece dcdpromo(uint32_t encoding)
{
    return (Piece)((encoding & 0x000f0000) >> 16);
}

static inline bool dcdcap(uint32_t encoding)
{
    return (bool)((encoding & 0x00100000) >> 20);
}

static inline int dcddbl(uint32_t encoding)
{
    return (bool)((encoding & 0x00200000) >> 21);
}

static inline bool dcden(uint32_t encoding)
{
    return (bool)((encoding & 0x00400000) >> 22);
}

static inline bool dcdcstl(uint32_t encoding)
{
    return (bool)((encoding & 0x00800000) >> 23);
}

#endif /* MOVEGEN_H */