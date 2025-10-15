#include "movegen.h"

inline void gen_pawn_moves(const Position *pos, Color c)
{
    const bb64 occ   = pos->both;
    const bb64 empty = ~occ;
    const bb64 pawns = pos->pcbb[(c==WHITE) ? W_PAWN : B_PAWN];

    bb64 single = (c == WHITE) ? ((pawns >> 8) & empty) :
                                 ((pawns << 8) & empty);

    bb64 dbl;
    if (c == WHITE)
    {
        bb64 fwdstep = ((pawns & RANK_2) >> 8) & empty;
        dbl = ((fwdstep >> 8) & empty) & RANK_4;
    }
    else
    {
        bb64 fwdstep = ((pawns & RANK_7) << 8) & empty;
        dbl = ((fwdstep << 8) & empty) & RANK_5;
    }

    bb64 promotion = (c == WHITE) ? (single & RANK_8) : (single & RANK_1);

    /* Generate quiet promotions */
    while (promotion)
    {
        int dst = lsb(promotion);
        clrbit(promotion, dst);
        int src = (c == WHITE) ? (dst + 8) : (dst - 8);
        char s1[3], s2[3]; idxtosq(src,s1); idxtosq(dst,s2);
        printf("%c=Q %s%s\n", (!c) ? 'P' : 'p', s1, s2);
        printf("%c=R %s%s\n", (!c) ? 'P' : 'p', s1, s2);
        printf("%c=B %s%s\n", (!c) ? 'P' : 'p', s1, s2);
        printf("%c=N %s%s\n", (!c) ? 'P' : 'p', s1, s2);
    }

    /* Generate single pawn pushes */
    while (single) 
    {
        int dst = lsb(single);
        clrbit(single, dst);
        int src = (c == WHITE) ? (dst + 8) : (dst - 8);

        char s1[3], s2[3]; idxtosq(src, s1); idxtosq(dst, s2);
        printf(" %c1 %s%s\n", (!c) ? 'P' : 'p', s1, s2);
    }

    /* Generate double pawn pushes */
    while (dbl)
    {
        int dst = lsb(dbl);
        clrbit(dbl, dst);
        int src = (c == WHITE) ? (dst + 16) : (dst - 16);

        char s1[3], s2[3]; idxtosq(src, s1); idxtosq(dst, s2);
        printf(" %c2 %s%s\n", (!c) ? 'P' : 'p', s1, s2);
    }

    /* Generate pawn captures (can promote into capture) */
}


void gen_all()
{
    gen_pawn_moves(&pos, WHITE);
    gen_pawn_moves(&pos, BLACK);
}