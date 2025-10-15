#include <stdio.h>

#include "attacks.h"
#include "movegen.h"

inline void gen_pawn_moves(const Position *pos, Color c)
{
    const bb64 occ   = pos->both;
    const bb64 empty = ~occ;
    const bb64 pawns = pos->pcbb[(c==WHITE) ? W_PAWN : B_PAWN];
    const bb64 enemies  = (c == WHITE) ? pos->black : pos->white;

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
        int dst = pop_lsb(promotion);
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
        int dst = pop_lsb(single);
        int src = (c == WHITE) ? (dst + 8) : (dst - 8);

        char s1[3], s2[3]; idxtosq(src, s1); idxtosq(dst, s2);
        printf(" %c1 %s%s\n", (!c) ? 'P' : 'p', s1, s2);
    }

    /* Generate double pawn pushes */
    while (dbl)
    {
        int dst = pop_lsb(dbl);
        int src = (c == WHITE) ? (dst + 16) : (dst - 16);

        char s1[3], s2[3]; idxtosq(src, s1); idxtosq(dst, s2);
        printf(" %c2 %s%s\n", (!c) ? 'P' : 'p', s1, s2);
    }

    /* Generate pawn captures */
    bb64 copy = pawns;
    while (copy)
    {
        int src = pop_lsb(copy);
        
        bb64 captures = ptable[c][src] & enemies;
        bb64 promocaps = captures & ((c == WHITE) ? RANK_8 : RANK_1);
        captures ^= promocaps;

        char s1[3]; idxtosq(src, s1); char buf[7];
        snprintf(buf, 6, "%cx %s", (!c) ? 'P' : 'p', s1);
        while (captures)
        {
            int dst = pop_lsb(captures);
            char s2[3]; idxtosq(dst, s2);
            printf("%s%s\n", buf, s2);
        }

        snprintf(buf, 6, "%cx %s", (!c) ? 'P' : 'p', s1);
        while (promocaps)
        {
            int dst = pop_lsb(promocaps);
            char s2[3]; idxtosq(dst, s2);
            printf("%s%s %c=Q\n", buf, s2, (!c) ? 'P' : 'p');
            printf("%s%s %c=R\n", buf, s2, (!c) ? 'P' : 'p');
            printf("%s%s %c=B\n", buf, s2, (!c) ? 'P' : 'p');
            printf("%s%s %c=N\n", buf, s2, (!c) ? 'P' : 'p');
        }

        if (pos->enpassant != none)
        {
            bb64 enmask = ptable[c][src] & (1ULL << pos->enpassant);

            if (enmask)
            {
                int dst = pop_lsb(enmask);
                char s2[3]; idxtosq(dst, s2);
                printf("%ce %s%s\n", (!c) ? 'P' : 'p', s1, s2);
            }
        }
    }

}


void gen_all()
{
    gen_pawn_moves(&pos, WHITE);
    gen_pawn_moves(&pos, BLACK);
}