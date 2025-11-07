#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <assert.h>
#include <stdio.h>

#include "bitboard.h"

#define  PTYPE(p)   ((p) >= 6) ? ((p) - 6) : (p)

/* Bitboards with rank i set */
#define  RANK_1  0xFF00000000000000ULL
#define  RANK_2  0x00FF000000000000ULL
#define  RANK_3  0x0000FF0000000000ULL
#define  RANK_4  0x000000FF00000000ULL
#define  RANK_5  0x00000000FF000000ULL
#define  RANK_6  0x0000000000FF0000ULL
#define  RANK_7  0x000000000000FF00ULL
#define  RANK_8  0x00000000000000FFULL

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

extern  void  gen_all(const Position *P, MoveList *ml, GenMode flag);
extern  void  add_move(MoveList *ml, uint32_t mv);
extern  void  print_moves(const MoveList *ml);
extern  bool  make_move(Position *P, uint32_t mv);

/* Generate a 32 bit (28 used) integer encoding a move*/
static inline uint32_t encode(int src, int dst, Piece pc, Piece promo,
                              Piece target, bool cap, bool dbl, bool en,
                              bool cstl)
{
    return  (src)          |
            (dst    << 6)  |
            (pc     << 12) |
            (promo  << 16) |
            (target << 20) |
            (cap    << 24) |
            (dbl    << 25) |
            (en     << 26) |
            (cstl   << 27);

}

static inline void decode(uint32_t encoding, int *src, int *dst, Piece *pc,
                          Piece *promo, Piece *target, bool *cap, bool *dbl,
                          bool *en, bool *cstl)
{
    *src    =  encoding & 0x0000003f;
    *dst    = (encoding & 0x00000fc0) >> 6;
    *pc     = (encoding & 0x0000f000) >> 12;
    *promo  = (encoding & 0x000f0000) >> 16;
    *target = (encoding & 0x00f00000) >> 20;
    *cap    = (encoding & 0x01000000) >> 24;
    *dbl    = (encoding & 0x02000000) >> 25;
    *en     = (encoding & 0x04000000) >> 26;
    *cstl   = (encoding & 0x08000000) >> 27;
}

static inline void print_move(uint32_t encoding)
{
    int   src    =  encoding & 0x0000003f;
    int   dst    = (encoding & 0x00000fc0) >> 6;
    Piece pc     = (encoding & 0x0000f000) >> 12;
    Piece promo  = (encoding & 0x000f0000) >> 16;
    Piece target = (encoding & 0x00f00000) >> 20;
    bool  cap    = (encoding & 0x01000000) >> 24;
    bool  dbl    = (encoding & 0x02000000) >> 25;
    bool  en     = (encoding & 0x04000000) >> 26;
    bool  cstl   = (encoding & 0x08000000) >> 27;

    char buf1[3]; idxtosq(src, buf1);
    char buf2[3]; idxtosq(dst, buf2);
    char promoted = (promo) ? idxtopc(promo) : ' ';
    char captured = (target == NO_PC) ? ' ' : idxtopc(target);

    printf("MOVE: %c%s%sx%c %c %d%d%d%d\n", idxtopc(pc), buf1, buf2, 
                                            captured, promoted, cap, dbl,
                                            en, cstl);
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

static inline Piece dcdtarget(uint32_t encoding)
{
    return (Piece)((encoding & 0x00f00000) >> 20);
}

static inline bool dcdcap(uint32_t encoding)
{
    return (bool)((encoding & 0x01000000) >> 24);
}

static inline int dcddbl(uint32_t encoding)
{
    return (bool)((encoding & 0x02000000) >> 25);
}

static inline bool dcden(uint32_t encoding)
{
    return (bool)((encoding & 0x04000000) >> 26);
}

static inline bool dcdcstl(uint32_t encoding)
{
    return (bool)((encoding & 0x08000000) >> 27);
}

static inline Piece get_target(const Position *P, int dst)
{
    int dx = (P->side == WHITE) ? 6 : 0;

    for (int i = W_PAWN + dx; i <= W_KING + dx; i++)
    {
        if (getbit(P->pcbb[i], dst)) {
            return (Piece)(i);
        }
    }

    /* Should never be called w/o capture */
    assert(0);
    return NO_PC;
}

#endif /* MOVEGEN_H */